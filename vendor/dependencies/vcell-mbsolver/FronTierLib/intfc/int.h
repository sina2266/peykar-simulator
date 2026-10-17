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
*				int.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*			INTERFACE Structures:
*/

#if !defined(_INT_H)
#define _INT_H

#include <intfc/geom.h>
#ifdef IMESH
#include <intfc/iTaps.h>
#endif /*def IMESH */

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

typedef int COMPONENT;		/* Labels Connected Domains */


 /* Point in the Plane: */
struct _POINT
{
	int _boundary;		/* Mandatory first element,  see notes on
				 * boundary macros below */

	double	_coords[MAXD];

	struct {
	    unsigned int _boundary : 1;
	    unsigned int _user0 : 1;
	    unsigned int _user1 : 1;
	    unsigned int _user2 : 1;
	    unsigned int _user3 : 1;
	    unsigned int _user4 : 1;
	    unsigned int _user5 : 1;
	    unsigned int _user6 : 1;
	    unsigned int _user7 : 1;
	    unsigned int _user8 : 1;
	    unsigned int _user9 : 1;
	} _point_flags;

	struct _INTERFACE *interface;
	struct _POINT *obj;	/* refers back to point, see boundary macro */
	struct _HYPER_SURF *hs;
	union _HYPER_SURF_ELEMENT *hse;
	struct _TRI **tris;	/* 3D for tris with vertex as the point */
	int num_tris;

	union {
	    boolean      _sorted;
	    POINTER	 _opaque_pointer;
	    struct _NODE *_node;
	    int          _index;
	} private_data;
	double		_nor[3];
	double		_nor0[3];	/* First order normal for WLSP */
	double		curvature;
	double           vel[3];
	boolean		crx;
	int		indx;
};
typedef struct _POINT FT_POINT;

 /* Macros of accessing private fields of points */
#define Private_data(p)	   (p)->private_data
#define Index_of_point(p)  Private_data(p)._index
#define node_at_point(p)   Private_data(p)._node
#define sorted(p)          Private_data(p)._sorted
#define opaque_pointer(p)  Private_data(p)._opaque_pointer


 /* An Elementary Bond on a Curve: */

struct _BOND_TRI
{
	struct _TRI     *tri;
  	struct _SURFACE *surface;
	struct _CURVE   *curve;
	struct _BOND    *bond;
	ORIENTATION     orient;
};
typedef struct _BOND_TRI BOND_TRI;

struct _BOND
{
	struct _BOND *next;
	struct _BOND *prev;
	FT_POINT  *start;
	FT_POINT  *end;
	double length;
	double length0;			/* for fixed length bond */

	struct _BOND_TRI **_btris;	/* Bounding triangles in angle order */
};
typedef struct _BOND BOND;

struct _BOND_REDIST_PARAMS {
        double   max_bond_length;   /* maximum bond length        */
        double   min_bond_length;   /* minimum bond length        */
};
typedef struct _BOND_REDIST_PARAMS BOND_REDIST_PARAMS;


 /* A Curve in the Plane: */
struct _CURVE
{

	int _boundary;		/* Mandatory first element,  see notes on
				 *  boundary macros below */
	struct _CURVE *obj;	/* refers back to curve, see boundary macro */
	struct _HYPER_SURF *hs;
	struct _HYPER_SURF_BDRY *hsb;

	/* Topological Part: */
	struct _INTERFACE *interface;
	struct _NODE *start;
	struct _NODE *end;
	double nor_start[MAXD];
	double nor_end[MAXD];
	double curvature_start;
	double curvature_end;

	struct _SURFACE **pos_surfaces;
	struct _SURFACE **neg_surfaces;
	int	number;
	int	redist_order;

	/* Representation Specific Part: */
	BOND *first;
	BOND *last;
	int num_points;
	int orientation;	/* Orientation of closed curve */
				/* 1 for positive, -1 for negative */

	int  sindx, eindx;
	POINTER extra;
};
typedef struct _CURVE CURVE;


 /* Oriented curves */

struct _O_CURVE
{
	struct _CURVE *curve;
	ORIENTATION orient;
	/* REMOVE THE TWO LINES BELOW */
	struct _O_CURVE *prev;
	struct _O_CURVE *next;
};
typedef struct _O_CURVE O_CURVE;

 /* REMOVE THIS DATA TYPE FROM THE CODE */
struct _O_CURVE_FAMILY
{
	struct _O_CURVE *first;
	struct _O_CURVE *last;
};
typedef struct _O_CURVE_FAMILY O_CURVE_FAMILY;


 /* A Node of an Interface: */
struct _NODE
{
	int _boundary;		/* Mandatory first element,  see notes on
				 *  boundary macros below */
	struct _NODE *obj;	/* refers back to node, see boundary macro */
	struct _HYPER_SURF_BDRY *hsb;

	struct _INTERFACE *interface;
	FT_POINT *posn;

	struct _CURVE **in_curves;	/* Pointer to Set of In Curves */
	struct _CURVE **out_curves;	/* Pointer to Set of Out Curves */
	POINTER extra;			/* For special use */
};
typedef struct _NODE NODE;


 /* Allowable neighbor objects to tris */

union _TRI_NEIGHBOR
{
	struct _TRI      *tri;
	struct _BOND_TRI *btri;
	POINTER          ptr;
	int              index;
};
typedef union  _TRI_NEIGHBOR TRI_NEIGHBOR;

enum _TRI_STORAGE_TYPE {
	MIN_TRI_STORAGE   = 0x001,
	TRI_PLUS_NORMAL   = 0x010,
	FULL_TRI_GEOMETRY = 0x100
};
typedef enum _TRI_STORAGE_TYPE TRI_STORAGE_TYPE;

struct _TRI
{
	FT_POINT *__pts[3];
	/* object adj to edge ij */
	TRI_NEIGHBOR neighbor[3];
	double side_length0[3];		/* equilibrium length of each side */
	struct _SURFACE	*surf;		/* surface in which the triangle lies */
	struct _TRI *prev;
	struct _TRI *next;
	int boundary;		/* tri bonds on interface curves? */
	int order;		/* used as an identification for a tri, avoid 
				   the conflict with _index in private_data. */
	union
	{
	    int		   _index;
	    POINTER 	   _workspace;
	    struct _C_BOND **_tri_cross_list;
	    int		   _icoords[3];
	    boolean           _projection_computed;
	    boolean	   _modified;
	    /* The following is used in ellip for 3d tetrahedral mesh */
	    struct
            {
                char c0;
                char c1;
                char c2;
                char c3;
            } c;
	} private_data;
};
typedef struct _TRI TRI;

typedef struct {struct _TRI tri; struct _SURFACE *s;} TRI_LIST_HEAD;

struct _TRI_REDIST_PARAMS {
        double   max_sqr_area;   /* maximum triangle area        */
        double   min_sqr_area;   /* minimum triangle area        */
        double   max_sqr_length; /* maximun triangle side length */
        double   aspect_tol2;    /* square of aspect ratio tolerance */
};
typedef struct _TRI_REDIST_PARAMS TRI_REDIST_PARAMS;

struct _SURFACE
{
	int _boundary;		/* Mandatory first element,  see notes on
				 *  boundary macros below */
	struct _SURFACE *obj;	/* refers back to surface, see boundary macro */
	struct _HYPER_SURF *hs;

	/* Topological Part */

	struct _INTERFACE *interface;

	/*
	 * SURFACE BOUNDARIES.  The boundary of a surface consists of
	 * a set of curves gathered into two classes,  pos_curves and
	 * neg_curves.  A given curve is defined as positive with respect
	 * the surface if its orientation agrees with that of the adjoining
	 * triangles that connect the curve to the surface, negative otherwise.
	 * Triangles are assumed to be oriented in a counter clockwise manner
	 * where observed from the side into which the normal vector points.
	 * Thus if the vertices of a triangle are p[0], p[1], p[2], then the
	 * normal has direction given by the vector cross product
	 * (p[1] - p[0])X(p[2] - p[0]).
	 * This means that a curve is positive with respect to a surface if
	 * each bond on that curve statisfies bond->start = p[i] and
	 * bond->end = p[(i+1)%3] for the triangle connected to that bond.
	 * Similarlly the curve is negative if bond->start = p[i] and
	 * bond->end = p[(i-1)%3].  It is assumed that all triangles on
	 * a surface are oriented consistently.
	 */

	struct _CURVE **pos_curves;	/* pos orient curves in bdry */
	struct _CURVE **neg_curves;	/* neg orient curves in bdry */

	int number;
	int redist_order;

	struct _C_CURVE **c_curves;	/* c_curves on surface */

	TRI_LIST_HEAD _First_tri;
	struct _TRI  _Last_tri;

	/* Representation Specific Part */

	int num_tri;
	POINTER extra;
};
typedef struct _SURFACE SURFACE;

struct _O_SURFACE
{
	struct _SURFACE *surface;
	ORIENTATION	orient;
};
typedef struct _O_SURFACE O_SURFACE;

struct _WEDGE{
	SURFACE	*s;
	TRI	**tris;
	int	n_tris;

};  
typedef struct _WEDGE WEDGE;

struct _NEIGHBORHOOD{
	struct  _POINT	*posn;
	struct  _WEDGE	*wedge;
	int		n_wedges;
	int		n_max_wedges;
};
typedef struct _NEIGHBORHOOD NEIGHBORHOOD;


	/* Hypersurface Data structures */

struct _HYPER_SURF
{
	union {
		int	*bptr;
		FT_POINT	*p;
		CURVE	*c;
		SURFACE	*s;
	} obj;

	struct _INTERFACE *interface;
	struct _HYPER_SURF *hs;
	/*
	 * NOTE on components.  The positive side of a hypersurface is by
	 * definition the side into which the hypersurface normal points.
	 *
	 * For points in 1D this is the right side of the point and the
	 * negative side is the left side.
	 *
	 * For curves in 2D the positive side is the right side of curve
	 * and the negative side is the left side of the curve when
	 * viewed in a direction aligned with the curve tangent (ie from the
	 * start to end of a bond).
	 *
	 * For surfaces in 3D the positive side is defined as the side
	 * into which the normal points. The normal vector on a triangle
	 * has direction given by the vector cross product (p1 - p0)X(p2 - p0)
	 * so standing on point p0 and looking into the normal direction we
	 * are looking into the local positive side of surface.  The negative
	 * side is of curve the side opposite to the positive side.
	 */

	COMPONENT pos_comp, neg_comp;
	int pp_index;  /* Identifies families of connected hypersurfaces */
        struct _DYNAMIC_COMP_CHANGE {
            boolean   change;
            int    side;
            COMPONENT  new_comp;
        } dyn_comp_ch;
};
typedef struct _HYPER_SURF HYPER_SURF;


union _HYPER_SURF_ELEMENT {
	byte	bt;
	BOND	b;
	TRI	tri;
};
typedef union _HYPER_SURF_ELEMENT HYPER_SURF_ELEMENT;

struct _HYPER_SURF_BDRY
{
	union {
		int	*bptr;
		NODE	*n;
		CURVE	*c;
	} obj;
	struct _INTERFACE *interface;
	struct _HYPER_SURF_BDRY *hsb;
};
typedef struct _HYPER_SURF_BDRY HYPER_SURF_BDRY;

struct _COMP_LIST {
	int ncomps, max_ncomps;
	COMPONENT *comps;
	boolean (*_is_comp_in_list)(COMPONENT,struct _COMP_LIST*,
				    struct _INTERFACE*);
	void (*_add_comp_to_list)(COMPONENT,struct _COMP_LIST*,
				  struct _INTERFACE*);
};
typedef struct _COMP_LIST COMP_LIST;

struct _VOLUME_FRAC {
	COMPONENT comp_vfrac;
};
typedef struct _VOLUME_FRAC VOLUME_FRAC;

 /* Interface Structure: */
struct _INTERFACE
{
	struct _HYPER_SURF	**hss;
	struct _HYPER_SURF_BDRY **hses;

	struct _POINT	**points; /* Pointer to Set of Points */
	struct _NODE	**nodes;	/* Pointer to Set of Nodes */
	struct _CURVE	**curves;	/* Pointer to Set of Curves */
	struct _SURFACE **surfaces;	/* Pointer to Set of Surfaces */
	struct _C_CURVE **c_curves;	/* c_curves on interface */

	int		dim;		/* Dimension of Imbedding Space */
	int		num_points;	/* Total from curves */

	/* Internal Variables: */
	struct Table	*table;	/* Pointer to Interface Table */
	boolean	modified;	/* Interface Recently Modified */
	boolean	normal_unset;	/* Normal unset since last modify */
	boolean	curvature_unset;/* Curvature unset since last modify */
	boolean            _interface_reconstructed;
	int		rect_bdry_type[MAXD][2];
	COMPONENT	elliptic_comp;	/* component of elliptic region */
	COMPONENT	default_comp;	/* for subdomain with no surf */
	struct _INTERFACE  *prev_interf;
	POINTER		e_comps;
	struct	_TRI	**point_tri_store;
};
typedef struct _INTERFACE INTERFACE;

#define interface_reconstructed(intfc) ((intfc)->_interface_reconstructed)
#define Dimension(intfc) ((intfc)->dim)
/*#bjet2 */
#define prev_interface(intfc)    ((intfc)->prev_interf)

 /* Node with curves angle ordered */
 /* documentation needed for structure elements */
struct _O_NODE
{
	struct _O_NODE	*prev, *next;
	struct _NODE	*node;
	struct _NODE	**nopp;
	struct _CURVE	**nc;
	FT_POINT		**pt;
	double		*ang;
	ORIENTATION	*orient;
	int		num_c;
};
typedef struct _O_NODE O_NODE;

/* Pointer linkage used in hash table */

struct _P_LINK {
	POINTER pl, pr;
};
typedef struct _P_LINK P_LINK;

/*
*		Block based interface reconstruction structure:
*/

struct _BBI_POINT {		/* Block based interface point */
        SURFACE *s;
	CURVE   *c;
	FT_POINT	*p;
};
typedef struct _BBI_POINT BBI_POINT;

struct _BLK_INFO {
        SURFACE         **surfs;
        CURVE           **curves;
        TRI             **cur_tris;
        TRI             **cur_bonds;
        int             num_surfs;
        int             num_curves;
	boolean		do_volume_frac;
};
typedef struct _BLK_INFO BLK_INFO;

enum _BLK_TYPE {
        UNKNOWN_BLOCK = -1,
        BDRY_BLOCK = 0,
        COMP2_BLOCK = 1,
        COMP3_BLOCK = 2
};
typedef enum _BLK_TYPE BLK_TYPE;

enum _ELEMENT_TYPE {
        NONE_EDGE = 0,
        PARTIAL_EDGE = 1,
        FULL_EDGE = 2,
        BDRY_EDGE = 3
};
typedef enum _ELEMENT_TYPE ELEMENT_TYPE;

enum _CELL_TYPE {
        EXTERNAL = 0,
        PARTIAL = 1,
        INTERNAL = 2,
	BOUNDARY = 3
};
typedef enum _CELL_TYPE CELL_TYPE;

struct _SURF {
        ELEMENT_TYPE    etype;      /*type of the surface */
        double           area;
        double           centroid[3];
};
typedef struct _SURF SURF;

struct _EDGE {
        ELEMENT_TYPE    etype;      /*type of the edge */
        double           length;
        double           cen[2];    /*center point of the edge */
};
typedef struct _EDGE EDGE;

struct _BLK_SURF {
        SURF            surf[3][2];    /*six surfaces */
        CELL_TYPE       ctype;        /*type of the block */
};
typedef struct _BLK_SURF BLK_SURF;

struct _BLK_EDGE {
        EDGE            edge[2][2];    /*four surfaces */
        CELL_TYPE       ctype;        /*type of the block */
};
typedef struct _BLK_EDGE BLK_EDGE;
struct _BLK_CRX {
        COMPONENT       ***comp;
        int             ***ix;
        int             ***iy;
        int             ***iz;
        int             num_comps;
        int             nv[8];
        COMPONENT       comps[8];
        BBI_POINT       ****crx;
        BBI_POINT       ***curve_crx;
        BBI_POINT       *node_crx;
        BBI_POINT       *crx_store;
        BLK_INFO        *blk_info;
        BLK_TYPE        blk_type;
	double		****corner_coords;	/* for volume fraction */
	double		h[MAXD];		/* length in each dimension */
	double		cell_volume;		/* volume of regular cell */
	COMPONENT       comp_vfrac;		/* comp of volume fraction */
	boolean		debug_flag;	/* for debugging the block */
};
typedef struct _BLK_CRX BLK_CRX;

struct _BLK_TRI {
        TRI             *first[12];
        BOND            *bonds[12];
        /*#bjet2        two bonds belong to only one curve */
	BOND            *bonds1[12];
        CURVE           *curves[12];
        SURFACE         *surfs[8];
        int             num_null_sides[8];
        int             num_tris[8];
	int             ic[3];
        int             is[3];
        int             num_curves;
        int             num_surfaces;
        BLK_INFO        *blk_info;
	double		area;
	double		volume_fraction;
};
typedef struct _BLK_TRI BLK_TRI;

struct _BLK_BOND {
        BOND            *bonds[4][2];
        CURVE           *curves[4];
        int             num_bonds[4];
        int             num_curves;
        BLK_INFO        *blk_info;
};
typedef struct _BLK_BOND BLK_BOND;

struct  _EG_CRX {
        BBI_POINT ****x_crx;
        BBI_POINT ****y_crx;
        BBI_POINT ****z_crx;
        BBI_POINT ****x_curve_crx;
        BBI_POINT ****y_curve_crx;
        BBI_POINT ****z_curve_crx;
        BBI_POINT ****node_crx;
        BBI_POINT *crx_store;
        COMPONENT ***comp;
	SURFACE   **surfaces;
	int	  num_surfaces;
        CURVE     **curves;
        int       num_curves;
};
typedef struct _EG_CRX EG_CRX;



/*
*				CROSS Structure:
*
*	This structure describes the an intersection point on an INTERFACE.
*	It is a highly redundant object.   The NODE entry is a NODE
*	describing the crossing in terms of position and sets of in and
*	out curves.  The other entries consist of the POINT p of
*	intersection, the intersecting BONDS b1,b2 and the intersecting
*	CURVES c1,c2.
*
*	The start and end verticies and sides identify the sides cut
*	by the intersection line.  If this line crosses a vertex, that
*	vertex index is recorded in the start or end field corresponding
*	to the start or end of the bond.  Otherwise these fields are set
*	to ERROR.  Similarly the start and end fields identify the sides
*	of the triangles cut by the intersection line.  If the start or
*	end occurs at a vertex,  the corresponding side field is ERROR.
*/


struct _C_SURF_FLAG
{
	boolean _on_bdry;        /* TRUE if the crossing point is on the
				  * boundary (edge or vertex) of the
				  * correspond triangle */
	boolean _edge_vertex;    /* only used when _on_bdry is TRUE,  TRUE if
				  * the crossing point is not a triangle
				  * vertex */
	int     _tri_side_index; /* only used when both _on_bdry iand
			          * _edge_vertex are TRUE,  gives the side of
			          * triangle upon which the crossing point
				  * lies */
};
typedef struct _C_SURF_FLAG C_SURF_FLAG;

#define cs_tri_side_index(f) (f)._tri_side_index
#define cs_on_bdry(f) (f)._on_bdry
#define cs_edge_vertex(f) (f)._edge_vertex

struct _C_SURF           /* records detailed crossing information */
{
    	struct	_TRI	*t,          /* pair of intersecting triangles */
			*prev_t,     /* prev, next intscting tris */
			*next_t;
	C_SURF_FLAG 	_flag_start,  /* new stuff, records detailed crossing */
			_flag_end;    /* information */
};
typedef struct _C_SURF C_SURF;

#define	cs_flag_start(cs) (cs)._flag_start
#define	cs_flag_end(cs)	  (cs)._flag_end

struct _C_BOND   /* cross bond */
{
	struct	_C_BOND	*prev;	/* fields present in a normal BOND */
	struct	_C_BOND	*next;
	struct	_POINT	*start;
	struct	_POINT	*end;
	struct	_BOND	*bond;
	struct	_C_SURF	s[2];  /* 2 structures, one for each surface */
	struct _C_CURVE	*c_curve;
};
typedef struct _C_BOND   C_BOND;

#define	cb_flag_start(cb,i)           cs_flag_start((cb)->s[i])
#define	cb_start_tri_side_index(cb,i) cs_tri_side_index(cb_flag_start(cb,i))
#define is_start_on_bdry(cb,i)        cs_on_bdry(cb_flag_start(cb,i))
#define is_start_to_edge(cb,i)        cs_edge_vertex(cb_flag_start(cb,i))
#define is_start_to_vertex(cb,i)      (!is_start_to_edge(cb,i))

#define	cb_flag_end(cb,i)           cs_flag_end((cb)->s[i])
#define	cb_end_tri_side_index(cb,i) cs_tri_side_index(cb_flag_end(cb,i))
#define is_end_on_bdry(cb,i)        cs_on_bdry(cb_flag_start(cb,i))
#define is_end_to_edge(cb,i)        cs_edge_vertex(cb_flag_end(cb,i))
#define is_end_to_vertex(cb,i)      (!is_end_to_edge(cb,i))

struct _C_CURVE    /* 3d Cross (intersection) Curve */
{
	/* Topological Part: */
	INTERFACE *interface;
	NODE	  *start;
	NODE	  *end;

	SURFACE	  *s[2];
	int	  boundary;
	/* Representation Specific Part: */
	C_BOND	  *first;
	C_BOND	  *last;
	int	  num_points;
	CURVE	  *curve;
};
typedef struct _C_CURVE C_CURVE;

struct _Cross
{
	struct _Cross *prev;
	struct _Cross *next;

	FT_POINT	**pt;	/* Points that have crossed */
	int     npt;

	FT_POINT *p;		/* Intersection FT_POINT */
	BOND *b1, *b2;		/* BONDS that Intersect */
	CURVE *c1, *c2;		/* CURVES that Intersect */

	C_CURVE *c_curve;
};
typedef struct _Cross CROSS;

struct _INDEX {
	int I1, I2, I3;  
	int tempspace;   
	POINTER  tri_workspace;
};		         
typedef struct _INDEX INDEX;

/* Grid line crossing structures */

        /* crossing directions */

enum _CROSSING_DIRECTION {
        LEFT_TO_RIGHT  = 1,
        RIGHT_TO_LEFT  = 2,
        BELOW_TO_ABOVE = 3,
        ABOVE_TO_BELOW = 4
};
typedef enum _CROSSING_DIRECTION CROSSING_DIRECTION;

typedef struct { double _coords[MAXD];} TG_PT;

struct _CRXING {
        int                crx_num;
        FT_POINT              *pt;
	BOND		   *bond;
        int                end;
        TG_PT              *nd;
        CROSSING_DIRECTION crossing_direction;
        HYPER_SURF         *hs;
	/*#bjet2  curve crx */
        HYPER_SURF_BDRY    *hsb;
	COMPONENT          lcomp, ucomp;
        TRI                *tri;
        double              a[3], dp2[3], ds2[3];
        int                dir, index, icl[3];
        struct _CRXING     *prev, *next;
};
typedef struct _CRXING CRXING;

enum _CRX_TYPE {
        SINGLE = 0,
        MULTIPLE = 1
};
typedef enum _CRX_TYPE CRX_TYPE;

typedef struct {
        double           coords[4];
        FT_POINT           *vertex;
        FT_POINT           *edge[2];
} CRX_STORE;

/*#bjet2 */
enum _EDGE_FLAG {
        INSIDE_WALL = 0,
        ON_WALL = 1,
	OUTSIDE_WALL = 2
};
typedef enum _EDGE_FLAG EDGE_FLAG;

enum _DOMAIN_FLAG {
        INSIDE_DOMAIN = -3,
	OUTSIDE_DOMAIN = -2,
	UNKNOWN_DOMAIN = -1
};
typedef enum _DOMAIN_FLAG DOMAIN_FLAG;


enum {
        MAX_NUM_CRX  = 100, /* Maximum number of crossing on a block side */
        MAX_CRX_FILL = 20  /* Maximum number of missing crxings to be filled */
};

        /* compass directions on grid */ 
        /* NOTE: These side and corner definitions allow bitwise comparisons */        /* ie:   NORTH_WEST == NORTH & WEST,  etc */

enum _GRID_DIRECTION {
        EAST  = 0x01,
        WEST  = 0x02,
        NORTH = 0x04,
        SOUTH = 0x08,
        UPPER = 0x10,
        LOWER = 0x20
};
typedef enum _GRID_DIRECTION GRID_DIRECTION;

#define	Is_outside(icoord,G,dir)					\
	((icoord)[(dir)] < 0 || (icoord)[(dir)] >= (G)[(dir)])

 /* Macros for accessing hyper surface data */

#define	hyper_surf_list(intfc) (intfc)->hss
#define Hyper_surf(ptr)	       ((ptr)->hs)
#define Hyper_surf_index(ptr)  Hyper_surf(ptr)->pp_index

#define Point_of_hs(hs)	       (hs)->obj.p

#define Curve_of_hs(hs)	       (hs)->obj.c

#define Surface_of_hs(hs)      (hs)->obj.s



 /* Macros for accessing hyper surface element data */
#define Hyper_surf_element(ptr)	((HYPER_SURF_ELEMENT *) (ptr))
#define Bond_of_hse(hse)	((BOND *) hse)
#define Tri_of_hse(hse)		((TRI *) hse)

/* Macros for accessing hyper surface boundary data */
#define	hyper_surf_bdry_list(intfc)	(intfc)->hses
#define Hyper_surf_bdry(ptr)   		((ptr)->hsb)

#define Node_of_hsb(hsb)       		(hsb)->obj.n

#define Curve_of_hsb(hsb)      		(hsb)->obj.c


/*
*		Node, Curve and Surface Boundary Flags:
*
*	These flags use 6 bits, 2 per coordinate direction.
*	The boundary flags are stored in bits, the first 2 for the x 
*	coordinate, the next 2 for the y and the third 2 for z. A two bit
*	flag is thus 0 = INSIDE, 1 = BOTTOM, 2 = TOP, 3 unft_assigned. Thus
*	the upper x,y,z corner has the binary flag: 101010 = (octal) 022,
*	and the upper z face has the binary flag: 100000 = (octal) 040.
*
*	In summary,  each flag is a binary number of the form:
*	flag = (zzyyxx),  where zz, yy, or xx = 00, 01, 10, or 11 (binary).
*
*	IMPORTANT NOTE:	The following macro assumes that the boundary field
*	is the first element of any of the possible objects to which
*	hs or hsb can point.  The current legal objects are
*
*		SURFACE, CURVE, NODE (2d only), and FT_POINT (1d only)
*
*	The arrangement of data allows the boundary information to
*	be extracted from these structures without an explicit knowledge
*	of the spatial dimension.  This in turn provides an increase
*	in run time efficiency.
*
*	Note:  In many cases, the specific boundary is not specified.  In
*	fact, as of this writing (9/23/96), only parts of the 3d code use the
*	mapping described above.  The rest of the time, we only care whether
*	the object is on the boundary or not.  A set of macros is provided
*	below using BDRY_MASK to serve this purpose.  They set or check all
*	six of the lower order bits.  This is necessary because the higher
*	order bits are being used in certain parts of the code.  This will
*	require some care.  Some of the bit field declarations may be local
*	to facilitate development, and due to the fact that they are used
*	only locally. These fields may need to be preserved globally, however,
*	which is the reason for the BDRY_MASK macros below.  One cannot
*	simply set theboundary field to YES or NO, as this will not preserve
*	the higher order bits.	
*/

enum {
	INSIDE 	  = 000,
	BOTTOM	  = 001,
	TOP	  = 002,
	BDRY_MASK = 077
};

#define Boundary(ptr)		(ptr)->_boundary
#define Boundary_hs(hs)		(*(hs )->obj.bptr)
#define Boundary_hsb(hsb)	(*(hsb)->obj.bptr)

#define set_is_bdry(ptr)	(Boundary(ptr)     |=  BDRY_MASK)
#define set_not_bdry(ptr)	(Boundary(ptr)     &= ~BDRY_MASK)
#define is_bdry(ptr)		(Boundary(ptr)     &   BDRY_MASK)
#define is_bdry_hs(ptr)		(Boundary_hs(ptr)  &   BDRY_MASK)
#define is_bdry_hsb(ptr)	(Boundary_hsb(ptr) &   BDRY_MASK)

	/* Macros for evaluation of boundary fields */
#define xbdry_side(f) 	((f)        & 03)
#define ybdry_side(f)	((f >> 2)   & 03)
#define zbdry_side(f)	((f >> 4)   & 03)
#define bdry_side(f,i)	((f >> 2*i) & 03)

	/* Macros for setting of boundary fields */
#define set_xbdry_side(f,v)						\
	( (f) = ( ((f) & ~03         ) | (((v) & 03)       ) ) )
#define set_ybdry_side(f,v)						\
	( (f) = ( ((f) & ~014        ) | (((v) & 03) << 2  ) ) )
#define set_zbdry_side(f,v)						\
	( (f) = ( ((f) & ~060        ) | (((v) & 03) << 4  ) ) )
#define set_bdry_side(f,d,v)						\
	( (f) = ( ((f) & ~(03 << 2*d)) | (((v) & 03) << 2*d) ) )



 /* Macros for accessing component information Note: */
 /* negative side is the lefe side and positive side is the right side */

#define positive_component(ptr)	(Hyper_surf(ptr)->pos_comp)
#define negative_component(ptr)	(Hyper_surf(ptr)->neg_comp)

#define change_curve_comp(ptr)          (Hyper_surf(ptr)->dyn_comp_ch.change)
#define change_curve_comp_on_side(ptr)  (Hyper_surf(ptr)->dyn_comp_ch.side)
#define change_curve_comp_by(ptr)       (Hyper_surf(ptr)->dyn_comp_ch.new_comp)

#define	comp_is_on_hyper_surf(hs,comp)					\
		(((comp) == NO_COMP) || 				\
		(negative_component(hs) == (comp)) || 			\
		(positive_component(hs) == (comp)))

#define	comps_are_on_hyper_surf(hs,l_comp,r_comp)			\
	((((l_comp)==NO_COMP) || (negative_component(hs)==(l_comp)))	\
	&&								\
	(((r_comp)==NO_COMP) || (positive_component(hs)==(r_comp))))


 /* Sides of Boundary Square -- for nearest_boundary() */

enum _BDRY_SIDE {
    NOT_A_BDRY  = -1,
    LEFT_BDRY	=  0,
    RIGHT_BDRY	=  1,
    LOWER_BDRY	=  2,
    UPPER_BDRY	=  3,
    ZMIN_BDRY	=  4,
    ZMAX_BDRY	=  5
};
typedef enum _BDRY_SIDE BDRY_SIDE;

 /* Values for rectangular boundary type */

#define rect_boundary_type(intfc,i,j)	((intfc)->rect_bdry_type[i][j])

enum {
	UNKNOWN_BOUNDARY_TYPE = -3,
	SUBDOMAIN_BOUNDARY    =  1,
	PERIODIC_BOUNDARY     =  1,
	REFLECTION_BOUNDARY,
	MIXED_TYPE_BOUNDARY,
	OPEN_BOUNDARY,
	FIRST_USER_BOUNDARY_TYPE
#if defined(USE_OVERTURE)
        AMR_SUBDOMAIN_BOUNDARY = 100000
#endif /* defined(USE_OVERTURE) */
};

#if defined(USE_OVERTURE)
#define buffered_boundary_type(b_type)                                  \
        (((b_type) == SUBDOMAIN_BOUNDARY) || ((b_type) == REFLECTION_BOUNDARY) \
         || ((b_type) == AMR_SUBDOMAIN_BOUNDARY))
#else /* if defined(USE_OVERTURE) */
#define buffered_boundary_type(b_type)                                  \
        (((b_type) == SUBDOMAIN_BOUNDARY) || ((b_type) == REFLECTION_BOUNDARY) \
	 || ((b_type) == OPEN_BOUNDARY))
#endif /* if defined(USE_OVERTURE) */

#define	is_pp_node(node)						\
	(is_subdomain_node(node) || is_virtual_fixed_node(node))

enum {
	NO_COMP          = -1,
	ONFRONT          = -2,
	NEW_COMP         = -3,
	ON_RECT_BOUNDARY = -4,
	UNUSED_COMP      = -5,
	ERROR_COMP       = -6
};

enum _USE_BOUNDARIES {
	NO_BOUNDARIES         = 0,
	INCLUDE_BOUNDARIES    = 1,
	NO_SUBDOMAIN          = 2
};
typedef enum _USE_BOUNDARIES USE_BOUNDARIES;

#define	skip_boundary_hs(hs,bdry)					\
	(((bdry == NO_BOUNDARIES) && is_bdry_hs(hs)) ||			\
	 ((bdry == NO_SUBDOMAIN) && is_subdomain_boundary(hs)))


#define	min3(xx,yy,zz) 	(min((xx),min((yy),(zz))))
#define	max3(xx,yy,zz) 	(max((xx),max((yy),(zz))))
#define	min4(ww,xx,yy,zz) (min((ww),min3((xx),(yy),(zz))))
#define	max4(ww,xx,yy,zz) (max((ww),max3((xx),(yy),(zz))))
#define same_sign(c1,c2)                                                \
        ((((c1) > 0.0 && (c2) > 0.0) || ((c1) < 0.0 && (c2) < 0.0)) ? \
        YES : NO)

#define within_interval(x1,x2,x) 				\
	(((x1) <= (x) && (x) <= (x2)) || ((x1) >= (x) && (x) >= (x2)) \
	? YES : NO)


enum {
	MAXNCORNERS = (1<<MAXD)	/* MAX no. corners */
};
#define	Ncorners(dim)		(1<<dim)	/* no. corners of hyper cube */

#define Dot3d(A,B)							\
	((A)[0]*(B)[0] + (A)[1]*(B)[1] + (A)[2]*(B)[2])

#define	Mag3d(A) sqrt(Dot3d(A,A))

#define QDot3d(A,B)							\
	(A ## 0*B ## 0 + A ## 1*B ## 1 + A ## 2*B ## 2)

#define Cross2d(B,C,ans)						\
	{								\
		(ans) = ((B)[0])*((C)[1]) - ((B)[1])*((C)[0]);		\
	}

#define Cross3d(B,C,ans)						\
	{								\
		(ans)[0] = ((B)[1])*((C)[2]) - ((B)[2])*((C)[1]);	\
		(ans)[1] = ((B)[2])*((C)[0]) - ((B)[0])*((C)[2]);	\
		(ans)[2] = ((B)[0])*((C)[1]) - ((B)[1])*((C)[0]);	\
	}

#define QCross3d(B,C,ans)						\
		ans ## 0 = B ## 1*C ## 2 - B ## 2*C ## 1;		\
		ans ## 1 = B ## 2*C ## 0 - B ## 0*C ## 2;		\
		ans ## 2 = B ## 0*C ## 1 - B ## 1*C ## 0

#define Det3d(a,b,c) ( (a)[0]*(b)[1]*(c)[2] + (a)[1]*(b)[2]*(c)[0] + (a)[2]*(b)[0]*(c)[1] - (a)[0]*(b)[2]*(c)[1] - (a)[1]*(b)[0]*(c)[2] - (a)[2]*(b)[1]*(c)[0] )

#define QDet3d(a,b,c) ( a ## 0*b ##1*c ## 2 + a ## 1*b ## 2*c ## 0 + a ## 2*b ## 0*c ## 1 - a ## 0*b ## 2*c ## 1 - a ## 1*b ## 0*c ## 2 - a ## 2*b ## 1*c ## 0 )

#define difference(B,C,ans,dim)						\
	{								\
	    int i;							\
	    for (i = 0; i < (dim); ++i)					\
	    {								\
		(ans)[i] = (B)[i] - (C)[i];				\
	    }								\
	}

 /* Position of a Point as an array */

#define Coords(p)	((p)->_coords)
#define COORDS(P)	((P)._coords)

/* Stored normal vector at point */
#define	normal_at_point(p) ((p)->_nor)

 /* Length of a Bond */

#define  bond_length(b)  ((b)->length)
#define  bond_length0(b)  ((b)->length0)	/* for fixed length bond */

 /* Separation between two FT_POINTS: */

#define scaled_separation(p,q,h,dim)					\
	_scaled_separation(Coords(p),Coords(q),h,dim)

IMPORT double separation(FT_POINT *p, FT_POINT *q, int dim);


 /* Is a CURVE closed ? */

#define	 is_closed_curve(c)	 ((c)->start == (c)->end)


 /* Is a NODE the joining point of a simple closed curve? */

#define  is_node_of_closed_curve_only(node) 			\
	 (    ((node)->in_curves != NULL)			\
	   && ((node)->out_curves != NULL)			\
	   && (node)->in_curves[0]				\
	   && ((node)->in_curves[0] == (node)->out_curves[0])	\
	   && !(node)->in_curves[1]				\
	   && !(node)->out_curves[1]    )

 /* The bond on a given curve that attaches to a node */

#define Prev_bond(b,c)  ((b)->prev != NULL) ? (b)->prev : \
        (is_closed_curve((c)) ? (c)->last : NULL)

#define Next_bond(b,c)  ((b)->next != NULL) ? (b)->next : \
        (is_closed_curve((c)) ? (c)->first : NULL)

#define Bond_at_node(curve,orient)					\
	((orient) == POSITIVE_ORIENTATION ? (curve)->first : (curve)->last)

#define Bond_at_node_of_o_curve(oc)					\
	((oc)->orient == POSITIVE_ORIENTATION ? (oc)->curve->first : 	\
					       (oc)->curve->last)

#define Bond_at_opp_node_of_o_curve(oc)					\
	((oc)->orient == NEGATIVE_ORIENTATION ? (oc)->curve->first : 	\
						(oc)->curve->last)


 /* Returns the bond following a bond, as defined by the orientation */

#define Following_bond(bond,orient)					\
	((orient) == POSITIVE_ORIENTATION ? (bond)->next : (bond)->prev)


 /* Returns the point associated with a bond and an orientation */

#define Point_of_bond(b,orient)						\
	(((orient) == POSITIVE_ORIENTATION) ? (b)->start : (b)->end)


 /* Returns the first point out along the curve from the node */

#define Point_adjacent_to_node(curve,orient)				\
	((orient) == POSITIVE_ORIENTATION				\
		? (curve)->first->end : (curve)->last->start)


 /* Returns the node associated with a curve and an orientation */

#define Node_of(curve,orient)						\
	((orient) == POSITIVE_ORIENTATION ? (curve)->start : (curve)->end)

#define Opp_node_of(curve,orient)					\
	((orient) == POSITIVE_ORIENTATION ? (curve)->end : (curve)->start)

#define Node_of_o_curve(oc)						\
	(((oc)->orient == POSITIVE_ORIENTATION) ? (oc)->curve->start : 	\
						 (oc)->curve->end)

#define Opp_node_of_o_curve(oc)						\
	(((oc)->orient == NEGATIVE_ORIENTATION) ? (oc)->curve->start : 	\
						  (oc)->curve->end)

 /* Point flags fields for points */
#define Point_flags(p)		((p)->_point_flags)
#define	Boundary_point(p)	(Point_flags(p)._boundary)

#define Btris(b)		(b)->_btris
#define Point_of_tri(_tri_)	(_tri_)->__pts

/*	Macros controling tri list on surfaces */
#define surface_for_head_of_tri_list(htl) ((TRI_LIST_HEAD *) htl)->s
#define head_of_tri_list(s) (&(s)->_First_tri.tri)
#define tail_of_tri_list(s) (&(s)->_Last_tri)
#define first_tri(s)	(s)->_First_tri.tri.next
#define last_tri(s)	(s)->_Last_tri.prev
#define at_end_of_tri_list(tri,s)	((tri) == tail_of_tri_list(s))
#define at_start_of_tri_list(tri,s)	((tri) == head_of_tri_list(s))
#define no_tris_on_surface(s)		(at_end_of_tri_list(first_tri(s),s))



/*
*			TRI boundary flags:
*
*	The flag Boundary_tri(tri) flag specifies which sides of the triangle
*	lie on curves of the interface, according to the following
*	truth table.
*
*		   0  1  2  3  4  5  6  7
*	side 01:   N  Y  N  Y  N  Y  N  Y
*	side 12:   N  N  Y  Y  N  N  Y  Y
*	side 20:   N  N  N  N  Y  Y  Y  Y
*
*/

#define Boundary_tri(tri)	(tri)->boundary
#define	Bin_side(side) 	(1<<(side))

enum {
	BIN_SIDE01 = 0x1,
	BIN_SIDE12 = 0x2,
	BIN_SIDE20 = 0x4
};

#define is_side01_a_bond(_tri_)		(Boundary_tri(_tri_) & BIN_SIDE01)
#define is_side12_a_bond(_tri_)		(Boundary_tri(_tri_) & BIN_SIDE12)
#define is_side20_a_bond(_tri_)		(Boundary_tri(_tri_) & BIN_SIDE20)
#define is_side_bdry(_tri_,side)	(Boundary_tri(_tri_) & Bin_side(side))

#define set_01_bdry(bdry,yes_no)					\
	(bdry) = (((bdry) & ~BIN_SIDE01) | (yes_no))
#define set_12_bdry(bdry,yes_no)					\
	(bdry) = (((bdry) & ~BIN_SIDE12) | ((yes_no) << 1))
#define set_20_bdry(bdry,yes_no)					\
	(bdry) = (((bdry) & ~BIN_SIDE20) | ((yes_no) << 2))
#define set_side_bdry(bdry,side,yes_no)					\
	(bdry) = ( ((bdry) & ~ Bin_side(side)) | ((yes_no) << (side)) )

#define vertex_on_bond(_tri_,i)						\
	( Boundary_tri(_tri_) & (Bin_side(i) & Bin_side(Prev_m3(i))) )

 /* Macros for accessing tri data fields */

enum {
	GLOBAL_INDICES = 0,
	LOCAL_INDICES  = 1
};

#define Tri_order(_tri_)                ((_tri_)->order)
#define	Surface_of_tri(_tri_)		((_tri_)->surf)
#define Tri_workspace(_tri_)		((_tri_)->private_data._workspace)
#define Tri_index(_tri_)		((_tri_)->private_data._index)
#define Tri_cross_list(_tri_)		((_tri_)->private_data._tri_cross_list)
#define	Tri_icoords(_tri_)		((_tri_)->private_data._icoords)
#define	Tri_modified(_tri_)		((_tri_)->private_data._modified)
#define	Tri_projection_computed(_tri_)					\
    ((_tri_)->private_data._projection_computed)

#define	Tri_neighbor(_tri_)	     (_tri_)->neighbor
#define	Tri_on_side01(_tri_)	     (Tri_neighbor(_tri_)[0].tri)
#define	Tri_on_side12(_tri_)	     (Tri_neighbor(_tri_)[1].tri)
#define	Tri_on_side20(_tri_)	     (Tri_neighbor(_tri_)[2].tri)
#define	Tri_on_side(_tri_,side)	     ((Tri_neighbor(_tri_) + (side))->tri)

#define	Bond_tri_on_side01(_tri_)    (Tri_neighbor(_tri_)[0].btri)
#define	Bond_tri_on_side12(_tri_)    (Tri_neighbor(_tri_)[1].btri)
#define	Bond_tri_on_side20(_tri_)    (Tri_neighbor(_tri_)[2].btri)
#define	Bond_tri_on_side(_tri_,side) ((Tri_neighbor(_tri_) + (side))->btri)

#define	Bond_on_side01(_tri_)	     (Bond_tri_on_side01(_tri_)->bond)
#define	Bond_on_side12(_tri_)	     (Bond_tri_on_side12(_tri_)->bond)
#define	Bond_on_side20(_tri_)	     (Bond_tri_on_side20(_tri_)->bond)
#define	Bond_on_side(_tri_,side)     (Bond_tri_on_side(_tri_,side)->bond)

#define	Neighbor_on_side01(_tri_)    (Tri_neighbor(_tri_)[0].ptr)
#define	Neighbor_on_side12(_tri_)    (Tri_neighbor(_tri_)[1].ptr)
#define	Neighbor_on_side20(_tri_)    (Tri_neighbor(_tri_)[2].ptr)
#define	Neighbor_on_side(_tri_,side) ((Tri_neighbor(_tri_) + (side))->ptr)

#define	Index_on_side01(_tri_)	     (Tri_neighbor(_tri_)[0].index)
#define	Index_on_side12(_tri_)	     (Tri_neighbor(_tri_)[1].index)
#define	Index_on_side20(_tri_)	     (Tri_neighbor(_tri_)[2].index)
#define	Index_on_side(_tri_,side)    ((Tri_neighbor(_tri_) + (side))->index)

#define Point_on_tri(_tri_,p)						\
	(((p)==Point_of_tri(_tri_)[0]) ||                               \
	 ((p)==Point_of_tri(_tri_)[1]) || 				\
	 ((p)==Point_of_tri(_tri_)[2]))

#define Vertex_of_point(_tri_,p)					\
	( (((p)==Point_of_tri(_tri_)[0])) ? 0 :				\
	  (((p)==Point_of_tri(_tri_)[1])) ? 1 :				\
	  (((p)==Point_of_tri(_tri_)[2])) ? 2 : ERROR)

#define Prev_side_at_vertex(_tri_,p)					\
	(Point_on_tri(_tri_,p) ? Prev_m3(Vertex_of_point(_tri_,p)) 	\
	: ERROR)

#define Next_side_at_vertex(_tri_,p)					\
	(Point_on_tri(_tri_,p) ? Vertex_of_point(_tri_,p) : ERROR)

#define Following_tri_at_vertex(_tri_,p,dir)				\
	( ((dir) == COUNTER_CLOCK) ?	Prev_tri_at_vertex(_tri_,p)	\
	: ((dir) == CLOCKWISE)	   ?	Next_tri_at_vertex(_tri_,p)	\
	:				NULL				)

#define Previous_side_at_vertex(_tri_,p,dir)				\
	( ((dir) == COUNTER_CLOCK) ?	Prev_side_at_vertex(_tri_,p)	\
	: ((dir) == CLOCKWISE)	   ?	Next_side_at_vertex(_tri_,p)	\
	:				ERROR				)

#define Previous_tri_at_vertex(_tri_,p,dir)				\
	( ((dir) == CLOCKWISE)	   ?	Prev_tri_at_vertex(_tri_,p)	\
	: ((dir) == COUNTER_CLOCK) ?	Next_tri_at_vertex(_tri_,p)	\
	:				NULL				)

#define Following_side_at_vertex(_tri_,p,dir)				\
	( ((dir) == CLOCKWISE)	   ?	Prev_side_at_vertex(_tri_,p)	\
	: ((dir) == COUNTER_CLOCK) ?	Next_side_at_vertex(_tri_,p)	\
	:				ERROR				)


#define Next_corner(n,nc)       (((n) + 1) % (nc))

#define Prev_corner(n,nc)       (((n) + (nc) - 1) % (nc))

#include <intfc/array.h>

#define Num_pos_curves_of_surface(surface)				\
		size_of_pointers((POINTER *)(surface)->pos_curves)

#define Num_neg_curves_of_surface(surface)				\
		size_of_pointers((POINTER *)(surface)->neg_curves)

#define Num_surfaces_bding_curve(curve)					\
		(Num_pos_surfaces_of_curve(curve) +			\
			Num_neg_surfaces_of_curve(curve))

#define Num_pos_surfaces_of_curve(curve)				\
		size_of_pointers((POINTER *)(curve)->pos_surfaces)

#define Num_neg_surfaces_of_curve(curve)				\
		size_of_pointers((POINTER *)(curve)->neg_surfaces)

#define Num_surfaces(intfc) size_of_pointers((POINTER *)(intfc)->surfaces)

#define Num_nodes(intfc) size_of_pointers((POINTER *)(intfc)->nodes)

#define Num_curves(intfc) size_of_pointers((POINTER *)(intfc)->curves)

#define Num_in_curves(node) size_of_pointers((POINTER *)(node)->in_curves)

#define Num_out_curves(node) size_of_pointers((POINTER *)(node)->out_curves)

#define new_address(intfc,p,ocad,ncad,nchks)				\
		_new_address(intfc,(POINTER) (p),(ocad),(ncad),(nchks))

enum {
	TABLE_ID = USER_MIN_MESG_ID + 1000,
	CHUNK_ADDR_ID,
	CHUNK0_ID,
	ARRAY_ID = USER_MIN_MESG_ID + 1000,
	FIRST_USER_MESSAGE_ID = USER_MIN_MESG_ID + 10000
};

#define chunk_id(i)		(CHUNK0_ID + (i))
#define array_id(i)		(ARRAY_ID + (i))

		/* Structure Defining a Parallel Processing Grid: */

typedef struct {
	double   *dom[MAXD];     /* corner position of domain */
	int     gmax[MAXD];     /* # of subdomains in each dir */
	int	buf[MAXD];	/* shaded subdomain extension. every mesh
				block has the same buffer extension.	*/
	int	nn;		/* total number of nodes 		*/	
	RECT_GRID Global_grid;	/* Rect_grid of total region		*/
	RECT_GRID Zoom_grid;	/* Rect_grid for subdomain		*/
} PP_GRID; 

/*
*	Initialization structure for interface library
*/

struct _I_INIT_DATA {
	INIT_DATA	U_init_data;
	INTERFACE	*_intfc;
	int		_subdomains[3];		/*COMMAND LINE*/
	int		_buffer_zones[3];	/*COMMAND LINE*/
	boolean		_pp_grid_set;		/*COMMAND LINE*/
};
typedef struct _I_INIT_DATA I_INIT_DATA;
#define	i_init_data(init)	((I_INIT_DATA*)(init))
#define i_intfc(init)		i_init_data(init)->_intfc
#define Top_grid(init)      	topological_grid(i_intfc(init))
#define	subdomains(init)	i_init_data(init)->_subdomains
#define	buffer_zones(init)	i_init_data(init)->_buffer_zones
#define	pp_grid_set(init)	i_init_data(init)->_pp_grid_set

enum _COORDINATE_PLANE {
    YZ_PLANE = 0,
    XZ_PLANE = 1,
    XY_PLANE = 2
};
typedef enum _COORDINATE_PLANE COORDINATE_PLANE;

enum _SURFACE_COLOR {
	pBLACK   = 0,
	pRED	 = 1,
	pBLUE	 = 2,
	pMAGENTA = 3,
	pGREEN	 = 4,
	pYELLOW	 = 5,
	pCYAN	 = 6,
	pWHITE	 = 7
};
typedef enum _SURFACE_COLOR SURFACE_COLOR;

/*
*       Tolerances used in constructing the grid_intfc function.
*
*/

#define _NTOL 1.0e-5
#define _TOL  1.0e-6

#define IG_NTOL  _NTOL  /*TOLERANCE */
#define IG_TOL   _TOL  /*TOLERANCE */
#define IG_ONEMNTOL  (1.0 - _NTOL) /*TOLERANCE */
#define IG_ONEMTOL   (1.0 - _TOL) /*TOLERANCE */

/* smooth parameters */
typedef struct {
        double   cor, cos;
        FT_POINT   *pt;
        TRI     *tri;
        double   avep[3];
}       SMOOTH_PARA;

typedef struct {
        double   cone_ratio, max_cos;
        double   alpha;          /*alpha must be irrational. */
}       SMOOTH_TOL;

struct _BBI_POINT2 {             /* Block based interface point */
        HYPER_SURF      *hs;
        CURVE           *c;
        FT_POINT           *p;
        COMPONENT       lcomp, ucomp;   /* add for 2d bond construction */
};
typedef struct _BBI_POINT2 BBI_POINT2;

struct _BLK_CRX2 {
        int             blkic[MAXD]; /* blk icoords */
        COMPONENT       ***comp;
        int             ***ix;
        int             ***iy;
        int             ***iz;
        COMPONENT       comps[4];   /* save different comp index of this 
				     * blk corner */
        int             num_comps; 
        int             num_curves; /* Number of curves inside this blk */
        int             num_waves;  /* The number of different waves inside 
				     * blk*/
                                    /* Different curves may belong to the 
				     * same wave */
        COMPONENT       pos_comp;
        COMPONENT       neg_comp;
        BBI_POINT2      ****crx;    /* For the record, current max. # of 
				     * crx on a grid line
                                     * is limited to 5 */
        BBI_POINT2      *crx_store;
        int             ***n_crx;   /* Number of CRX on each grid line, multi 
			    	     * crxs are possible */
        BLK_TYPE        blk_type;   
        int             bdry;       /* The flags whether the boundary curves 
				     * appear in blk */
        int             m_crx;      /* If there are mixed types of 
				     * crxings(tracked, untracked), 
                                     * m_crx = YES */
        void        (*assign_2dblk_type)(struct _BLK_CRX2*,int,
					BBI_POINT2 *crxs[]);
        int         (*trk_wv_val)(CURVE*); 
};
typedef struct _BLK_CRX2 BLK_CRX2;

struct _EQUIV_COMPS {
        int                     n_equiv;        /* num elements in *comp */
        COMPONENT               *comp;          /* an array of equiv objs */
        struct  _EQUIV_COMPS    *prev, *next;
};
typedef struct  _EQUIV_COMPS    EQUIV_COMPS;

        /* Statuses returned by in_comp_equiv_list() */
enum _COMP_EQUIV_LIST {
        NEITHER_FOUND = 0,
        COMP1_FOUND,
        COMP2_FOUND,
        BOTH_FOUND
};
typedef enum _COMP_EQUIV_LIST COMP_EQUIV_LIST;

enum { EQUIV_COMPS_LEN  = 5 };

#define E_comps(intfc)          ((EQUIV_COMPS *) (intfc)->e_comps)

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#include <intfc/table.h>
#include <intfc/userint.h>
#include <intfc/iprotos.h>

#include <intfc/int_amr.h>

#endif /* !defined(_INT_H) */
