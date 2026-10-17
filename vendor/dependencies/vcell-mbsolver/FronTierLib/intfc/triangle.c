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


/*****************************************************************************/
/*                                                                           */
/*      888888888        ,o,                          / 888                  */
/*         888    88o88o  "    o8888o  88o8888o o88888o 888  o88888o         */
/*         888    888    888       88b 888  888 888 888 888 d888  88b        */
/*         888    888    888  o88^o888 888  888 "88888" 888 8888oo888        */
/*         888    888    888 C888  888 888  888  /      888 q888             */
/*         888    888    888  "88o^888 888  888 Cb      888  "88oooo"        */
/*                                              "8oo8D                       */
/*                                                                           */
/*  A Two-Dimensional Quality Mesh Generator and Delaunay Triangulator.      */
/*  (triangle.c)                                                             */
/*                                                                           */
/*  Version 1.3                                                              */
/*  July 19, 1996                                                            */
/*                                                                           */
/*  Copyright 1996                                                           */
/*  Jonathan Richard Shewchuk                                                */
/*  School of Computer Science                                               */
/*  Carnegie Mellon University                                               */
/*  5000 Forbes Avenue                                                       */
/*  Pittsburgh, Pennsylvania  15213-3891                                     */
/*  jrs@cs.cmu.edu                                                           */
/*                                                                           */
/*  This program may be freely redistributed under the condition that the    */
/*    copyright notices (including this entire header and the copyright      */
/*    notice printed when the `-h' switch is selected) are not removed, and  */
/*    no compensation is received.  Private, research, and institutional     */
/*    use is free.  You may distribute modified versions of this code UNDER  */
/*    THE CONDITION THAT THIS CODE AND ANY MODIFICATIONS MADE TO IT IN THE   */
/*    SAME FILE REMAIN UNDER COPYRIGHT OF THE ORIGINAL AUTHOR, BOTH SOURCE   */
/*    AND OBJECT CODE ARE MADE FREELY AVAILABLE WITHOUT CHARGE, AND CLEAR    */
/*    NOTICE IS GIVEN OF THE MODIFICATIONS.  Distribution of this code as    */
/*    part of a commercial system is permissible ONLY BY DIRECT ARRANGEMENT  */
/*    WITH THE AUTHOR.  (If you are not directly supplying this code to a    */
/*    customer, and you are instead telling them how they can obtain it for  */
/*    free, then you are not required to make any arrangement with me.)      */
/*                                                                           */
/*  Hypertext instructions for Triangle are available on the Web at          */
/*                                                                           */
/*      http://www.cs.cmu.edu/~quake/triangle.html                           */
/*                                                                           */
/*  Some of the references listed below are marked [*].  These are available */
/*    for downloading from the Web page                                      */
/*                                                                           */
/*      http://www.cs.cmu.edu/~quake/triangle.research.html                  */
/*                                                                           */
/*  A paper discussing some aspects of Triangle is available.  See Jonathan  */
/*    Richard Shewchuk, "Triangle:  Engineering a 2D Quality Mesh Generator  */
/*    and Delaunay Triangulator," First Workshop on Applied Computational    */
/*    Geometry, ACM, May 1996.  [*]                                          */
/*                                                                           */
/*  Triangle was created as part of the Archimedes project in the School of  */
/*    Computer Science at Carnegie Mellon University.  Archimedes is a       */
/*    system for compiling parallel finite element solvers.  For further     */
/*    information, see Anja Feldmann, Omar Ghattas, John R. Gilbert, Gary L. */
/*    Miller, David R. O'Hallaron, Eric J. Schwabe, Jonathan R. Shewchuk,    */
/*    and Shang-Hua Teng, "Automated Parallel Solution of Unstructured PDE   */
/*    Problems."  To appear in Communications of the ACM, we hope.           */
/*                                                                           */
/*  The quality mesh generation algorithm is due to Jim Ruppert, "A          */
/*    Delaunay Refinement Algorithm for Quality 2-Dimensional Mesh           */
/*    Generation," Journal of Algorithms 18(3):548-585, May 1995.  [*]       */
/*                                                                           */
/*  My implementation of the divide-and-conquer and incremental Delaunay     */
/*    triangulation algorithms follows closely the presentation of Guibas    */
/*    and Stolfi, even though I use a triangle-based data structure instead  */
/*    of their quad-edge data structure.  (In fact, I originally implemented */
/*    Triangle using the quad-edge data structure, but switching to a        */
/*    triangle-based data structure sped Triangle by a factor of two.)  The  */
/*    mesh manipulation primitives and the two aforementioned Delaunay       */
/*    triangulation algorithms are described by Leonidas J. Guibas and Jorge */
/*    Stolfi, "Primitives for the Manipulation of General Subdivisions and   */
/*    the Computation of Voronoi Diagrams," ACM Transactions on Graphics     */
/*    4(2):74-123, April 1985.                                               */
/*                                                                           */
/*  Their O(n log n) divide-and-conquer algorithm is adapted from Der-Tsai   */
/*    Lee and Bruce J. Schachter, "Two Algorithms for Constructing the       */
/*    Delaunay Triangulation," International Journal of Computer and         */
/*    Information Science 9(3):219-242, 1980.  The idea to improve the       */
/*    divide-and-conquer algorithm by alternating between vertical and       */
/*    horizontal cuts was introduced by Rex A. Dwyer, "A Faster Divide-and-  */
/*    Conquer Algorithm for Constructing Delaunay Triangulations,"           */
/*    Algorithmica 2(2):137-151, 1987.                                       */
/*                                                                           */
/*  The incremental insertion algorithm was first proposed by C. L. Lawson,  */
/*    "Software for C1 Surface Interpolation," in Mathematical Software III, */
/*    John R. Rice, editor, Academic Press, New York, pp. 161-194, 1977.     */
/*    For point location, I use the algorithm of Ernst P. Mucke, Isaac       */
/*    Saias, and Binhai Zhu, "Fast Randomized Point Location Without         */
/*    Preprocessing in Two- and Three-dimensional Delaunay Triangulations,"  */
/*    Proceedings of the Twelfth Annual Symposium on Computational Geometry, */
/*    ACM, May 1996.  [*]  If I were to randomize the order of point         */
/*    insertion (I currently don't bother), their result combined with the   */
/*    result of Leonidas J. Guibas, Donald E. Knuth, and Micha Sharir,       */
/*    "Randomized Incremental Construction of Delaunay and Voronoi           */
/*    Diagrams," Algorithmica 7(4):381-413, 1992, would yield an expected    */
/*    O(n^{4/3}) bound on running time.                                      */
/*                                                                           */
/*  The O(n log n) sweepline Delaunay triangulation algorithm is taken from  */
/*    Steven Fortune, "A Sweepline Algorithm for Voronoi Diagrams",          */
/*    Algorithmica 2(2):153-174, 1987.  A random sample of edges on the      */
/*    boundary of the triangulation are maintained in a splay tree for the   */
/*    purpose of point location.  Splay trees are described by Daniel        */
/*    Dominic Sleator and Robert Endre Tarjan, "Self-Adjusting Binary Search */
/*    Trees," Journal of the ACM 32(3):652-686, July 1985.                   */
/*                                                                           */
/*  The algorithms for exact computation of the signs of determinants are    */
/*    described in Jonathan Richard Shewchuk, "Adaptive Precision Floating-  */
/*    Point Arithmetic and Fast Robust Geometric Predicates," Technical      */
/*    Report CMU-CS-96-140, School of Computer Science, Carnegie Mellon      */
/*    University, Pittsburgh, Pennsylvania, May 1996.  [*]  (Submitted to    */
/*    Discrete & Computational Geometry.)  An abbreviated version appears as */
/*    Jonathan Richard Shewchuk, "Robust Adaptive Floating-Point Geometric   */
/*    Predicates," Proceedings of the Twelfth Annual Symposium on Computa-   */
/*    tional Geometry, ACM, May 1996.  [*]  Many of the ideas for my exact   */
/*    arithmetic routines originate with Douglas M. Priest, "Algorithms for  */
/*    Arbitrary Precision Floating Point Arithmetic," Tenth Symposium on     */
/*    Computer Arithmetic, 132-143, IEEE Computer Society Press, 1991.  [*]  */
/*    Many of the ideas for the correct evaluation of the signs of           */
/*    determinants are taken from Steven Fortune and Christopher J. Van Wyk, */
/*    "Efficient Exact Arithmetic for Computational Geometry," Proceedings   */
/*    of the Ninth Annual Symposium on Computational Geometry, ACM,          */
/*    pp. 163-172, May 1993, and from Steven Fortune, "Numerical Stability   */
/*    of Algorithms for 2D Delaunay Triangulations," International Journal   */
/*    of Computational Geometry & Applications 5(1-2):193-213, March-June    */
/*    1995.                                                                  */
/*                                                                           */
/*  For definitions of and results involving Delaunay triangulations,        */
/*    constrained and conforming versions thereof, and other aspects of      */
/*    triangular mesh generation, see the excellent survey by Marshall Bern  */
/*    and David Eppstein, "Mesh Generation and Optimal Triangulation," in    */
/*    Computing and Euclidean Geometry, Ding-Zhu Du and Frank Hwang,         */
/*    editors, World Scientific, Singapore, pp. 23-90, 1992.                 */
/*                                                                           */
/*  The time for incrementally adding PSLG (planar straight line graph)      */
/*    segments to create a constrained Delaunay triangulation is probably    */
/*    O(n^2) per segment in the worst case and O(n) per edge in the common   */
/*    case, where n is the number of triangles that intersect the segment    */
/*    before it is inserted.  This doesn't count point location, which can   */
/*    be much more expensive.  (This note does not apply to conforming       */
/*    Delaunay triangulations, for which a different method is used to       */
/*    insert segments.)                                                      */
/*                                                                           */
/*  The time for adding segments to a conforming Delaunay triangulation is   */
/*    not clear, but does not depend upon n alone.  In some cases, very      */
/*    small features (like a point lying next to a segment) can cause a      */
/*    single segment to be split an arbitrary number of times.  Of course,   */
/*    floating-point precision is a practical barrier to how much this can   */
/*    happen.                                                                */
/*                                                                           */
/*  The time for deleting a point from a Delaunay triangulation is O(n^2) in */
/*    the worst case and O(n) in the common case, where n is the degree of   */
/*    the point being deleted.  I could improve this to expected O(n) time   */
/*    by "inserting" the neighboring vertices in random order, but n is      */
/*    usually quite small, so it's not worth the bother.  (The O(n) time     */
/*    for random insertion follows from L. Paul Chew, "Building Voronoi      */
/*    Diagrams for Convex Polygons in Linear Expected Time," Technical       */
/*    Report PCS-TR90-147, Department of Mathematics and Computer Science,   */
/*    Dartmouth College, 1990.                                               */
/*                                                                           */
/*  Ruppert's Delaunay refinement algorithm typically generates triangles    */
/*    at a linear rate (constant time per triangle) after the initial        */
/*    triangulation is formed.  There may be pathological cases where more   */
/*    time is required, but these never arise in practice.                   */
/*                                                                           */
/*  The segment intersection formulae are straightforward.  If you want to   */
/*    see them derived, see Franklin Antonio.  "Faster Line Segment          */
/*    Intersection."  In Graphics Gems III (David Kirk, editor), pp. 199-    */
/*    202.  Academic Press, Boston, 1992.                                    */
/*                                                                           */
/*  If you make any improvements to this code, please please please let me   */
/*    know, so that I may obtain the improvements.  Even if you don't change */
/*    the code, I'd still love to hear what it's being used for.             */
/*                                                                           */
/*  Disclaimer:  Neither I nor Carnegie Mellon warrant this code in any way  */
/*    whatsoever.  This code is provided "as-is".  Use at your own risk.     */
/*                                                                           */
/*****************************************************************************/

/* For single precision (which will save some memory and reduce paging),     */
/*   define the symbol SINGLE by using the -DSINGLE compiler switch or by    */
/*   writing "#define SINGLE" below.                                         */
/*                                                                           */
/* For double precision (which will allow you to refine meshes to a smaller  */
/*   edge length), leave SINGLE undefined.                                   */
/*                                                                           */
/* Double precision uses more memory, but improves the resolution of the     */
/*   meshes you can generate with Triangle.  It also reduces the likelihood  */
/*   of a floating exception due to overflow.  Finally, it is much faster    */
/*   than single precision on 64-bit architectures like the DEC Alpha.  I    */
/*   recommend double precision unless you want to generate a mesh for which */
/*   you do not have enough memory.                                          */

#include <intfc/triangledefs.h>

/* On some machines, the exact arithmetic routines might be defeated by the  */
/*   use of internal extended precision floating-point registers.  Sometimes */
/*   this problem can be fixed by defining certain values to be volatile,    */
/*   thus forcing them to be stored to memory and rounded off.  This isn't   */
/*   a great solution, though, as it slows Triangle down.                    */
/*                                                                           */
/* To try this out, write "#define INEXACT volatile" below.  Normally,       */
/*   however, INEXACT should be defined to be nothing.  ("#define INEXACT".) */

#define INEXACT /* Nothing */
/* #define INEXACT volatile */

/* For efficiency, a variety of data structures are allocated in bulk.  The  */
/*   following constants determine how many of each structure is allocated   */
/*   at once.                                                                */

LOCAL const int TRIPERBLOCK = 4092;/* Number of triangles allocated at once. */
LOCAL const int SHELLEPERBLOCK = 508;/* Number of shell edges allocated at once. */
LOCAL const int POINTPERBLOCK = 4092; /* Number of points allocated at once. */
LOCAL const int VIRUSPERBLOCK = 1020; /* Number of virus triangles allocated at once. */

/* The point marker DEADPOINT is an arbitrary number chosen large enough to  */
/*   (hopefully) not conflict with user boundary markers.  Make sure that it */
/*   is small enough to fit into your machine's integer size.                */

LOCAL const int DEADPOINT = INT_MIN;

/* Two constants for algorithms based on random sampling.  Both constants    */
/*   have been chosen empirically to optimize their respective algorithms.   */

/* Used for the point location scheme of Mucke, Saias, and Zhu, to decide    */
/*   how large a random sample of triangles to inspect.                      */
LOCAL const int SAMPLEFACTOR = 11;

/* A number that speaks for itself, every kissable digit.                    */

#if defined(PI)
#undef PI
#endif /* defined(PI) */
LOCAL const double PI = 3.141592653589793238462643383279502884197169399375105820974944592308;


/* Labels that signify whether a record consists primarily of pointers or of */
/*   floating-point words.  Used to make decisions about data alignment.     */

/* Labels that signify the result of point location.  The result of a        */
/*   search indicates that the point falls in the interior of a triangle, on */
/*   an edge, on a vertex, or outside the mesh.                              */

enum locateresult {
    INTRIANGLE,
    ONEDGE,
    ONVERTEX,
    OUTSIDE
};
typedef enum locateresult locateresult;

/* Labels that signify the result of site insertion.  The result indicates   */
/*   that the point was inserted with complete success, was inserted but     */
/*   encroaches on a segment, was not inserted because it lies on a segment, */
/*   or was not inserted because another point occupies the same location.   */

enum insertsiteresult {
    SUCCESSFULPOINT,
    ENCROACHINGPOINT,
    VIOLATINGPOINT,
    DUPLICATEPOINT
};
typedef enum insertsiteresult insertsiteresult;

/* Labels that signify the result of direction finding.  The result          */
/*   indicates that a segment connecting the two query points falls within   */
/*   the direction triangle, along the left edge of the direction triangle,  */
/*   or along the right edge of the direction triangle.                      */

enum finddirectionresult {
    WITHIN,
    LEFTCOLLINEAR,
    RIGHTCOLLINEAR
};
typedef enum finddirectionresult finddirectionresult;

/* Labels that signify the result of the circumcenter computation routine.   */
/*   The return value indicates which edge of the triangle is shortest.      */

enum circumcenterresult {
    OPPOSITEORG,
    OPPOSITEDEST,
    OPPOSITEAPEX
};
typedef enum circumcenterresult circumcenterresult;

/*****************************************************************************/
/*                                                                           */
/*  The basic mesh data structures                                           */
/*                                                                           */
/*  There are three:  points, triangles, and shell edges (abbreviated        */
/*  `shelle').  These three data structures, linked by pointers, comprise    */
/*  the mesh.  A point simply represents a point in space and its properties.*/
/*  A triangle is a triangle.  A shell edge is a special data structure used */
/*  to represent impenetrable segments in the mesh (including the outer      */
/*  boundary, boundaries of holes, and internal boundaries separating two    */
/*  triangulated regions).  Shell edges represent boundaries defined by the  */
/*  user that triangles may not lie across.                                  */
/*                                                                           */
/*  A triangle consists of a list of three vertices, a list of three         */
/*  adjoining triangles, a list of three adjoining shell edges (when shell   */
/*  edges are used), an arbitrary number of optional user-defined floating-  */
/*  point attributes, and an optional area constraint.  The latter is an     */
/*  upper bound on the permissible area of each triangle in a region, used   */
/*  for mesh refinement.                                                     */
/*                                                                           */
/*  For a triangle on a boundary of the mesh, some or all of the neighboring */
/*  triangles may not be present.  For a triangle in the interior of the     */
/*  mesh, often no neighboring shell edges are present.  Such absent         */
/*  triangles and shell edges are never represented by NULL pointers; they   */
/*  are represented by two special records:  `dummytri', the triangle that   */
/*  fills "outer space", and `dummysh', the omnipresent shell edge.          */
/*  `dummytri' and `dummysh' are used for several reasons; for instance,     */
/*  they can be dereferenced and their contents examined without causing the */
/*  memory protection exception that would occur if NULL were dereferenced.  */
/*                                                                           */
/*  However, it is important to understand that a triangle includes other    */
/*  information as well.  The pointers to adjoining vertices, triangles, and */
/*  shell edges are ordered in a way that indicates their geometric relation */
/*  to each other.  Furthermore, each of these pointers contains orientation */
/*  information.  Each pointer to an adjoining triangle indicates which face */
/*  of that triangle is contacted.  Similarly, each pointer to an adjoining  */
/*  shell edge indicates which side of that shell edge is contacted, and how */
/*  the shell edge is oriented relative to the triangle.                     */
/*                                                                           */
/*  Shell edges are found abutting edges of triangles; either sandwiched     */
/*  between two triangles, or resting against one triangle on an exterior    */
/*  boundary or hole boundary.                                               */
/*                                                                           */
/*  A shell edge consists of a list of two vertices, a list of two           */
/*  adjoining shell edges, and a list of two adjoining triangles.  One of    */
/*  the two adjoining triangles may not be present (though there should      */
/*  always be one), and neighboring shell edges might not be present.        */
/*  Shell edges also store a user-defined integer "boundary marker".         */
/*  Typically, this integer is used to indicate what sort of boundary        */
/*  conditions are to be applied at that location in a finite element        */
/*  simulation.                                                              */
/*                                                                           */
/*  Like triangles, shell edges maintain information about the relative      */
/*  orientation of neighboring objects.                                      */
/*                                                                           */
/*  Points are relatively simple.  A point is a list of floating point       */
/*  numbers, starting with the x, and y coordinates, followed by an          */
/*  arbitrary number of optional user-defined floating-point attributes,     */
/*  followed by an integer boundary marker.  During the segment insertion    */
/*  phase, there is also a pointer from each point to a triangle that may    */
/*  contain it.  Each pointer is not always correct, but when one is, it     */
/*  speeds up segment insertion.  These pointers are ft_assigned values once    */
/*  at the beginning of the segment insertion phase, and are not used or     */
/*  updated at any other time.  Edge swapping during segment insertion will  */
/*  render some of them incorrect.  Hence, don't rely upon them for          */
/*  anything.  For the most part, points do not have any information about   */
/*  what triangles or shell edges they are linked to.                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Handles                                                                  */
/*                                                                           */
/*  The oriented triangle (`triedge') and oriented shell edge (`edge') data  */
/*  structures defined below do not themselves store any part of the mesh.   */
/*  The mesh itself is made of `triangle's, `shelle's, and `point's.         */
/*                                                                           */
/*  Oriented triangles and oriented shell edges will usually be referred to  */
/*  as "handles".  A handle is essentially a pointer into the mesh; it       */
/*  allows you to "hold" one particular part of the mesh.  Handles are used  */
/*  to specify the regions in which one is traversing and modifying the mesh.*/
/*  A single `triangle' may be held by many handles, or none at all.  (The   */
/*  latter case is not a memory leak, because the triangle is still          */
/*  connected to other triangles in the mesh.)                               */
/*                                                                           */
/*  A `triedge' is a handle that holds a triangle.  It holds a specific side */
/*  of the triangle.  An `edge' is a handle that holds a shell edge.  It     */
/*  holds either the left or right side of the edge.                         */
/*                                                                           */
/*  Navigation about the mesh is accomplished through a set of mesh          */
/*  manipulation primitives, further below.  Many of these primitives take   */
/*  a handle and produce a new handle that holds the mesh near the first     */
/*  handle.  Other primitives take two handles and glue the corresponding    */
/*  parts of the mesh together.  The exact position of the handles is        */
/*  important.  For instance, when two triangles are glued together by the   */
/*  bond() primitive, they are glued by the sides on which the handles lie.  */
/*                                                                           */
/*  Because points have no information about which triangles they are        */
/*  attached to, I commonly represent a point by use of a handle whose       */
/*  origin is the point.  A single handle can simultaneously represent a     */
/*  triangle, an edge, and a point.                                          */
/*                                                                           */
/*****************************************************************************/

/* The triangle data structure.  Each triangle contains three pointers to    */
/*   adjoining triangles, plus three pointers to vertex points, plus three   */
/*   pointers to shell edges (defined below; these pointers are usually      */
/*   `dummysh').  It may or may not also contain user-defined attributes     */
/*   and/or a floating-point "area constraint".  It may also contain extra   */
/*   pointers for nodes, when the user asks for high-order elements.         */
/*   Because the size and structure of a `triangle' is not decided until     */
/*   runtime, I haven't simply defined the type `triangle' to be a struct.   */

typedef double **triangle;            /* Really:  typedef triangle *triangle   */

/* An oriented triangle:  includes a pointer to a triangle and orientation.  */
/*   The orientation denotes an edge of the triangle.  Hence, there are      */
/*   three possible orientations.  By convention, each edge is always        */
/*   directed to point counterclockwise about the corresponding triangle.    */

struct triedge {
    triangle *tri;
    int orient;                                       /* Ranges from 0 to 2. */
};
typedef struct triedge triedge;

/* The shell data structure.  Each shell edge contains two pointers to       */
/*   adjoining shell edges, plus two pointers to vertex points, plus two     */
/*   pointers to adjoining triangles, plus one shell marker.                 */

typedef double **shelle;                  /* Really:  typedef shelle *shelle   */

/* An oriented shell edge:  includes a pointer to a shell edge and an        */
/*   orientation.  The orientation denotes a side of the edge.  Hence, there */
/*   are two possible orientations.  By convention, the edge is always       */
/*   directed so that the "side" denoted is the right side of the edge.      */

struct edge {
    shelle *sh;
    int shorient;                                       /* Ranges from 0 to 1. */
};
typedef struct edge edge;

/* The point data structure.  Each point is actually an array of floats.     */
/*   The number of floats is unknown until runtime.  An integer boundary     */
/*   marker, and sometimes a pointer to a triangle, is appended after the    */
/*   floats.                                                                 */

typedef double *point;

/* A type used to allocate memory.  firstblock is the first block of items.  */
/*   nowblock is the block from which items are currently being allocated.   */
/*   nextitem points to the next slab of free memory for an item.            */
/*   deaditemstack is the head of a linked list (stack) of deallocated items */
/*   that can be recycled.  unallocateditems is the number of items that     */
/*   remain to be allocated from nowblock.                                   */
/*                                                                           */
/* Traversal is the process of walking through the entire list of items, and */
/*   is separate from allocation.  Note that a traversal will visit items on */
/*   the "deaditemstack" stack as well as live items.  pathblock points to   */
/*   the block currently being traversed.  pathitem points to the next item  */
/*   to be traversed.  pathitemsleft is the number of items that remain to   */
/*   be traversed in pathblock.                                              */
/*                                                                           */
/* itemwordtype is set to POINTER or FLOATINGPOINT, and is used to suggest   */
/*   what sort of word the record is primarily made up of.  alignbytes       */
/*   determines how new records should be aligned in memory.  itembytes and  */
/*   itemwords are the length of a record in bytes (after rounding up) and   */
/*   words.  itemsperblock is the number of items allocated at once in a     */
/*   single block.  items is the number of currently allocated items.        */
/*   maxitems is the maximum number of items that have been allocated at     */
/*   once; it is the current number of items plus the number of records kept */
/*   on deaditemstack.                                                       */

struct PoolBlock {
    struct PoolBlock *next, *prev;
    size_t            block_size;
    ALIGN             alignment;
};
typedef struct PoolBlock PoolBlock;

struct memorypool {
    PoolBlock *firstblock, *nowblock, *pathblock;
    void      *nextitem;
    void      *deaditemstack;
    void      *pathitem;
    size_t    alignbytes;
    size_t    itembytes, itemwords;
    size_t    wordsize;
    int       itemsperblock;
    int       items, maxitems;
    int       unallocateditems;
    int       pathitemsleft;
};
typedef struct memorypool memorypool;

/* Local Prototypes.                                               */
LOCAL	double	counterclockwise(point,point,point);
LOCAL	double	counterclockwiseadapt(point,point,point,double);
LOCAL	double	estimate(int,double*);
LOCAL	double	incircle(point,point,point,point);
LOCAL	double	incircleadapt(point,point,point,point,double);
LOCAL	circumcenterresult findcircumcenter(point,point,point,point,
					    double*,double*);
LOCAL	finddirectionresult finddirection(triedge*,point);
LOCAL	insertsiteresult	insertsite(point,triedge*,edge*,int,int);
LOCAL	locateresult	locate(point,triedge*);
LOCAL	locateresult	preciselocate(point,triedge*);
LOCAL	int	fast_expansion_sum_zeroelim(int,double*,int,double*,double*);
LOCAL	int	scale_expansion_zeroelim(int,double*,double,double*);
LOCAL	int	scoutsegment(triedge*,point,int);
LOCAL	long	delaunay(void);
LOCAL	long	divconqdelaunay(void);
LOCAL	long	removeghosts(triedge*);
LOCAL	point	getpoint(int);
LOCAL	point	pointtraverse(void);
LOCAL	shelle	*shelletraverse(void);
LOCAL  size_t  PoolBlockSize(memorypool*p);
LOCAL	size_t	formskeleton(int*,int*,size_t);
LOCAL	size_t	randomnation(size_t);
LOCAL	triangle	*triangletraverse(void);
LOCAL  void*   FirstItemInBlock(PoolBlock*);
LOCAL	void*	NextItem(void*,memorypool*);
LOCAL  void*   allocate_storage(void*,size_t*,size_t);
LOCAL	void	*poolalloc(memorypool*);
LOCAL	void	*traverse(memorypool*);
LOCAL	void	alternateaxes(point*,int,int);
LOCAL	void	carveholes(double*,int,double*,int);
LOCAL	void	constrainededge(triedge*,point,int);
LOCAL	void	delaunayfixup(triedge*,int);
LOCAL	void	divconqrecurse(point*,int,int,triedge*,triedge*);
LOCAL	void	dummyinit(size_t trianglewords,size_t);
LOCAL	void	exactinit(void);
LOCAL	void	flip(triedge*);
LOCAL	void	highorder(void);
LOCAL	void	infecthull(void);
LOCAL	void	initializepointpool(void);
LOCAL	void	initializetrisegpools(void);
LOCAL	void	insertsegment(point,point,int);
LOCAL	void	insertshelle(triedge*,int);
LOCAL	void	internalerror(void);
LOCAL	void	makepointmap(void);
LOCAL	void	makeshelle(edge*);
LOCAL	void	maketriangle(triedge*);
LOCAL	void	markhull(void);
LOCAL	void	mergehulls(triedge*,triedge*,triedge*,triedge*,int);
LOCAL	void	numbernodes(void);
LOCAL	void	parsecommandline(TriangulateOpts*);
LOCAL	void	plague(void);
LOCAL	void	pointdealloc(point);
LOCAL	void	pointmedian(point*,int,int,int);
LOCAL	void	pointsort(point*,size_t);
LOCAL	void	pooldealloc(memorypool*,void*);
LOCAL	void	pooldeinit(memorypool*);
LOCAL	void	poolinit(memorypool*,size_t,int,size_t);
LOCAL	void	poolrestart(memorypool*);
LOCAL	void	regionplague(double,double);
LOCAL	void	segmentintersection(triedge*,edge*,point);
LOCAL	void	shelledealloc(shelle*);
LOCAL	void	transfernodes(double*,double*,int*,size_t,int);
LOCAL	void	traversalinit(memorypool*);
LOCAL	void	triangledealloc(triangle*);
LOCAL	void	triangledeinit(void);
LOCAL	void	triangleinit(void);
LOCAL	void	triangulatepolygon(triedge*,triedge*,int,int,int);
LOCAL	void	writeedges(triangulateio*);
LOCAL	void	writeelements(triangulateio*);
LOCAL	void	writeneighbors(triangulateio*);
LOCAL	void	writenodes(triangulateio*);
LOCAL	void	writepoly(triangulateio*);
LOCAL	void	writevoronoi(triangulateio*);

/* Variables used to allocate memory for triangles, shell edges, points,     */
/*   viri (triangles being eaten), bad (encroached) segments, bad (skinny    */
/*   or too large) triangles, and splay tree nodes.                          */

LOCAL memorypool triangles;
LOCAL memorypool shelles;
LOCAL memorypool points;
LOCAL memorypool viri;
LOCAL memorypool badsegments;

/* Variables that maintain the bad triangle queues.  The tails are pointers  */
/*   to the pointers that have to be filled in to enqueue an item.           */

LOCAL double xmin, xmax, ymin, ymax;                    /* x and y bounds. */
LOCAL size_t inpoints;                         /* Number of input points. */
LOCAL int holes;                                /* Number of input holes. */
LOCAL int regions;                            /* Number of input regions. */
LOCAL long edges;                              /* Number of output edges. */
LOCAL int mesh_dim;                         /* Dimension (ought to be 2). */
LOCAL int nextras;                     /* Number of attributes per point. */
LOCAL int eextras;                  /* Number of attributes per triangle. */
LOCAL long hullsize;                   /* Number of edges of convex hull. */
LOCAL size_t triwords;                       /* Total words per triangle. */
LOCAL size_t shwords;                      /* Total words per shell edge. */
LOCAL size_t pointmarkindex; /* Index to find boundary marker of a point. */
LOCAL size_t point2triindex;/* Index to find a triangle adjacent to a point. */
LOCAL int highorderindex;/* Index to find extra nodes for high-order elements. */
LOCAL size_t elemattribindex;    /* Index to find attributes of a triangle. */
LOCAL size_t areaboundindex;     /* Index to find area bound of a triangle. */
LOCAL int checksegments;     /* Are there segments in the triangulation yet? */
LOCAL int readnodefile;                       /* Has a .node file been read? */
LOCAL long samples;          /* Number of random samples for point location. */
LOCAL size_t randomseed;               /* Current random number seed. */

LOCAL double splitter;/* Used to split double factors for exact multiplication.*/
LOCAL double epsilon;                      /* Floating-point machine epsilon. */
LOCAL double resulterrbound;
LOCAL double ccwerrboundA, ccwerrboundB, ccwerrboundC;
LOCAL double iccerrboundA, iccerrboundB, iccerrboundC;

LOCAL long incirclecount;             /* Number of incircle tests performed. */
LOCAL long counterclockcount; /* Number of counterclockwise tests performed. */
LOCAL long circumcentercount;/* Number of circumcenter calculations performed. */

/* Switches for the triangulator.                                            */
/*   poly: -p switch.  refine: -r switch.                                    */
/*   quality: -q switch.                                                     */
/*     minangle: minimum angle bound, specified after -q switch.             */
/*     goodangle: cosine squared of minangle.                                */
/*   vararea: -a switch without number.                                      */
/*   fixedarea: -a switch with number.                                       */
/*     maxarea: maximum area bound, specified after -a switch.               */
/*   regionattrib: -A switch.  convex: -c switch.                            */
/*   firstnumber: inverse of -z switch.  All items are numbered starting     */
/*     from firstnumber.                                                     */
/*   edgesout: -e switch.  voronoi: -v switch.                               */
/*   neighbors: -n switch.  geomview: -g switch.                             */
/*   nobound: -B switch.  nopolywritten: -P switch.                          */
/*   nonodewritten: -N switch.  noelewritten: -E switch.                     */
/*   noiterationnum: -I switch.  noholes: -O switch.                         */
/*   noexact: -X switch.                                                     */
/*   order: element order, specified after -o switch.                        */
/*   nobisect: count of how often -Y switch is selected.                     */
/*   steiner: maximum number of Steiner points, specified after -S switch.   */
/*     steinerleft: number of Steiner points not yet used.                   */
/*   incremental: -i switch.  sweepline: -F switch.                          */
/*   dwyer: inverse of -l switch.                                            */
/*   splitseg: -s switch.                                                    */
/*   docheck: -C switch.                                                     */
/*   quiet: -Q switch.  verbose: count of how often -V switch is selected.   */
/*   useshelles: -p, -r, -q, or -c switch; determines whether shell edges    */
/*     are used at all.                                                      */
/*                                                                           */
/* Read the instructions to find out the meaning of these switches.          */

LOCAL TriangulateOpts TriOpts;
LOCAL boolean dwyer;
LOCAL int steinerleft;
LOCAL int useshelles;
LOCAL int order;
LOCAL double goodangle;

/* Variables for file names.                                                 */


/* Triangular bounding box points.                                           */

LOCAL point infpoint1, infpoint2, infpoint3;

/* Pointer to the `triangle' that occupies all of "outer space".             */

LOCAL triangle *dummytri = NULL;
LOCAL triangle *dummytribase;/* Keep base address so we can free() it later. */
LOCAL size_t   size_dummytribase;

/* Pointer to the omnipresent shell edge.  Referenced by any triangle or     */
/*   shell edge that isn't really connected to a shell edge at that          */
/*   location.                                                               */

LOCAL shelle *dummysh = NULL;
LOCAL shelle *dummyshbase;   /* Keep base address so we can free() it later. */
LOCAL size_t size_dummyshbase;

/* Pointer to a recently visited triangle.  Improves point location if       */
/*   proximate points are inserted sequentially.                             */

LOCAL triedge recenttri;

/*****************************************************************************/
/*                                                                           */
/*  Mesh manipulation primitives.  Each triangle contains three pointers to  */
/*  other triangles, with orientations.  Each pointer points not to the      */
/*  first byte of a triangle, but to one of the first three bytes of a       */
/*  triangle.  It is necessary to extract both the triangle itself and the   */
/*  orientation.  To save memory, I keep both pieces of information in one   */
/*  pointer.  To make this possible, I assume that all triangles are aligned */
/*  to four-byte boundaries.  The `decode' routine below decodes a pointer,  */
/*  extracting an orientation (in the range 0 to 2) and a pointer to the     */
/*  beginning of a triangle.  The `encode' routine compresses a pointer to a */
/*  triangle and an orientation into a single pointer.  My assumptions that  */
/*  triangles are four-byte-aligned and that the `size_t' type is     */
/*  long enough to hold a pointer are two of the few kludges in this program.*/
/*                                                                           */
/*  Shell edges are manipulated similarly.  A pointer to a shell edge        */
/*  carries both an address and an orientation in the range 0 to 1.          */
/*                                                                           */
/*  The other primitives take an oriented triangle or oriented shell edge,   */
/*  and return an oriented triangle or oriented shell edge or point; or they */
/*  change the connections in the data structure.                            */
/*                                                                           */
/*****************************************************************************/

/********* Mesh manipulation primitives begin here                   *********/
/**                                                                         **/
/**                                                                         **/

/* Fast lookup arrays to speed some of the mesh manipulation primitives.     */

LOCAL int plus1mod3[3] = {1, 2, 0};
LOCAL int minus1mod3[3] = {2, 0, 1};

/********* Primitives for triangles                                  *********/
/*                                                                           */
/*                                                                           */

/* decode() converts a pointer to an oriented triangle.  The orientation is  */
/*   extracted from the two least significant bits of the pointer.           */

#define decode(ptr, triedge)                                                  \
  (triedge).orient = (int) ((size_t) (ptr) & (size_t) 3l);      \
  (triedge).tri = (triangle *)                                                \
                  ((size_t) (ptr) ^ (size_t) (triedge).orient)

/* encode() compresses an oriented triangle into a single pointer.  It       */
/*   relies on the assumption that all triangles are aligned to four-byte    */
/*   boundaries, so the two least significant bits of (triedge).tri are zero.*/

#define encode(triedge)                                                       \
  (triangle) ((size_t) (triedge).tri | (size_t) (triedge).orient)

/* The following edge manipulation primitives are all described by Guibas    */
/*   and Stolfi.  However, they use an edge-based data structure, whereas I  */
/*   am using a triangle-based data structure.                               */

/* sym() finds the abutting triangle, on the same edge.  Note that the       */
/*   edge direction is necessarily reversed, because triangle/edge handles   */
/*   are always directed counterclockwise around the triangle.               */

#define sym(triedge1, triedge2)                                               \
  ptr = (triedge1).tri[(triedge1).orient];                                    \
  decode(ptr, triedge2);

#define symself(triedge)                                                      \
  ptr = (triedge).tri[(triedge).orient];                                      \
  decode(ptr, triedge);

/* lnext() finds the next edge (counterclockwise) of a triangle.             */

#define lnext(triedge1, triedge2)                                             \
  (triedge2).tri = (triedge1).tri;                                            \
  (triedge2).orient = plus1mod3[(triedge1).orient]

#define lnextself(triedge)                                                    \
  (triedge).orient = plus1mod3[(triedge).orient]

/* lprev() finds the previous edge (clockwise) of a triangle.                */

#define lprev(triedge1, triedge2)                                             \
  (triedge2).tri = (triedge1).tri;                                            \
  (triedge2).orient = minus1mod3[(triedge1).orient]

#define lprevself(triedge)                                                    \
  (triedge).orient = minus1mod3[(triedge).orient]

/* onext() spins counterclockwise around a point; that is, it finds the next */
/*   edge with the same origin in the counterclockwise direction.  This edge */
/*   will be part of a different triangle.                                   */

#define onext(triedge1, triedge2)                                             \
  lprev(triedge1, triedge2);                                                  \
  symself(triedge2);

#define onextself(triedge)                                                    \
  lprevself(triedge);                                                         \
  symself(triedge);

/* oprev() spins clockwise around a point; that is, it finds the next edge   */
/*   with the same origin in the clockwise direction.  This edge will be     */
/*   part of a different triangle.                                           */

#define oprev(triedge1, triedge2)                                             \
  sym(triedge1, triedge2);                                                    \
  lnextself(triedge2);

#define oprevself(triedge)                                                    \
  symself(triedge);                                                           \
  lnextself(triedge);

/* dnext() spins counterclockwise around a point; that is, it finds the next */
/*   edge with the same destination in the counterclockwise direction.  This */
/*   edge will be part of a different triangle.                              */

#define dnext(triedge1, triedge2)                                             \
  sym(triedge1, triedge2);                                                    \
  lprevself(triedge2);

#define dnextself(triedge)                                                    \
  symself(triedge);                                                           \
  lprevself(triedge);

/* dprev() spins clockwise around a point; that is, it finds the next edge   */
/*   with the same destination in the clockwise direction.  This edge will   */
/*   be part of a different triangle.                                        */

#define dprev(triedge1, triedge2)                                             \
  lnext(triedge1, triedge2);                                                  \
  symself(triedge2);

#define dprevself(triedge)                                                    \
  lnextself(triedge);                                                         \
  symself(triedge);

/* rnext() moves one edge counterclockwise about the adjacent triangle.      */
/*   (It's best understood by reading Guibas and Stolfi.  It involves        */
/*   changing triangles twice.)                                              */

#define rnext(triedge1, triedge2)                                             \
  sym(triedge1, triedge2);                                                    \
  lnextself(triedge2);                                                        \
  symself(triedge2);

#define rnextself(triedge)                                                    \
  symself(triedge);                                                           \
  lnextself(triedge);                                                         \
  symself(triedge);

/* rnext() moves one edge clockwise about the adjacent triangle.             */
/*   (It's best understood by reading Guibas and Stolfi.  It involves        */
/*   changing triangles twice.)                                              */

#define rprev(triedge1, triedge2)                                             \
  sym(triedge1, triedge2);                                                    \
  lprevself(triedge2);                                                        \
  symself(triedge2);

#define rprevself(triedge)                                                    \
  symself(triedge);                                                           \
  lprevself(triedge);                                                         \
  symself(triedge);

/* These primitives determine or set the origin, destination, or apex of a   */
/* triangle.                                                                 */

#define org(triedge, pointptr)                                                \
  pointptr = (point) (triedge).tri[plus1mod3[(triedge).orient] + 3]

#define dest(triedge, pointptr)                                               \
  pointptr = (point) (triedge).tri[minus1mod3[(triedge).orient] + 3]

#define apex(triedge, pointptr)                                               \
  pointptr = (point) (triedge).tri[(triedge).orient + 3]

#define setorg(triedge, pointptr)                                             \
  (triedge).tri[plus1mod3[(triedge).orient] + 3] = (triangle) pointptr

#define setdest(triedge, pointptr)                                            \
  (triedge).tri[minus1mod3[(triedge).orient] + 3] = (triangle) pointptr

#define setapex(triedge, pointptr)                                            \
  (triedge).tri[(triedge).orient + 3] = (triangle) pointptr

#define setvertices2null(triedge)                                             \
  (triedge).tri[3] = NULL;                                         \
  (triedge).tri[4] = NULL;                                         \
  (triedge).tri[5] = NULL;

/* Bond two triangles together.                                              */

#define bond(triedge1, triedge2)                                              \
  (triedge1).tri[(triedge1).orient] = encode(triedge2);                       \
  (triedge2).tri[(triedge2).orient] = encode(triedge1)

/* Dissolve a bond (from one side).  Note that the other triangle will still */
/*   think it's connected to this triangle.  Usually, however, the other     */
/*   triangle is being deleted entirely, or bonded to another triangle, so   */
/*   it doesn't matter.                                                      */

#define dissolve(triedge)                                                     \
  (triedge).tri[(triedge).orient] = (triangle) dummytri

/* Copy a triangle/edge handle.                                              */

#define triedgecopy(triedge1, triedge2)                                       \
  (triedge2).tri = (triedge1).tri;                                            \
  (triedge2).orient = (triedge1).orient

/* Test for equality of triangle/edge handles.                               */

#define triedgeequal(triedge1, triedge2)                                      \
  (((triedge1).tri == (triedge2).tri) &&                                      \
   ((triedge1).orient == (triedge2).orient))

/* Primitives to infect or cure a triangle with the virus.  These rely on    */
/*   the assumption that all shell edges are aligned to four-byte boundaries.*/

#define infect(triedge)                                                       \
  (triedge).tri[6] = (triangle)                                               \
                     ((size_t) (triedge).tri[6] | (size_t) 2l)

#define uninfect(triedge)                                                     \
  (triedge).tri[6] = (triangle)                                               \
                     ((size_t) (triedge).tri[6] & ~ (size_t) 2l)

/* Test a triangle for viral infection.                                      */

#define infected(triedge)                                                     \
  (((size_t) (triedge).tri[6] & (size_t) 2l) != 0)

/* Check or set a triangle's attributes.                                     */

#define elemattribute(triedge, attnum)                                        \
  ((double *) (triedge).tri)[elemattribindex + (attnum)]

#define setelemattribute(triedge, attnum, value)                              \
  ((double *) (triedge).tri)[elemattribindex + (attnum)] = value

/* Check or set a triangle's maximum area bound.                             */

#define areabound(triedge)  ((double *) (triedge).tri)[areaboundindex]

#define setareabound(triedge, value)                                          \
  ((double *) (triedge).tri)[areaboundindex] = value

/********* Primitives for shell edges                                *********/
/*                                                                           */
/*                                                                           */

/* sdecode() converts a pointer to an oriented shell edge.  The orientation  */
/*   is extracted from the least significant bit of the pointer.  The two    */
/*   least significant bits (one for orientation, one for viral infection)   */
/*   are masked out to produce the real pointer.                             */

#define sdecode(sptr, edge)                                                   \
  (edge).shorient = (int) ((size_t) (sptr) & (size_t) 1l);      \
  (edge).sh = (shelle *)                                                      \
              ((size_t) (sptr) & ~ (size_t) 3l)

/* sencode() compresses an oriented shell edge into a single pointer.  It    */
/*   relies on the assumption that all shell edges are aligned to two-byte   */
/*   boundaries, so the least significant bit of (edge).sh is zero.          */

#define sencode(edge)                                                         \
  (shelle) ((size_t) (edge).sh | (size_t) (edge).shorient)

/* ssym() toggles the orientation of a shell edge.                           */

#define ssym(edge1, edge2)                                                    \
  (edge2).sh = (edge1).sh;                                                    \
  (edge2).shorient = 1 - (edge1).shorient

#define ssymself(edge)                                                        \
  (edge).shorient = 1 - (edge).shorient

/* spivot() finds the other shell edge (from the same segment) that shares   */
/*   the same origin.                                                        */

#define spivot(edge1, edge2)                                                  \
  sptr = (edge1).sh[(edge1).shorient];                                        \
  sdecode(sptr, edge2)

#define spivotself(edge)                                                      \
  sptr = (edge).sh[(edge).shorient];                                          \
  sdecode(sptr, edge)

/* snext() finds the next shell edge (from the same segment) in sequence;    */
/*   one whose origin is the input shell edge's destination.                 */

#define snext(edge1, edge2)                                                   \
  sptr = (edge1).sh[1 - (edge1).shorient];                                    \
  sdecode(sptr, edge2)

#define snextself(edge)                                                       \
  sptr = (edge).sh[1 - (edge).shorient];                                      \
  sdecode(sptr, edge)

/* These primitives determine or set the origin or destination of a shell    */
/*   edge.                                                                   */

#define sorg(edge, pointptr)                                                  \
  pointptr = (point) (edge).sh[2 + (edge).shorient]

#define sdest(edge, pointptr)                                                 \
  pointptr = (point) (edge).sh[3 - (edge).shorient]

#define setsorg(edge, pointptr)                                               \
  (edge).sh[2 + (edge).shorient] = (shelle) pointptr

#define setsdest(edge, pointptr)                                              \
  (edge).sh[3 - (edge).shorient] = (shelle) pointptr

/* These primitives read or set a shell marker.  Shell markers are used to   */
/*   hold user boundary information.                                         */

#define mark(edge)  (* (int *) ((edge).sh + 6))

#define setmark(edge, value)                                                  \
  * (int *) ((edge).sh + 6) = value

/* Bond two shell edges together.                                            */

#define sbond(edge1, edge2)                                                   \
  (edge1).sh[(edge1).shorient] = sencode(edge2);                              \
  (edge2).sh[(edge2).shorient] = sencode(edge1)

/* Dissolve a shell edge bond (from one side).  Note that the other shell    */
/*   edge will still think it's connected to this shell edge.                */

#define sdissolve(edge)                                                       \
  (edge).sh[(edge).shorient] = (shelle) dummysh

/* Copy a shell edge.                                                        */

#define shellecopy(edge1, edge2)                                              \
  (edge2).sh = (edge1).sh;                                                    \
  (edge2).shorient = (edge1).shorient

/* Test for equality of shell edges.                                         */

#define shelleequal(edge1, edge2)                                             \
  (((edge1).sh == (edge2).sh) &&                                              \
   ((edge1).shorient == (edge2).shorient))

/********* Primitives for interacting triangles and shell edges      *********/
/*                                                                           */
/*                                                                           */

/* tspivot() finds a shell edge abutting a triangle.                         */

#define tspivot(triedge, edge)                                                \
  sptr = (shelle) (triedge).tri[6 + (triedge).orient];                        \
  sdecode(sptr, edge)

/* stpivot() finds a triangle abutting a shell edge.  It requires that the   */
/*   variable `ptr' of type `triangle' be defined.                           */

#define stpivot(edge, triedge)                                                \
  ptr = (triangle) (edge).sh[4 + (edge).shorient];                            \
  decode(ptr, triedge)

/* Bond a triangle to a shell edge.                                          */

#define tsbond(triedge, edge)                                                 \
  (triedge).tri[6 + (triedge).orient] = (triangle) sencode(edge);             \
  (edge).sh[4 + (edge).shorient] = (shelle) encode(triedge)

/* Dissolve a bond (from the triangle side).                                 */

#define tsdissolve(triedge)                                                   \
  (triedge).tri[6 + (triedge).orient] = (triangle) dummysh

/* Dissolve a bond (from the shell edge side).                               */

#define stdissolve(edge)                                                      \
  (edge).sh[4 + (edge).shorient] = (shelle) dummytri

/********* Primitives for points                                     *********/
/*                                                                           */
/*                                                                           */

#define pointmark(pt)  ((int *) (pt))[pointmarkindex]

#define setpointmark(pt, value)                                               \
  ((int *) (pt))[pointmarkindex] = value

#define point2tri(pt)  ((triangle *) (pt))[point2triindex]

#define setpoint2tri(pt, value)                                               \
  ((triangle *) (pt))[point2triindex] = value

/*****************************************************************************/
/*                                                                           */
/*  internalerror()   Ask the user to send me the defective product.  Exit.  */
/*                                                                           */
/*****************************************************************************/

LOCAL void internalerror(void)
{
    (void) printf("ERROR - internal error in triangle construction\n"
                  "  Please report this bug to jrs@cs.cmu.edu\n"
                  "  Include the message above, your input data set, "
		  "and the exact\n"
                  "    command line you used to run Triangle.\n");
    clean_up(ERROR);
}		/*end internalerror*/

/*****************************************************************************/
/*                                                                           */
/*  parsecommandline()   Read the command line, identify switches, and set   */
/*                       up options and file names.                          */
/*                                                                           */
/*  The effects of this routine are felt entirely through global variables.  */
/*                                                                           */
/*****************************************************************************/

LOCAL void parsecommandline(
    TriangulateOpts *opts)
{
    TriOpts = *opts;
    dwyer = (TriOpts.nodwyer) ? NO : YES;
    order = TriOpts.order;

    steinerleft = TriOpts.steiner;
    useshelles = (TriOpts.poly || TriOpts.refine || TriOpts.convex) ? YES : NO;
    goodangle = cos(TriOpts.minangle * PI / 180.0);
    goodangle *= goodangle;
    if (TriOpts.refine && TriOpts.noiterationnum)
    {
	screen("ERROR in parsecommandline(),  "
	       "You cannot use noiterationnum when "
	       "refining a triangulation.\n");
	clean_up(ERROR);
    }
    /* Be careful not to allocate space for element area constraints that */
    /*   will never be ft_assigned any value (other than the default -1.0).  */
    if (!TriOpts.refine && !TriOpts.poly)
    {
	TriOpts.vararea = NO;
    }
    /* Be careful not to add an extra attribute to each element unless the */
    /*   input supports it (PSLG in, but not refining a preexisting mesh). */
    if (TriOpts.refine || !TriOpts.poly)
    {
	TriOpts.regionattrib = ZERO;
    }

}		/*end parsecommandline*/

/*****************************************************************************/
/*                                                                           */
/*  poolinit()   Initialize a pool of memory for allocation of items.        */
/*                                                                           */
/*  This routine initializes the machinery for allocating items.  A `pool'   */
/*  is created whose records have size at least `bytecount'.  Items will be  */
/*  allocated in `itemcount'-item blocks.  Each item is assumed to be a      */
/*  collection of words, and either pointers or floating-point values are    */
/*  assumed to be the "primary" word type.  (The "primary" word type is used */
/*  to determine alignment of items.)  If `alignment' isn't zero, all items  */
/*  will be `alignment'-byte aligned in memory.  `alignment' must be either  */
/*  a multiple or a factor of the primary word size; powers of two are safe. */
/*  `alignment' is normally used to create a few unused bits at the bottom   */
/*  of each item's pointer, in which information may be stored.              */
/*                                                                           */
/*  Don't change this routine unless you understand it.                      */
/*                                                                           */
/*****************************************************************************/

LOCAL void poolinit(
    memorypool *pool,
    size_t     bytecount,
    int        itemcount,
    size_t     wordsize)
{
    size_t firstblock_size;

    /* Initialize values in the pool. */
    /* Find the proper alignment, which must be at least as large as:   */
    /*   - The parameter `alignment'.                                   */
    /*   - The primary word type, to avoid unaligned accesses.          */
    /*   - sizeof(void *), so the stack of dead items can be maintained */
    /*       without unaligned accesses.                                */
    pool->wordsize = wordsize;
    pool->alignbytes = sizeof(ALIGN);
    pool->itemwords = ((bytecount + pool->alignbytes - 1) / pool->alignbytes)
        * (pool->alignbytes / wordsize);
    pool->itembytes = pool->itemwords * wordsize;
    pool->itemsperblock = itemcount;

    /* Allocate a block of items.  Space for `itemsperblock' items and one    */
    /*   pointer (to point to the next block) are allocated, as well as space */
    /*   to ensure alignment of the items.                                    */

    firstblock_size = PoolBlockSize(pool);
    if (pool->firstblock == NULL)
    {
	scalar(&pool->firstblock,firstblock_size);
        if (pool->firstblock == NULL)
        {
	    screen("ERROR in poolinit(),  Out of memory.\n");
	    clean_up(ERROR);
        }
	pool->firstblock->block_size = firstblock_size;
	pool->firstblock->prev = pool->firstblock->next = NULL;
    }
    else if (firstblock_size > pool->firstblock->block_size)
    {
	PoolBlock *next, *prev;
	next = pool->firstblock->next;
	prev = pool->firstblock->prev;
	free(pool->firstblock);
	scalar(&pool->firstblock,firstblock_size);
        if (pool->firstblock == NULL)
        {
	    screen("ERROR in poolinit(),  Out of memory.\n");
	    clean_up(ERROR);
        }
	pool->firstblock->block_size = firstblock_size;
	pool->firstblock->prev = prev;
	pool->firstblock->next = next;
	if (prev != NULL)
	    prev->next = pool->firstblock;
	if (next != NULL)
	    next->prev = pool->firstblock;
    }
    poolrestart(pool);
}		/*end poolinit*/

LOCAL size_t PoolBlockSize(
    memorypool *p)
{
    return sizeof(PoolBlock) + p->alignbytes + p->itemsperblock*p->itembytes;
}		/*end PoolBlockSize*/

/*****************************************************************************/
/*                                                                           */
/*  poolrestart()   Deallocate all items in a pool.                          */
/*                                                                           */
/*  The pool is returned to its starting state, except that no memory is     */
/*  freed to the operating system.  Rather, the previously allocated blocks  */
/*  are ready to be reused.                                                  */
/*                                                                           */
/*****************************************************************************/

LOCAL void poolrestart(
    memorypool *pool)
{
    pool->items = 0;
    pool->maxitems = 0;

    /* Set the currently active block. */
    pool->nowblock = pool->firstblock;
    /* Find the first item in the pool.  Increment by the size of (void *). */
    pool->nextitem = FirstItemInBlock(pool->nowblock);
    /* There are lots of unallocated items left in this block. */
    pool->unallocateditems = pool->itemsperblock;
    /* The stack of deallocated items is empty. */
    pool->deaditemstack = NULL;
}		/*end poolrestart*/

LOCAL void *FirstItemInBlock(
    PoolBlock *pb)
{
    return (void *) &pb->alignment;
}		/*end FirstItemInBlock*/

/*****************************************************************************/
/*                                                                           */
/*  pooldeinit()   Free to the operating system all memory taken by a pool.  */
/*                                                                           */
/*****************************************************************************/

LOCAL void pooldeinit(
    memorypool *pool)
{
    poolrestart(pool);
}		/*end pooldeinit*/

/*****************************************************************************/
/*                                                                           */
/*  poolalloc()   Allocate space for an item.                                */
/*                                                                           */
/*****************************************************************************/

LOCAL void *poolalloc(
	memorypool *pool)
{
    void      *newitem;
    PoolBlock *newblock;

    /* First check the linked list of dead items.  If the list is not   */
    /*   empty, allocate an item from the list rather than a fresh one. */
    if (pool->deaditemstack != NULL)
    {
	newitem = pool->deaditemstack;           /* Take first item in list. */
	pool->deaditemstack = * (void **) pool->deaditemstack;
    }
    else
    {
	/* Check if there are any free items left in the current block. */
	if (pool->unallocateditems == 0)
	{
	    size_t newblock_size;
            newblock_size = PoolBlockSize(pool);

	    /* Check if another block must be allocated. */
	    if (pool->nowblock->next == NULL)
	    {
		/*
		 * Allocate a new block of items, pointed to by the
		 * previous block.
		 */
	        scalar(&newblock,newblock_size);
                if (newblock == NULL)
                {
	            screen("ERROR in poolalloc(),  Out of memory.\n");
		    clean_up(ERROR);
                }
		newblock->block_size = newblock_size;
		pool->nowblock->next = newblock;
		newblock->prev = pool->nowblock;
		newblock->next = NULL;
	    }
	    else if (newblock_size > pool->nowblock->next->block_size)
	    {
		PoolBlock *next = pool->nowblock->next->next;
		free(pool->nowblock->next);
	        scalar(&newblock,newblock_size);
                if (newblock == NULL)
                {
	            screen("ERROR in poolalloc(),  Out of memory.\n");
		    clean_up(ERROR);
                }
		newblock->block_size = newblock_size;
		pool->nowblock->next = newblock;
		newblock->prev = pool->nowblock;
		newblock->next = next;
		if (next != NULL)
		    next->prev = newblock;
	    }
	    /* Move to the new block. */
	    pool->nowblock = pool->nowblock->next;
	    pool->nextitem = FirstItemInBlock(pool->nowblock);
	    /* There are lots of unallocated items left in this block. */
	    pool->unallocateditems = pool->itemsperblock;
	}
	/* Allocate a new item. */
	newitem = pool->nextitem;
	/* Advance `nextitem' pointer to next free item in block. */
	pool->nextitem = NextItem(pool->nextitem,pool);
	pool->unallocateditems--;
	pool->maxitems++;
    }
    pool->items++;
    return newitem;
}		/*end poolalloc*/

LOCAL void *NextItem(
    void       *nextitem,
    memorypool *pool)
{
    return (void*) ((byte*)nextitem + pool->itemwords*pool->wordsize);
}		/*end NextItem*/

/*****************************************************************************/
/*                                                                           */
/*  pooldealloc()   Deallocate space for an item.                            */
/*                                                                           */
/*  The deallocated space is stored in a queue for later reuse.              */
/*                                                                           */
/*****************************************************************************/

LOCAL void pooldealloc(
    memorypool *pool,
    void       *dyingitem)
{
    /* Push freshly killed item onto stack. */
    *((void **) dyingitem) = pool->deaditemstack;
    pool->deaditemstack = dyingitem;
    pool->items--;
}		/*end pooldealloc*/

/*****************************************************************************/
/*                                                                           */
/*  traversalinit()   Prepare to traverse the entire list of items.          */
/*                                                                           */
/*  This routine is used in conjunction with traverse().                     */
/*                                                                           */
/*****************************************************************************/

LOCAL void traversalinit(
    memorypool *pool)
{
    /* Begin the traversal in the first block. */
    pool->pathblock = pool->firstblock;
    /* Find the first item in the block.  Increment by the size of (void *). */
    pool->pathitem = FirstItemInBlock(pool->pathblock);
    /* Set the number of items left in the current block. */
    pool->pathitemsleft = pool->itemsperblock;
}		/*end traversalinit*/

/*****************************************************************************/
/*                                                                           */
/*  traverse()   Find the next item in the list.                             */
/*                                                                           */
/*  This routine is used in conjunction with traversalinit().  Be forewarned */
/*  that this routine successively returns all items in the list, including  */
/*  deallocated ones on the deaditemqueue.  It's up to you to figure out     */
/*  which ones are actually dead.  Why?  I don't want to allocate extra      */
/*  space just to demarcate dead items.  It can usually be done more         */
/*  space-efficiently by a routine that knows something about the structure  */
/*  of the item.                                                             */
/*                                                                           */
/*****************************************************************************/

LOCAL void *traverse(
    memorypool *pool)
{
    void *newitem;

    /* Stop upon exhausting the list of items. */
    if (pool->pathitem == pool->nextitem)
    {
	return NULL;
    }
    /* Check whether any untraversed items remain in the current block. */
    if (pool->pathitemsleft == 0)
    {
	pool->pathblock = pool->pathblock->next;
	pool->pathitem = (void *) FirstItemInBlock(pool->pathblock);
	/* Set the number of items left in the current block. */
	pool->pathitemsleft = pool->itemsperblock;
    }
    newitem = pool->pathitem;
    pool->pathitem = NextItem(pool->pathitem,pool);
    pool->pathitemsleft--;
    return newitem;
}		/*end traverse*/

/*****************************************************************************/
/*                                                                           */
/*  dummyinit()   Initialize the triangle that fills "outer space" and the   */
/*                omnipresent shell edge.                                    */
/*                                                                           */
/*  The triangle that fills "outer space", called `dummytri', is pointed to  */
/*  by every triangle and shell edge on a boundary (be it outer or inner) of */
/*  the triangulation.  Also, `dummytri' points to one of the triangles on   */
/*  the convex hull (until the holes and concavities are carved), making it  */
/*  possible to find a starting triangle for point location.                 */
/*                                                                           */
/*  The omnipresent shell edge, `dummysh', is pointed to by every triangle   */
/*  or shell edge that doesn't have a full complement of real shell edges    */
/*  to point to.                                                             */
/*                                                                           */
/*****************************************************************************/

LOCAL void dummyinit(
    size_t trianglewords,
    size_t shellewords)
{
    size_t alignptr;

    /* `triwords' and `shwords' are used by the mesh manipulation primitives */
    /*   to extract orientations of triangles and shell edges from pointers. */
    triwords = trianglewords;      /* Initialize `triwords' once and for all. */
    shwords = shellewords;         /* Initialize `shwords' once and for all. */

    /* Set up `dummytri', the `triangle' that occupies "outer space". */
    dummytribase =
	(triangle *) allocate_storage(dummytribase,&size_dummytribase,
				      triwords*sizeof(triangle) +
				      triangles.alignbytes);
    /* Align `dummytri' on a `triangles.alignbytes'-byte boundary. */
    alignptr = (size_t) dummytribase;
    dummytri = (triangle *)
        (alignptr + (size_t) triangles.alignbytes
        - (alignptr % (size_t) triangles.alignbytes));
    /* Initialize the three adjoining triangles to be "outer space".  These  */
    /*   will eventually be changed by various bonding operations, but their */
    /*   values don't really matter, as long as they can legally be          */
    /*   dereferenced.                                                       */
    dummytri[0] = (triangle) dummytri;
    dummytri[1] = (triangle) dummytri;
    dummytri[2] = (triangle) dummytri;
    /* Three NULL vertex points. */
    dummytri[3] = NULL;
    dummytri[4] = NULL;
    dummytri[5] = NULL;

    if (useshelles)
    {
	/* Set up `dummysh', the omnipresent "shell edge" pointed to by any   */
	/*   triangle side or shell edge end that isn't attached to a real    */
	/*   shell   edge.                                                    */
        dummyshbase =
	    (shelle *) allocate_storage(dummyshbase,&size_dummyshbase,
					shwords*sizeof(shelle) +
					shelles.alignbytes);

	/* Align `dummysh' on a `shelles.alignbytes'-byte boundary. */
	alignptr = (size_t) dummyshbase;
	dummysh = (shelle *)
	    (alignptr + (size_t) shelles.alignbytes
	    - (alignptr % (size_t) shelles.alignbytes));
	/* Initialize the two adjoining shell edges to be the omnipresent shell */
	/*   edge.  These will eventually be changed by various bonding         */
	/*   operations, but their values don't really matter, as long as they  */
	/*   can legally be dereferenced.                                       */
	dummysh[0] = (shelle) dummysh;
	dummysh[1] = (shelle) dummysh;
	/* Two NULL vertex points. */
	dummysh[2] = NULL;
	dummysh[3] = NULL;
	/* Initialize the two adjoining triangles to be "outer space". */
	dummysh[4] = (shelle) dummytri;
	dummysh[5] = (shelle) dummytri;
	/* Set the boundary marker to zero. */
	* (int *) (dummysh + 6) = 0;

	/* Initialize the three adjoining shell edges of `dummytri' to be */
	/*   the omnipresent shell edge.                                  */
	dummytri[6] = (triangle) dummysh;
	dummytri[7] = (triangle) dummysh;
	dummytri[8] = (triangle) dummysh;
    }
}		/*end dummyinit*/

/*****************************************************************************/
/*                                                                           */
/*  initializepointpool()   Calculate the size of the point data structure   */
/*                          and initialize its memory pool.                  */
/*                                                                           */
/*  This routine also computes the `pointmarkindex' and `point2triindex'     */
/*  indices used to find values within each point.                           */
/*                                                                           */
/*****************************************************************************/

LOCAL void initializepointpool(void)
{
    size_t pointsize;
    size_t wordsize;

    /* The index within each point at which the boundary marker is found.  */
    /*   Ensure the point marker is aligned to a INT-byte address. */
    pointmarkindex=((mesh_dim+nextras)*FLOAT+INT-1)/INT;
    pointsize = (pointmarkindex + 1) * INT;
    if (TriOpts.poly)
    {
	/* The index within each point at which a triangle pointer is found.  */
	/*  Ensure the pointer is aligned to a sizeof(triangle)-byte address. */
	point2triindex = (pointsize + sizeof(triangle) - 1) / sizeof(triangle);
	pointsize = (point2triindex + 1) * sizeof(triangle);
    }
    /* Initialize the pool of points. */
    wordsize = (FLOAT > sizeof(void*)) ? FLOAT : sizeof(void*);
    poolinit(&points,pointsize,POINTPERBLOCK,wordsize);
}		/*end initializepointpool*/

/*****************************************************************************/
/*                                                                           */
/*  initializetrisegpools()   Calculate the sizes of the triangle and shell  */
/*                            edge data structures and initialize their      */
/*                            memory pools.                                  */
/*                                                                           */
/*  This routine also computes the `highorderindex', `elemattribindex', and  */
/*  `areaboundindex' indices used to find values within each triangle.       */
/*                                                                           */
/*****************************************************************************/

LOCAL void initializetrisegpools(void)
{
    size_t trisize;

    /* The index within each triangle at which the extra nodes (above three)  */
    /*   associated with high order elements are found.  There are three      */
    /*   pointers to other triangles, three pointers to corners, and possibly */
    /*   three pointers to shell edges before the extra nodes.                */
    highorderindex = 6 + (useshelles * 3);
    /* The number of bytes occupied by a triangle. */
    trisize = ((order+1)*(order+2)/2 + (highorderindex - 3)) * sizeof(triangle);
    /* The index within each triangle at which its attributes are found, */
    /*   where the index is measured in floats.                           */
    elemattribindex = (trisize + FLOAT - 1) / FLOAT;
    /* The index within each triangle at which the maximum area constraint  */
    /*   is found, where the index is measured in floats.  Note that if the  */
    /*   `regionattrib' flag is set, an additional attribute will be added. */
    areaboundindex = elemattribindex + eextras + TriOpts.regionattrib;
    /* If triangle attributes or an area bound are needed, increase */
    /* the number of bytes occupied by a triangle.                  */
    if (TriOpts.vararea)
    {
	trisize = (areaboundindex + 1) * FLOAT;
    }
    else if (eextras + TriOpts.regionattrib > 0)
    {
	trisize = areaboundindex * FLOAT;
    }
    /* If a Voronoi diagram or triangle neighbor graph is requested, make    */
    /*   sure there's room to store an integer index in each triangle.  This */
    /*   integer index can occupy the same space as the shell edges or       */
    /*   attributes or area constraint or extra nodes.                       */
    if ((TriOpts.voronoi || TriOpts.neighbors) &&
	(trisize < 6*sizeof(triangle) + INT))
    {
	trisize = 6 * sizeof(triangle) + INT;
    }
    /* Having determined the memory size of a triangle, initialize the pool. */
    poolinit(&triangles,trisize,TRIPERBLOCK,sizeof(void*));

    if (useshelles)
    {
	/* Initialize the pool of shell edges. */
	poolinit(&shelles,6*sizeof(triangle)+INT,SHELLEPERBLOCK,
	         sizeof(void*));

	/* Initialize the "outer space" triangle and omnipresent shell edge. */
	dummyinit(triangles.itemwords, shelles.itemwords);
    }
    else
    {
	/* Initialize the "outer space" triangle. */
	dummyinit(triangles.itemwords, 0);
    }
}		/*end initializetrisegpools*/

/*****************************************************************************/
/*                                                                           */
/*  triangledealloc()   Deallocate space for a triangle, marking it dead.    */
/*                                                                           */
/*****************************************************************************/

LOCAL void triangledealloc(triangle *dyingtriangle)
{
    /* Set triangle's vertices to NULL.  This makes it possible to        */
    /*   detect dead triangles when traversing the list of all triangles. */
    dyingtriangle[3] = NULL;
    dyingtriangle[4] = NULL;
    dyingtriangle[5] = NULL;
    pooldealloc(&triangles, (void *) dyingtriangle);
}		/*end triangledealloc*/

/*****************************************************************************/
/*                                                                           */
/*  triangletraverse()   Traverse the triangles, skipping dead ones.         */
/*                                                                           */
/*****************************************************************************/

LOCAL triangle *triangletraverse(void)
{
    triangle *newtriangle;

    do
    {
	newtriangle = (triangle *) traverse(&triangles);
	if (newtriangle == NULL)
	{
	    return NULL;
	}
    } while (newtriangle[3] == NULL);          /* Skip dead ones. */
    return newtriangle;
}		/*end triangletraverse*/

/*****************************************************************************/
/*                                                                           */
/*  shelledealloc()   Deallocate space for a shell edge, marking it dead.    */
/*                                                                           */
/*****************************************************************************/

LOCAL void shelledealloc(shelle *dyingshelle)
{
    /* Set shell edge's vertices to NULL.  This makes it possible to */
    /*   detect dead shells when traversing the list of all shells.  */
    dyingshelle[2] = NULL;
    dyingshelle[3] = NULL;
    pooldealloc(&shelles, (void *) dyingshelle);
}		/*end shelledealloc*/

/*****************************************************************************/
/*                                                                           */
/*  shelletraverse()   Traverse the shell edges, skipping dead ones.         */
/*                                                                           */
/*****************************************************************************/

LOCAL shelle *shelletraverse(void)
{
    shelle *newshelle;

    do
    {
	newshelle = (shelle *) traverse(&shelles);
	if (newshelle == NULL)
	{
	    return NULL;
	}
    } while (newshelle[2] == NULL);                /* Skip dead ones. */
    return newshelle;
}		/*end shelletraverse*/

/*****************************************************************************/
/*                                                                           */
/*  pointdealloc()   Deallocate space for a point, marking it dead.          */
/*                                                                           */
/*****************************************************************************/

LOCAL void pointdealloc(point dyingpoint)
{
    /* Mark the point as dead.  This makes it possible to detect dead points */
    /*   when traversing the list of all points.                             */
    setpointmark(dyingpoint, DEADPOINT);
    pooldealloc(&points, (void *) dyingpoint);
}		/*end pointdealloc*/

/*****************************************************************************/
/*                                                                           */
/*  pointtraverse()   Traverse the points, skipping dead ones.               */
/*                                                                           */
/*****************************************************************************/

LOCAL point pointtraverse(void)
{
    point newpoint;

    do
    {
	newpoint = (point) traverse(&points);
	if (newpoint == NULL)
	{
	    return NULL;
	}
    } while (pointmark(newpoint) == DEADPOINT);           /* Skip dead ones. */
    return newpoint;
}		/*end pointtraverse*/

/*****************************************************************************/
/*                                                                           */
/*  getpoint()   Get a specific point, by number, from the list.             */
/*                                                                           */
/*  The first point is number 'firstnumber'.                                 */
/*                                                                           */
/*  Note that this takes O(n) time (with a small constant, if POINTPERBLOCK  */
/*  is large).  I don't care to take the trouble to make it work in constant */
/*  time.                                                                    */
/*                                                                           */
/*****************************************************************************/

LOCAL point getpoint(int number)
{
    PoolBlock *getblock;
    point foundpoint;
    int current;

    getblock = points.firstblock;
    current = TriOpts.firstnumber;
    /* Find the right block. */
    while (current + points.itemsperblock <= number)
    {
	getblock = getblock->next;
	current += points.itemsperblock;
    }
    /* Now find the right point. */
    foundpoint = (point) FirstItemInBlock(getblock);
    while (current < number)
    {
	foundpoint += points.itemwords;
	current++;
    }
    return foundpoint;
}		/*end getpoint*/

/*****************************************************************************/
/*                                                                           */
/*  triangledeinit()   Free all remaining allocated memory.                  */
/*                                                                           */
/*****************************************************************************/

LOCAL void triangledeinit(void)
{
    pooldeinit(&triangles);
    /*free(dummytribase);JWG*/
    if (useshelles)
    {
	pooldeinit(&shelles);
	/*free(dummyshbase);JWG*/
    }
    pooldeinit(&points);
}		/*end triangledeinit*/

/*****************************************************************************/
/*                                                                           */
/*  maketriangle()   Create a new triangle with orientation zero.            */
/*                                                                           */
/*****************************************************************************/

LOCAL void maketriangle(triedge *newtriedge)
{
    int i;

    newtriedge->tri = (triangle *) poolalloc(&triangles);
    /* Initialize the three adjoining triangles to be "outer space". */
    newtriedge->tri[0] = (triangle) dummytri;
    newtriedge->tri[1] = (triangle) dummytri;
    newtriedge->tri[2] = (triangle) dummytri;
    /* Three NULL vertex points. */
    newtriedge->tri[3] = NULL;
    newtriedge->tri[4] = NULL;
    newtriedge->tri[5] = NULL;
    /* Initialize the three adjoining shell edges to be the omnipresent */
    /*   shell edge.                                                    */
    if (useshelles)
    {
	newtriedge->tri[6] = (triangle) dummysh;
	newtriedge->tri[7] = (triangle) dummysh;
	newtriedge->tri[8] = (triangle) dummysh;
    }
    for (i = 0; i < eextras; i++)
    {
	setelemattribute(*newtriedge, i, 0.0);
    }
    if (TriOpts.vararea)
    {
	setareabound(*newtriedge, -1.0);
    }

    newtriedge->orient = 0;
}		/*end maketriangle*/

/*****************************************************************************/
/*                                                                           */
/*  makeshelle()   Create a new shell edge with orientation zero.            */
/*                                                                           */
/*****************************************************************************/

LOCAL void makeshelle(edge *newedge)
{
    newedge->sh = (shelle *) poolalloc(&shelles);
    /* Initialize the two adjoining shell edges to be the omnipresent */
    /*   shell edge.                                                  */
    newedge->sh[0] = (shelle) dummysh;
    newedge->sh[1] = (shelle) dummysh;
    /* Two NULL vertex points. */
    newedge->sh[2] = NULL;
    newedge->sh[3] = NULL;
    /* Initialize the two adjoining triangles to be "outer space". */
    newedge->sh[4] = (shelle) dummytri;
    newedge->sh[5] = (shelle) dummytri;
    /* Set the boundary marker to zero. */
    setmark(*newedge, 0);

    newedge->shorient = 0;
}		/*end makeshelle*/

/**                                                                         **/
/**                                                                         **/
/********* Constructors end here                                     *********/

/********* Determinant evaluation routines begin here                *********/
/**                                                                         **/
/**                                                                         **/

/* The adaptive exact arithmetic geometric predicates implemented herein are */
/*   described in detail in my Technical Report CMU-CS-96-140.  The complete */
/*   reference is given in the header.                                       */

/* Which of the following two methods of finding the absolute values is      */
/*   fastest is compiler-dependent.  A few compilers can inline and optimize */
/*   the fabs() call; but most will incur the overhead of a function call,   */
/*   which is disastrously slow.  A faster way on IEEE machines might be to  */
/*   mask the appropriate bit, but that's difficult to do in C.              */

#define Absolute(a)  ((a) >= 0.0 ? (a) : -(a))
/* #define Absolute(a)  fabs(a) */

/* Many of the operations are broken up into two pieces, a main part that    */
/*   performs an approximate operation, and a "tail" that computes the       */
/*   roundoff error of that operation.                                       */
/*                                                                           */
/* The operations Fast_Two_Sum(), Fast_Two_Diff(), Two_Sum(), Two_Diff(),    */
/*   Split(), and Two_Product() are all implemented as described in the      */
/*   reference.  Each of these macros requires certain variables to be       */
/*   defined in the calling routine.  The variables `bvirt', `c', `abig',    */
/*   `_i', `_j', `_k', `_l', `_m', and `_n' are declared `INEXACT' because   */
/*   they store the result of an operation that may incur roundoff error.    */
/*   The input parameter `x' (or the highest numbered `x_' parameter) must   */
/*   also be declared `INEXACT'.                                             */

#define Fast_Two_Sum_Tail(a, b, x, y) \
  bvirt = x - a; \
  y = b - bvirt

#define Fast_Two_Sum(a, b, x, y) \
  x = (double) (a + b); \
  Fast_Two_Sum_Tail(a, b, x, y)

#define Two_Sum_Tail(a, b, x, y) \
  bvirt = (double) (x - a); \
  avirt = x - bvirt; \
  bround = b - bvirt; \
  around = a - avirt; \
  y = around + bround

#define Two_Sum(a, b, x, y) \
  x = (double) (a + b); \
  Two_Sum_Tail(a, b, x, y)

#define Two_Diff_Tail(a, b, x, y) \
  bvirt = (double) (a - x); \
  avirt = x + bvirt; \
  bround = bvirt - b; \
  around = a - avirt; \
  y = around + bround

#define Two_Diff(a, b, x, y) \
  x = (double) (a - b); \
  Two_Diff_Tail(a, b, x, y)

#define Split(a, ahi, alo) \
  c = (double) (splitter * a); \
  abig = (double) (c - a); \
  ahi = c - abig; \
  alo = a - ahi

#define Two_Product_Tail(a, b, x, y) \
  Split(a, ahi, alo); \
  Split(b, bhi, blo); \
  err1 = x - (ahi * bhi); \
  err2 = err1 - (alo * bhi); \
  err3 = err2 - (ahi * blo); \
  y = (alo * blo) - err3

#define Two_Product(a, b, x, y) \
  x = (double) (a * b); \
  Two_Product_Tail(a, b, x, y)

/* Two_Product_Presplit() is Two_Product() where one of the inputs has       */
/*   already been split.  Avoids redundant splitting.                        */

#define Two_Product_Presplit(a, b, bhi, blo, x, y) \
  x = (double) (a * b); \
  Split(a, ahi, alo); \
  err1 = x - (ahi * bhi); \
  err2 = err1 - (alo * bhi); \
  err3 = err2 - (ahi * blo); \
  y = (alo * blo) - err3

/* Square() can be done more quickly than Two_Product().                     */

#define Square_Tail(a, x, y) \
  Split(a, ahi, alo); \
  err1 = x - (ahi * ahi); \
  err3 = err1 - ((ahi + ahi) * alo); \
  y = (alo * alo) - err3

#define Square(a, x, y) \
  x = (double) (a * a); \
  Square_Tail(a, x, y)

/* Macros for summing expansions of various fixed lengths.  These are all    */
/*   unrolled versions of Expansion_Sum().                                   */

#define Two_One_Sum(a1, a0, b, x2, x1, x0) \
  Two_Sum(a0, b , _i, x0); \
  Two_Sum(a1, _i, x2, x1)

#define Two_One_Diff(a1, a0, b, x2, x1, x0) \
  Two_Diff(a0, b , _i, x0); \
  Two_Sum( a1, _i, x2, x1)

#define Two_Two_Sum(a1, a0, b1, b0, x3, x2, x1, x0) \
  Two_One_Sum(a1, a0, b0, _j, _0, x0); \
  Two_One_Sum(_j, _0, b1, x3, x2, x1)

#define Two_Two_Diff(a1, a0, b1, b0, x3, x2, x1, x0) \
  Two_One_Diff(a1, a0, b0, _j, _0, x0); \
  Two_One_Diff(_j, _0, b1, x3, x2, x1)

/*****************************************************************************/
/*                                                                           */
/*  exactinit()   Initialize the variables used for exact arithmetic.        */
/*                                                                           */
/*  `epsilon' is the largest power of two such that 1.0 + epsilon = 1.0 in   */
/*  floating-point arithmetic.  `epsilon' bounds the relative roundoff       */
/*  error.  It is used for floating-point error analysis.                    */
/*                                                                           */
/*  `splitter' is used to split floating-point numbers into two half-        */
/*  length significands for exact multiplication.                            */
/*                                                                           */
/*  I imagine that a highly optimizing compiler might be too smart for its   */
/*  own good, and somehow cause this routine to fail, if it pretends that    */
/*  floating-point arithmetic is too much like real arithmetic.              */
/*                                                                           */
/*  Don't change this routine unless you fully understand it.                */
/*                                                                           */
/*****************************************************************************/

LOCAL void exactinit(void)
{
    static boolean first = YES;

    if (first == YES)
    {
        double half;
        double check, lastcheck;
        int every_other;

	first = NO;
        every_other = 1;
        half = 0.5;
        epsilon = 1.0;
        splitter = 1.0;
        check = 1.0;
    /* Repeatedly divide `epsilon' by two until it is too small to add to     */
    /*   one without causing roundoff.  (Also check if the sum is equal to    */
    /*   the previous sum, for machines that round up instead of using exact  */
    /*   rounding.  Not that these routines will work on such machines anyway.*/
        do
        {
	    lastcheck = check;
	    epsilon *= half;
	    if (every_other)
	    {
	        splitter *= 2.0;
	    }
	    every_other = !every_other;
	    check = 1.0 + epsilon;
        } while ((check != 1.0) && (check != lastcheck));
        splitter += 1.0;
        /* Error bounds for orientation and incircle tests. */
        resulterrbound = (3.0 + 8.0 * epsilon) * epsilon;
        ccwerrboundA = (3.0 + 16.0 * epsilon) * epsilon;
        ccwerrboundB = (2.0 + 12.0 * epsilon) * epsilon;
        ccwerrboundC = (9.0 + 64.0 * epsilon) * epsilon * epsilon;
        iccerrboundA = (10.0 + 96.0 * epsilon) * epsilon;
        iccerrboundB = (4.0 + 48.0 * epsilon) * epsilon;
        iccerrboundC = (44.0 + 576.0 * epsilon) * epsilon * epsilon;
    }
}		/*end exactinit*/

/*****************************************************************************/
/*                                                                           */
/*  fast_expansion_sum_zeroelim()   Sum two expansions, eliminating zero     */
/*                                  components from the output expansion.    */
/*                                                                           */
/*  Sets h = e + f.  See my Robust Predicates paper for details.             */
/*                                                                           */
/*  If round-to-even is used (as with IEEE 754), maintains the strongly      */
/*  nonoverlapping property.  (That is, if e is strongly nonoverlapping, h   */
/*  will be also.)  Does NOT maintain the nonoverlapping or nonadjacent      */
/*  properties.                                                              */
/*                                                                           */
/*****************************************************************************/

LOCAL int fast_expansion_sum_zeroelim(
    int elen,
    double *e,
    int flen,
    double *f,
    double *h)  /* h cannot be e or f. */
{
    double Q;
    INEXACT double Qnew;
    INEXACT double hh;
    INEXACT double bvirt;
    double avirt, bround, around;
    int eindex, findex, hindex;
    double enow, fnow;

    enow = e[0];
    fnow = f[0];
    eindex = findex = 0;
    if ((fnow > enow) == (fnow > -enow))
    {
	Q = enow;
	enow = e[++eindex];
    }
    else
    {
	Q = fnow;
	fnow = f[++findex];
    }
    hindex = 0;
    if ((eindex < elen) && (findex < flen))
    {
	if ((fnow > enow) == (fnow > -enow))
	{
	    Fast_Two_Sum(enow, Q, Qnew, hh);
	    enow = e[++eindex];
	}
	else
	{
	    Fast_Two_Sum(fnow, Q, Qnew, hh);
	    fnow = f[++findex];
	}
	Q = Qnew;
	if (hh != 0.0)
	{
	    h[hindex++] = hh;
	}
	while ((eindex < elen) && (findex < flen))
	{
	    if ((fnow > enow) == (fnow > -enow))
	    {
		Two_Sum(Q, enow, Qnew, hh);
		enow = e[++eindex];
	    }
	    else
	    {
		Two_Sum(Q, fnow, Qnew, hh);
		fnow = f[++findex];
	    }
	    Q = Qnew;
	    if (hh != 0.0)
	    {
		h[hindex++] = hh;
	    }
	}
    }
    while (eindex < elen)
    {
	Two_Sum(Q, enow, Qnew, hh);
	enow = e[++eindex];
	Q = Qnew;
	if (hh != 0.0)
	{
	    h[hindex++] = hh;
	}
    }
    while (findex < flen)
    {
	Two_Sum(Q, fnow, Qnew, hh);
	fnow = f[++findex];
	Q = Qnew;
	if (hh != 0.0)
	{
	    h[hindex++] = hh;
	}
    }
    if ((Q != 0.0) || (hindex == 0))
    {
	h[hindex++] = Q;
    }
    return hindex;
}		/*end fast_expansion_sum_zeroelim*/

/*****************************************************************************/
/*                                                                           */
/*  scale_expansion_zeroelim()   Multiply an expansion by a scalar,          */
/*                               eliminating zero components from the        */
/*                               output expansion.                           */
/*                                                                           */
/*  Sets h = be.  See my Robust Predicates paper for details.                */
/*                                                                           */
/*  Maintains the nonoverlapping property.  If round-to-even is used (as     */
/*  with IEEE 754), maintains the strongly nonoverlapping and nonadjacent    */
/*  properties as well.  (That is, if e has one of these properties, so      */
/*  will h.)                                                                 */
/*                                                                           */
/*****************************************************************************/

LOCAL int scale_expansion_zeroelim(
    int elen,
    double *e,
    double b,
    double *h)   /* e and h cannot be the same. */
{
    INEXACT double Q, sum;
    double hh;
    INEXACT double product1;
    double product0;
    int eindex, hindex;
    double enow;
    INEXACT double bvirt;
    double avirt, bround, around;
    INEXACT double c;
    INEXACT double abig;
    double ahi, alo, bhi, blo;
    double err1, err2, err3;

    Split(b, bhi, blo);
    Two_Product_Presplit(e[0], b, bhi, blo, Q, hh);
    hindex = 0;
    if (hh != 0)
    {
	h[hindex++] = hh;
    }
    for (eindex = 1; eindex < elen; eindex++)
    {
	enow = e[eindex];
	Two_Product_Presplit(enow, b, bhi, blo, product1, product0);
	Two_Sum(Q, product0, sum, hh);
	if (hh != 0)
	{
	    h[hindex++] = hh;
	}
	Fast_Two_Sum(product1, sum, Q, hh);
	if (hh != 0)
	{
	    h[hindex++] = hh;
	}
    }
    if ((Q != 0.0) || (hindex == 0))
    {
	h[hindex++] = Q;
    }
    return hindex;
}		/*end scale_expansion_zeroelim*/

/*****************************************************************************/
/*                                                                           */
/*  estimate()   Produce a one-word estimate of an expansion's value.        */
/*                                                                           */
/*  See my Robust Predicates paper for details.                              */
/*                                                                           */
/*****************************************************************************/

LOCAL double estimate(int elen, double *e)
{
    double Q;
    int eindex;

    for (Q = e[0], eindex = 1; eindex < elen; eindex++)
	Q += e[eindex];
    return Q;
}		/*end estimate*/

/*****************************************************************************/
/*                                                                           */
/*  counterclockwise()   Return a positive value if the points pa, pb, and   */
/*                       pc occur in counterclockwise order; a negative      */
/*                       value if they occur in clockwise order; and zero    */
/*                       if they are collinear.  The result is also a rough  */
/*                       approximation of twice the signed area of the       */
/*                       triangle defined by the three points.               */
/*                                                                           */
/*  Uses exact arithmetic if necessary to ensure a correct answer.  The      */
/*  result returned is the determinant of a bi_array.  This determinant is     */
/*  computed adaptively, in the sense that exact arithmetic is used only to  */
/*  the degree it is needed to ensure that the returned value has the        */
/*  correct sign.  Hence, this function is usually quite fast, but will run  */
/*  more slowly when the input points are collinear or nearly so.            */
/*                                                                           */
/*  See my Robust Predicates paper for details.                              */
/*                                                                           */
/*****************************************************************************/

LOCAL double counterclockwiseadapt(
    point pa,
    point pb,
    point pc,
    double detsum)
{
    INEXACT double acx, acy, bcx, bcy;
    double acxtail, acytail, bcxtail, bcytail;
    INEXACT double detleft, detright;
    double detlefttail, detrighttail;
    double det, errbound;
    double B[4], C1[8], C2[12], D[16];
    INEXACT double B3;
    int C1length, C2length, Dlength;
    double u[4];
    INEXACT double u3;
    INEXACT double s1, t1;
    double s0, t0;

    INEXACT double bvirt;
    double avirt, bround, around;
    INEXACT double c;
    INEXACT double abig;
    double ahi, alo, bhi, blo;
    double err1, err2, err3;
    INEXACT double _i, _j;
    double _0;

    acx = (double) (pa[0] - pc[0]);
    bcx = (double) (pb[0] - pc[0]);
    acy = (double) (pa[1] - pc[1]);
    bcy = (double) (pb[1] - pc[1]);

    Two_Product(acx, bcy, detleft, detlefttail);
    Two_Product(acy, bcx, detright, detrighttail);

    Two_Two_Diff(detleft, detlefttail, detright, detrighttail,
        B3, B[2], B[1], B[0]);
    B[3] = B3;

    det = estimate(4, B);
    errbound = ccwerrboundB * detsum;
    if ((det >= errbound) || (-det >= errbound))
    {
	return det;
    }

    Two_Diff_Tail(pa[0], pc[0], acx, acxtail);
    Two_Diff_Tail(pb[0], pc[0], bcx, bcxtail);
    Two_Diff_Tail(pa[1], pc[1], acy, acytail);
    Two_Diff_Tail(pb[1], pc[1], bcy, bcytail);

    if ((acxtail==0.0) && (acytail==0.0) && (bcxtail==0.0) && (bcytail==0.0))
    {
	return det;
    }

    errbound = ccwerrboundC * detsum + resulterrbound * Absolute(det);
    det += (acx * bcytail + bcy * acxtail)
        - (acy * bcxtail + bcx * acytail);
    if ((det >= errbound) || (-det >= errbound))
    {
	return det;
    }

    Two_Product(acxtail, bcy, s1, s0);
    Two_Product(acytail, bcx, t1, t0);
    Two_Two_Diff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
    u[3] = u3;
    C1length = fast_expansion_sum_zeroelim(4, B, 4, u, C1);

    Two_Product(acx, bcytail, s1, s0);
    Two_Product(acy, bcxtail, t1, t0);
    Two_Two_Diff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
    u[3] = u3;
    C2length = fast_expansion_sum_zeroelim(C1length, C1, 4, u, C2);

    Two_Product(acxtail, bcytail, s1, s0);
    Two_Product(acytail, bcxtail, t1, t0);
    Two_Two_Diff(s1, s0, t1, t0, u3, u[2], u[1], u[0]);
    u[3] = u3;
    Dlength = fast_expansion_sum_zeroelim(C2length, C2, 4, u, D);

    return(D[Dlength - 1]);
}		/*end counterclockwiseadapt*/

LOCAL double counterclockwise(point pa, point pb, point pc)
{
    double detleft, detright, det;
    double detsum, errbound;

    counterclockcount++;

    detleft = (pa[0] - pc[0]) * (pb[1] - pc[1]);
    detright = (pa[1] - pc[1]) * (pb[0] - pc[0]);
    det = detleft - detright;

    if (TriOpts.noexact)
	return det;

    if (detleft > 0.0)
    {
	if (detright <= 0.0)
	{
	    return det;
	}
	else
	{
	    detsum = detleft + detright;
	}
    }
    else if (detleft < 0.0)
    {
	if (detright >= 0.0)
	{
	    return det;
	}
	else
	{
	    detsum = -detleft - detright;
	}
    }
    else
    {
	return det;
    }

    errbound = ccwerrboundA * detsum;
    if ((det >= errbound) || (-det >= errbound))
    {
	return det;
    }

    return counterclockwiseadapt(pa, pb, pc, detsum);
}		/*end counterclockwise*/

/*****************************************************************************/
/*                                                                           */
/*  incircle()   Return a positive value if the point pd lies inside the     */
/*               circle passing through pa, pb, and pc; a negative value if  */
/*               it lies outside; and zero if the four points are cocircular.*/
/*               The points pa, pb, and pc must be in counterclockwise       */
/*               order, or the sign of the result will be reversed.          */
/*                                                                           */
/*  Uses exact arithmetic if necessary to ensure a correct answer.  The      */
/*  result returned is the determinant of a bi_array.  This determinant is     */
/*  computed adaptively, in the sense that exact arithmetic is used only to  */
/*  the degree it is needed to ensure that the returned value has the        */
/*  correct sign.  Hence, this function is usually quite fast, but will run  */
/*  more slowly when the input points are cocircular or nearly so.           */
/*                                                                           */
/*  See my Robust Predicates paper for details.                              */
/*                                                                           */
/*****************************************************************************/

LOCAL double incircleadapt(
    point  pa,
    point  pb,
    point  pc,
    point  pd,
    double permanent)
{
    INEXACT double adx, bdx, cdx, ady, bdy, cdy;
    double det, errbound;

    INEXACT double bdxcdy1, cdxbdy1, cdxady1, adxcdy1, adxbdy1, bdxady1;
    double bdxcdy0, cdxbdy0, cdxady0, adxcdy0, adxbdy0, bdxady0;
    double bc[4], ca[4], ab[4];
    INEXACT double bc3, ca3, ab3;
    double axbc[8], axxbc[16], aybc[8], ayybc[16], adet[32];
    int axbclen, axxbclen, aybclen, ayybclen, alen;
    double bxca[8], bxxca[16], byca[8], byyca[16], bdet[32];
    int bxcalen, bxxcalen, bycalen, byycalen, blen;
    double cxab[8], cxxab[16], cyab[8], cyyab[16], cdet[32];
    int cxablen, cxxablen, cyablen, cyyablen, clen;
    double abdet[64];
    int ablen;
    double fin1[1152], fin2[1152];
    double *finnow, *finother, *finswap;
    int finlength;

    double adxtail, bdxtail, cdxtail, adytail, bdytail, cdytail;
    INEXACT double adxadx1, adyady1, bdxbdx1, bdybdy1, cdxcdx1, cdycdy1;
    double adxadx0, adyady0, bdxbdx0, bdybdy0, cdxcdx0, cdycdy0;
    double aa[4], bb[4], cc[4];
    INEXACT double aa3, bb3, cc3;
    INEXACT double ti1, tj1;
    double ti0, tj0;
    double u[4], v[4];
    INEXACT double u3, v3;
    double temp8[8], temp16a[16], temp16b[16], temp16c[16];
    double temp32a[32], temp32b[32], temp48[48], temp64[64];
    int temp8len, temp16alen, temp16blen, temp16clen;
    int temp32alen, temp32blen, temp48len, temp64len;
    double axtbb[8], axtcc[8], aytbb[8], aytcc[8];
    int axtbblen, axtcclen, aytbblen, aytcclen;
    double bxtaa[8], bxtcc[8], bytaa[8], bytcc[8];
    int bxtaalen, bxtcclen, bytaalen, bytcclen;
    double cxtaa[8], cxtbb[8], cytaa[8], cytbb[8];
    int cxtaalen, cxtbblen, cytaalen, cytbblen;
    double axtbc[8], aytbc[8], bxtca[8], bytca[8], cxtab[8], cytab[8];
    int axtbclen, aytbclen, bxtcalen, bytcalen, cxtablen, cytablen;
    double axtbct[16], aytbct[16];
    double bxtcat[16], bytcat[16];
    double cxtabt[16], cytabt[16];
    int axtbctlen, aytbctlen, bxtcatlen, bytcatlen, cxtabtlen, cytabtlen;
    double axtbctt[8], aytbctt[8], bxtcatt[8];
    double bytcatt[8], cxtabtt[8], cytabtt[8];
    int axtbcttlen, aytbcttlen, bxtcattlen, bytcattlen, cxtabttlen, cytabttlen;
    double abt[8], bct[8], cat[8];
    int abtlen, bctlen, catlen;
    double abtt[4], bctt[4], catt[4];
    int abttlen, bcttlen, cattlen;
    INEXACT double abtt3, bctt3, catt3;
    double negate;

    INEXACT double bvirt;
    double avirt, bround, around;
    INEXACT double c;
    INEXACT double abig;
    double ahi, alo, bhi, blo;
    double err1, err2, err3;
    INEXACT double _i, _j;
    double _0;

    adx = (double) (pa[0] - pd[0]);
    bdx = (double) (pb[0] - pd[0]);
    cdx = (double) (pc[0] - pd[0]);
    ady = (double) (pa[1] - pd[1]);
    bdy = (double) (pb[1] - pd[1]);
    cdy = (double) (pc[1] - pd[1]);

    Two_Product(bdx, cdy, bdxcdy1, bdxcdy0);
    Two_Product(cdx, bdy, cdxbdy1, cdxbdy0);
    Two_Two_Diff(bdxcdy1, bdxcdy0, cdxbdy1, cdxbdy0, bc3, bc[2], bc[1], bc[0]);
    bc[3] = bc3;
    axbclen = scale_expansion_zeroelim(4, bc, adx, axbc);
    axxbclen = scale_expansion_zeroelim(axbclen, axbc, adx, axxbc);
    aybclen = scale_expansion_zeroelim(4, bc, ady, aybc);
    ayybclen = scale_expansion_zeroelim(aybclen, aybc, ady, ayybc);
    alen = fast_expansion_sum_zeroelim(axxbclen, axxbc, ayybclen, ayybc, adet);

    Two_Product(cdx, ady, cdxady1, cdxady0);
    Two_Product(adx, cdy, adxcdy1, adxcdy0);
    Two_Two_Diff(cdxady1, cdxady0, adxcdy1, adxcdy0, ca3, ca[2], ca[1], ca[0]);
    ca[3] = ca3;
    bxcalen = scale_expansion_zeroelim(4, ca, bdx, bxca);
    bxxcalen = scale_expansion_zeroelim(bxcalen, bxca, bdx, bxxca);
    bycalen = scale_expansion_zeroelim(4, ca, bdy, byca);
    byycalen = scale_expansion_zeroelim(bycalen, byca, bdy, byyca);
    blen = fast_expansion_sum_zeroelim(bxxcalen, bxxca, byycalen, byyca, bdet);

    Two_Product(adx, bdy, adxbdy1, adxbdy0);
    Two_Product(bdx, ady, bdxady1, bdxady0);
    Two_Two_Diff(adxbdy1, adxbdy0, bdxady1, bdxady0, ab3, ab[2], ab[1], ab[0]);
    ab[3] = ab3;
    cxablen = scale_expansion_zeroelim(4, ab, cdx, cxab);
    cxxablen = scale_expansion_zeroelim(cxablen, cxab, cdx, cxxab);
    cyablen = scale_expansion_zeroelim(4, ab, cdy, cyab);
    cyyablen = scale_expansion_zeroelim(cyablen, cyab, cdy, cyyab);
    clen = fast_expansion_sum_zeroelim(cxxablen, cxxab, cyyablen, cyyab, cdet);

    ablen = fast_expansion_sum_zeroelim(alen, adet, blen, bdet, abdet);
    finlength = fast_expansion_sum_zeroelim(ablen, abdet, clen, cdet, fin1);

    det = estimate(finlength, fin1);
    errbound = iccerrboundB * permanent;
    if ((det >= errbound) || (-det >= errbound))
    {
	return det;
    }

    Two_Diff_Tail(pa[0], pd[0], adx, adxtail);
    Two_Diff_Tail(pa[1], pd[1], ady, adytail);
    Two_Diff_Tail(pb[0], pd[0], bdx, bdxtail);
    Two_Diff_Tail(pb[1], pd[1], bdy, bdytail);
    Two_Diff_Tail(pc[0], pd[0], cdx, cdxtail);
    Two_Diff_Tail(pc[1], pd[1], cdy, cdytail);
    if ((adxtail == 0.0) && (bdxtail == 0.0) && (cdxtail == 0.0)
        && (adytail == 0.0) && (bdytail == 0.0) && (cdytail == 0.0))
    {
	return det;
    }

    errbound = iccerrboundC * permanent + resulterrbound * Absolute(det);
    det += ((adx * adx + ady * ady) * ((bdx * cdytail + cdy * bdxtail)
        - (bdy * cdxtail + cdx * bdytail))
        + 2.0 * (adx * adxtail + ady * adytail) * (bdx * cdy - bdy * cdx))
        + ((bdx * bdx + bdy * bdy) * ((cdx * adytail + ady * cdxtail)
        - (cdy * adxtail + adx * cdytail))
        + 2.0 * (bdx * bdxtail + bdy * bdytail) * (cdx * ady - cdy * adx))
        + ((cdx * cdx + cdy * cdy) * ((adx * bdytail + bdy * adxtail)
        - (ady * bdxtail + bdx * adytail))
        + 2.0 * (cdx * cdxtail + cdy * cdytail) * (adx * bdy - ady * bdx));
    if ((det >= errbound) || (-det >= errbound))
    {
	return det;
    }

    finnow = fin1;
    finother = fin2;

    if ((bdxtail != 0.0) || (bdytail != 0.0)
        || (cdxtail != 0.0) || (cdytail != 0.0))
    {
	Square(adx, adxadx1, adxadx0);
	Square(ady, adyady1, adyady0);
	Two_Two_Sum(adxadx1,adxadx0,adyady1,adyady0,aa3,aa[2],aa[1],aa[0]);
	aa[3] = aa3;
    }
    if ((cdxtail != 0.0) || (cdytail != 0.0)
        || (adxtail != 0.0) || (adytail != 0.0))
    {
	Square(bdx, bdxbdx1, bdxbdx0);
	Square(bdy, bdybdy1, bdybdy0);
	Two_Two_Sum(bdxbdx1, bdxbdx0, bdybdy1, bdybdy0, bb3, bb[2], bb[1], bb[0]);
	bb[3] = bb3;
    }
    if ((adxtail != 0.0) || (adytail != 0.0)
        || (bdxtail != 0.0) || (bdytail != 0.0))
    {
	Square(cdx, cdxcdx1, cdxcdx0);
	Square(cdy, cdycdy1, cdycdy0);
	Two_Two_Sum(cdxcdx1, cdxcdx0, cdycdy1, cdycdy0, cc3, cc[2], cc[1], cc[0]);
	cc[3] = cc3;
    }

    if (adxtail != 0.0)
    {
	axtbclen = scale_expansion_zeroelim(4, bc, adxtail, axtbc);
	temp16alen = scale_expansion_zeroelim(axtbclen, axtbc, 2.0 * adx,
	    temp16a);

	axtcclen = scale_expansion_zeroelim(4, cc, adxtail, axtcc);
	temp16blen = scale_expansion_zeroelim(axtcclen, axtcc, bdy, temp16b);

	axtbblen = scale_expansion_zeroelim(4, bb, adxtail, axtbb);
	temp16clen = scale_expansion_zeroelim(axtbblen, axtbb, -cdy, temp16c);

	temp32alen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	    temp16blen, temp16b, temp32a);
	temp48len = fast_expansion_sum_zeroelim(temp16clen, temp16c,
	    temp32alen, temp32a, temp48);
	finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	    temp48, finother);
	finswap = finnow; 
	finnow = finother; 
	finother = finswap;
    }
    if (adytail != 0.0)
    {
	aytbclen = scale_expansion_zeroelim(4, bc, adytail, aytbc);
	temp16alen = scale_expansion_zeroelim(aytbclen, aytbc, 2.0 * ady,
	    temp16a);

	aytbblen = scale_expansion_zeroelim(4, bb, adytail, aytbb);
	temp16blen = scale_expansion_zeroelim(aytbblen, aytbb, cdx, temp16b);

	aytcclen = scale_expansion_zeroelim(4, cc, adytail, aytcc);
	temp16clen = scale_expansion_zeroelim(aytcclen, aytcc, -bdx, temp16c);

	temp32alen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	    temp16blen, temp16b, temp32a);
	temp48len = fast_expansion_sum_zeroelim(temp16clen, temp16c,
	    temp32alen, temp32a, temp48);
	finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	    temp48, finother);
	finswap = finnow; 
	finnow = finother; 
	finother = finswap;
    }
    if (bdxtail != 0.0)
    {
	bxtcalen = scale_expansion_zeroelim(4, ca, bdxtail, bxtca);
	temp16alen = scale_expansion_zeroelim(bxtcalen, bxtca, 2.0 * bdx,
	    temp16a);

	bxtaalen = scale_expansion_zeroelim(4, aa, bdxtail, bxtaa);
	temp16blen = scale_expansion_zeroelim(bxtaalen, bxtaa, cdy, temp16b);

	bxtcclen = scale_expansion_zeroelim(4, cc, bdxtail, bxtcc);
	temp16clen = scale_expansion_zeroelim(bxtcclen, bxtcc, -ady, temp16c);

	temp32alen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	    temp16blen, temp16b, temp32a);
	temp48len = fast_expansion_sum_zeroelim(temp16clen, temp16c,
	    temp32alen, temp32a, temp48);
	finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	    temp48, finother);
	finswap = finnow; 
	finnow = finother; 
	finother = finswap;
    }
    if (bdytail != 0.0)
    {
	bytcalen = scale_expansion_zeroelim(4, ca, bdytail, bytca);
	temp16alen = scale_expansion_zeroelim(bytcalen, bytca, 2.0 * bdy,
	    temp16a);

	bytcclen = scale_expansion_zeroelim(4, cc, bdytail, bytcc);
	temp16blen = scale_expansion_zeroelim(bytcclen, bytcc, adx, temp16b);

	bytaalen = scale_expansion_zeroelim(4, aa, bdytail, bytaa);
	temp16clen = scale_expansion_zeroelim(bytaalen, bytaa, -cdx, temp16c);

	temp32alen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	    temp16blen, temp16b, temp32a);
	temp48len = fast_expansion_sum_zeroelim(temp16clen, temp16c,
	    temp32alen, temp32a, temp48);
	finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	    temp48, finother);
	finswap = finnow; 
	finnow = finother; 
	finother = finswap;
    }
    if (cdxtail != 0.0)
    {
	cxtablen = scale_expansion_zeroelim(4, ab, cdxtail, cxtab);
	temp16alen = scale_expansion_zeroelim(cxtablen, cxtab, 2.0 * cdx,
	    temp16a);

	cxtbblen = scale_expansion_zeroelim(4, bb, cdxtail, cxtbb);
	temp16blen = scale_expansion_zeroelim(cxtbblen, cxtbb, ady, temp16b);

	cxtaalen = scale_expansion_zeroelim(4, aa, cdxtail, cxtaa);
	temp16clen = scale_expansion_zeroelim(cxtaalen, cxtaa, -bdy, temp16c);

	temp32alen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	    temp16blen, temp16b, temp32a);
	temp48len = fast_expansion_sum_zeroelim(temp16clen, temp16c,
	    temp32alen, temp32a, temp48);
	finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	    temp48, finother);
	finswap = finnow; 
	finnow = finother; 
	finother = finswap;
    }
    if (cdytail != 0.0)
    {
	cytablen = scale_expansion_zeroelim(4, ab, cdytail, cytab);
	temp16alen = scale_expansion_zeroelim(cytablen, cytab, 2.0 * cdy,
	    temp16a);

	cytaalen = scale_expansion_zeroelim(4, aa, cdytail, cytaa);
	temp16blen = scale_expansion_zeroelim(cytaalen, cytaa, bdx, temp16b);

	cytbblen = scale_expansion_zeroelim(4, bb, cdytail, cytbb);
	temp16clen = scale_expansion_zeroelim(cytbblen, cytbb, -adx, temp16c);

	temp32alen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	    temp16blen, temp16b, temp32a);
	temp48len = fast_expansion_sum_zeroelim(temp16clen, temp16c,
	    temp32alen, temp32a, temp48);
	finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	    temp48, finother);
	finswap = finnow; 
	finnow = finother; 
	finother = finswap;
    }

    if ((adxtail != 0.0) || (adytail != 0.0))
    {
	if ((bdxtail != 0.0) || (bdytail != 0.0)
	    || (cdxtail != 0.0) || (cdytail != 0.0))
	{
	    Two_Product(bdxtail, cdy, ti1, ti0);
	    Two_Product(bdx, cdytail, tj1, tj0);
	    Two_Two_Sum(ti1, ti0, tj1, tj0, u3, u[2], u[1], u[0]);
	    u[3] = u3;
	    negate = -bdy;
	    Two_Product(cdxtail, negate, ti1, ti0);
	    negate = -bdytail;
	    Two_Product(cdx, negate, tj1, tj0);
	    Two_Two_Sum(ti1, ti0, tj1, tj0, v3, v[2], v[1], v[0]);
	    v[3] = v3;
	    bctlen = fast_expansion_sum_zeroelim(4, u, 4, v, bct);

	    Two_Product(bdxtail, cdytail, ti1, ti0);
	    Two_Product(cdxtail, bdytail, tj1, tj0);
	    Two_Two_Diff(ti1, ti0, tj1, tj0, bctt3, bctt[2], bctt[1], bctt[0]);
	    bctt[3] = bctt3;
	    bcttlen = 4;
	}
	else
	{
	    bct[0] = 0.0;
	    bctlen = 1;
	    bctt[0] = 0.0;
	    bcttlen = 1;
	}

	if (adxtail != 0.0)
	{
	    temp16alen = scale_expansion_zeroelim(axtbclen, axtbc, adxtail, temp16a);
	    axtbctlen = scale_expansion_zeroelim(bctlen, bct, adxtail, axtbct);
	    temp32alen = scale_expansion_zeroelim(axtbctlen, axtbct, 2.0 * adx,
	        temp32a);
	    temp48len = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp32alen, temp32a, temp48);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	        temp48, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	    if (bdytail != 0.0)
	    {
		temp8len = scale_expansion_zeroelim(4, cc, adxtail, temp8);
		temp16alen = scale_expansion_zeroelim(temp8len, temp8, bdytail,
		    temp16a);
		finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp16alen,
		    temp16a, finother);
		finswap = finnow; 
		finnow = finother; 
		finother = finswap;
	    }
	    if (cdytail != 0.0)
	    {
		temp8len = scale_expansion_zeroelim(4, bb, -adxtail, temp8);
		temp16alen = scale_expansion_zeroelim(temp8len, temp8, cdytail,
		    temp16a);
		finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp16alen,
		    temp16a, finother);
		finswap = finnow; 
		finnow = finother; 
		finother = finswap;
	    }

	    temp32alen = scale_expansion_zeroelim(axtbctlen, axtbct, adxtail,
	        temp32a);
	    axtbcttlen = scale_expansion_zeroelim(bcttlen, bctt, adxtail, axtbctt);
	    temp16alen = scale_expansion_zeroelim(axtbcttlen, axtbctt, 2.0 * adx,
	        temp16a);
	    temp16blen = scale_expansion_zeroelim(axtbcttlen, axtbctt, adxtail,
	        temp16b);
	    temp32blen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp16blen, temp16b, temp32b);
	    temp64len = fast_expansion_sum_zeroelim(temp32alen, temp32a,
	        temp32blen, temp32b, temp64);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp64len,
	        temp64, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	}
	if (adytail != 0.0)
	{
	    temp16alen = scale_expansion_zeroelim(aytbclen, aytbc, adytail, temp16a);
	    aytbctlen = scale_expansion_zeroelim(bctlen, bct, adytail, aytbct);
	    temp32alen = scale_expansion_zeroelim(aytbctlen, aytbct, 2.0 * ady,
	        temp32a);
	    temp48len = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp32alen, temp32a, temp48);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	        temp48, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;


	    temp32alen = scale_expansion_zeroelim(aytbctlen, aytbct, adytail,
	        temp32a);
	    aytbcttlen = scale_expansion_zeroelim(bcttlen, bctt, adytail, aytbctt);
	    temp16alen = scale_expansion_zeroelim(aytbcttlen, aytbctt, 2.0 * ady,
	        temp16a);
	    temp16blen = scale_expansion_zeroelim(aytbcttlen, aytbctt, adytail,
	        temp16b);
	    temp32blen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp16blen, temp16b, temp32b);
	    temp64len = fast_expansion_sum_zeroelim(temp32alen, temp32a,
	        temp32blen, temp32b, temp64);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp64len,
	        temp64, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	}
    }
    if ((bdxtail != 0.0) || (bdytail != 0.0))
    {
	if ((cdxtail != 0.0) || (cdytail != 0.0)
	    || (adxtail != 0.0) || (adytail != 0.0))
	{
	    Two_Product(cdxtail, ady, ti1, ti0);
	    Two_Product(cdx, adytail, tj1, tj0);
	    Two_Two_Sum(ti1, ti0, tj1, tj0, u3, u[2], u[1], u[0]);
	    u[3] = u3;
	    negate = -cdy;
	    Two_Product(adxtail, negate, ti1, ti0);
	    negate = -cdytail;
	    Two_Product(adx, negate, tj1, tj0);
	    Two_Two_Sum(ti1, ti0, tj1, tj0, v3, v[2], v[1], v[0]);
	    v[3] = v3;
	    catlen = fast_expansion_sum_zeroelim(4, u, 4, v, cat);

	    Two_Product(cdxtail, adytail, ti1, ti0);
	    Two_Product(adxtail, cdytail, tj1, tj0);
	    Two_Two_Diff(ti1, ti0, tj1, tj0, catt3, catt[2], catt[1], catt[0]);
	    catt[3] = catt3;
	    cattlen = 4;
	}
	else
	{
	    cat[0] = 0.0;
	    catlen = 1;
	    catt[0] = 0.0;
	    cattlen = 1;
	}

	if (bdxtail != 0.0)
	{
	    temp16alen = scale_expansion_zeroelim(bxtcalen, bxtca, bdxtail, temp16a);
	    bxtcatlen = scale_expansion_zeroelim(catlen, cat, bdxtail, bxtcat);
	    temp32alen = scale_expansion_zeroelim(bxtcatlen, bxtcat, 2.0 * bdx,
	        temp32a);
	    temp48len = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp32alen, temp32a, temp48);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	        temp48, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	    if (cdytail != 0.0)
	    {
		temp8len = scale_expansion_zeroelim(4, aa, bdxtail, temp8);
		temp16alen = scale_expansion_zeroelim(temp8len, temp8, cdytail,
		    temp16a);
		finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp16alen,
		    temp16a, finother);
		finswap = finnow; 
		finnow = finother; 
		finother = finswap;
	    }
	    if (adytail != 0.0)
	    {
		temp8len = scale_expansion_zeroelim(4, cc, -bdxtail, temp8);
		temp16alen = scale_expansion_zeroelim(temp8len, temp8, adytail,
		    temp16a);
		finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp16alen,
		    temp16a, finother);
		finswap = finnow; 
		finnow = finother; 
		finother = finswap;
	    }

	    temp32alen = scale_expansion_zeroelim(bxtcatlen, bxtcat, bdxtail,
	        temp32a);
	    bxtcattlen = scale_expansion_zeroelim(cattlen, catt, bdxtail, bxtcatt);
	    temp16alen = scale_expansion_zeroelim(bxtcattlen, bxtcatt, 2.0 * bdx,
	        temp16a);
	    temp16blen = scale_expansion_zeroelim(bxtcattlen, bxtcatt, bdxtail,
	        temp16b);
	    temp32blen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp16blen, temp16b, temp32b);
	    temp64len = fast_expansion_sum_zeroelim(temp32alen, temp32a,
	        temp32blen, temp32b, temp64);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp64len,
	        temp64, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	}
	if (bdytail != 0.0)
	{
	    temp16alen = scale_expansion_zeroelim(bytcalen, bytca, bdytail, temp16a);
	    bytcatlen = scale_expansion_zeroelim(catlen, cat, bdytail, bytcat);
	    temp32alen = scale_expansion_zeroelim(bytcatlen, bytcat, 2.0 * bdy,
	        temp32a);
	    temp48len = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp32alen, temp32a, temp48);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	        temp48, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;


	    temp32alen = scale_expansion_zeroelim(bytcatlen, bytcat, bdytail,
	        temp32a);
	    bytcattlen = scale_expansion_zeroelim(cattlen, catt, bdytail, bytcatt);
	    temp16alen = scale_expansion_zeroelim(bytcattlen, bytcatt, 2.0 * bdy,
	        temp16a);
	    temp16blen = scale_expansion_zeroelim(bytcattlen, bytcatt, bdytail,
	        temp16b);
	    temp32blen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp16blen, temp16b, temp32b);
	    temp64len = fast_expansion_sum_zeroelim(temp32alen, temp32a,
	        temp32blen, temp32b, temp64);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp64len,
	        temp64, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	}
    }
    if ((cdxtail != 0.0) || (cdytail != 0.0))
    {
	if ((adxtail != 0.0) || (adytail != 0.0)
	    || (bdxtail != 0.0) || (bdytail != 0.0))
	    {
	    Two_Product(adxtail, bdy, ti1, ti0);
	    Two_Product(adx, bdytail, tj1, tj0);
	    Two_Two_Sum(ti1, ti0, tj1, tj0, u3, u[2], u[1], u[0]);
	    u[3] = u3;
	    negate = -ady;
	    Two_Product(bdxtail, negate, ti1, ti0);
	    negate = -adytail;
	    Two_Product(bdx, negate, tj1, tj0);
	    Two_Two_Sum(ti1, ti0, tj1, tj0, v3, v[2], v[1], v[0]);
	    v[3] = v3;
	    abtlen = fast_expansion_sum_zeroelim(4, u, 4, v, abt);

	    Two_Product(adxtail, bdytail, ti1, ti0);
	    Two_Product(bdxtail, adytail, tj1, tj0);
	    Two_Two_Diff(ti1, ti0, tj1, tj0, abtt3, abtt[2], abtt[1], abtt[0]);
	    abtt[3] = abtt3;
	    abttlen = 4;
	}
	else
	{
	    abt[0] = 0.0;
	    abtlen = 1;
	    abtt[0] = 0.0;
	    abttlen = 1;
	}

	if (cdxtail != 0.0)
	{
	    temp16alen = scale_expansion_zeroelim(cxtablen, cxtab, cdxtail, temp16a);
	    cxtabtlen = scale_expansion_zeroelim(abtlen, abt, cdxtail, cxtabt);
	    temp32alen = scale_expansion_zeroelim(cxtabtlen, cxtabt, 2.0 * cdx,
	        temp32a);
	    temp48len = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp32alen, temp32a, temp48);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	        temp48, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	    if (adytail != 0.0)
	    {
		temp8len = scale_expansion_zeroelim(4, bb, cdxtail, temp8);
		temp16alen = scale_expansion_zeroelim(temp8len, temp8, adytail,
		    temp16a);
		finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp16alen,
		    temp16a, finother);
		finswap = finnow; 
		finnow = finother; 
		finother = finswap;
	    }
	    if (bdytail != 0.0)
	    {
		temp8len = scale_expansion_zeroelim(4, aa, -cdxtail, temp8);
		temp16alen = scale_expansion_zeroelim(temp8len, temp8, bdytail,
		    temp16a);
		finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp16alen,
		    temp16a, finother);
		finswap = finnow; 
		finnow = finother; 
		finother = finswap;
	    }

	    temp32alen = scale_expansion_zeroelim(cxtabtlen, cxtabt, cdxtail,
	        temp32a);
	    cxtabttlen = scale_expansion_zeroelim(abttlen, abtt, cdxtail, cxtabtt);
	    temp16alen = scale_expansion_zeroelim(cxtabttlen, cxtabtt, 2.0 * cdx,
	        temp16a);
	    temp16blen = scale_expansion_zeroelim(cxtabttlen, cxtabtt, cdxtail,
	        temp16b);
	    temp32blen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp16blen, temp16b, temp32b);
	    temp64len = fast_expansion_sum_zeroelim(temp32alen, temp32a,
	        temp32blen, temp32b, temp64);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp64len,
	        temp64, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	}
	if (cdytail != 0.0)
	{
	    temp16alen = scale_expansion_zeroelim(cytablen, cytab, cdytail, temp16a);
	    cytabtlen = scale_expansion_zeroelim(abtlen, abt, cdytail, cytabt);
	    temp32alen = scale_expansion_zeroelim(cytabtlen, cytabt, 2.0 * cdy,
	        temp32a);
	    temp48len = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp32alen, temp32a, temp48);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp48len,
	        temp48, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;


	    temp32alen = scale_expansion_zeroelim(cytabtlen, cytabt, cdytail,
	        temp32a);
	    cytabttlen = scale_expansion_zeroelim(abttlen, abtt, cdytail, cytabtt);
	    temp16alen = scale_expansion_zeroelim(cytabttlen, cytabtt, 2.0 * cdy,
	        temp16a);
	    temp16blen = scale_expansion_zeroelim(cytabttlen, cytabtt, cdytail,
	        temp16b);
	    temp32blen = fast_expansion_sum_zeroelim(temp16alen, temp16a,
	        temp16blen, temp16b, temp32b);
	    temp64len = fast_expansion_sum_zeroelim(temp32alen, temp32a,
	        temp32blen, temp32b, temp64);
	    finlength = fast_expansion_sum_zeroelim(finlength, finnow, temp64len,
	        temp64, finother);
	    finswap = finnow; 
	    finnow = finother; 
	    finother = finswap;
	}
    }

    return finnow[finlength - 1];
}		/*end incircleadapt*/

LOCAL double incircle(point pa, point pb, point pc, point pd)
{
    double adx, bdx, cdx, ady, bdy, cdy;
    double bdxcdy, cdxbdy, cdxady, adxcdy, adxbdy, bdxady;
    double alift, blift, clift;
    double det;
    double permanent, errbound;

    incirclecount++;

    adx = pa[0] - pd[0];
    bdx = pb[0] - pd[0];
    cdx = pc[0] - pd[0];
    ady = pa[1] - pd[1];
    bdy = pb[1] - pd[1];
    cdy = pc[1] - pd[1];

    bdxcdy = bdx * cdy;
    cdxbdy = cdx * bdy;
    alift = adx * adx + ady * ady;

    cdxady = cdx * ady;
    adxcdy = adx * cdy;
    blift = bdx * bdx + bdy * bdy;

    adxbdy = adx * bdy;
    bdxady = bdx * ady;
    clift = cdx * cdx + cdy * cdy;

    det = alift * (bdxcdy - cdxbdy)
        + blift * (cdxady - adxcdy)
        + clift * (adxbdy - bdxady);

    if (TriOpts.noexact)
	return det;

    permanent = (Absolute(bdxcdy) + Absolute(cdxbdy)) * alift
        + (Absolute(cdxady) + Absolute(adxcdy)) * blift
        + (Absolute(adxbdy) + Absolute(bdxady)) * clift;
    errbound = iccerrboundA * permanent;
    if ((det > errbound) || (-det > errbound))
    {
	return det;
    }

    return incircleadapt(pa, pb, pc, pd, permanent);
}		/*end incircle*/

/*****************************************************************************/
/*                                                                           */
/*  triangleinit()   Initialize some variables.                              */
/*                                                                           */
/*****************************************************************************/

LOCAL void triangleinit(void)
{
    points.maxitems = triangles.maxitems = shelles.maxitems =
	viri.maxitems = badsegments.maxitems = 0l;
    points.itembytes = triangles.itembytes = shelles.itembytes =
	viri.itembytes = badsegments.itembytes = 0;
    recenttri.tri = NULL;    /* No triangle has been visited yet. */
    samples = 1;           /* Point location should take at least one sample. */
    checksegments = 0;     /* There are no segments in the triangulation yet. */
    incirclecount = counterclockcount = 0;
    randomseed = 1;

    exactinit();                  /* Initialize exact arithmetic constants. */
}		/*end triangleinit*/

/*****************************************************************************/
/*                                                                           */
/*  randomnation()   Generate a random number between 0 and `choices' - 1.   */
/*                                                                           */
/*  This is a simple linear congruential random number generator.  Hence, it */
/*  is a bad random number generator, but good enough for most randomized    */
/*  geometric algorithms.                                                    */
/*                                                                           */
/*****************************************************************************/

LOCAL size_t randomnation(size_t choices)
{
    randomseed = (randomseed * 1366l + 150889l) % 714025l;
    return randomseed / (714025l / choices + 1);
}		/*end randomnation*/

/*****************************************************************************/
/*                                                                           */
/*  makepointmap()   Construct a mapping from points to triangles to improve */
/*                  the speed of point location for segment insertion.       */
/*                                                                           */
/*  Traverses all the triangles, and provides each corner of each triangle   */
/*  with a pointer to that triangle.  Of course, pointers will be            */
/*  overwritten by other pointers because (almost) each point is a corner    */
/*  of several triangles, but in the end every point will point to some      */
/*  triangle that contains it.                                               */
/*                                                                           */
/*****************************************************************************/

LOCAL void makepointmap(void)
{
    triedge triangleloop;
    point triorg;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    while (triangleloop.tri != NULL)
    {
	/* Check all three points of the triangle. */
	for (triangleloop.orient = 0; triangleloop.orient < 3;
	    triangleloop.orient++)
	    {
	    org(triangleloop, triorg);
	    setpoint2tri(triorg, encode(triangleloop));
	}
	triangleloop.tri = triangletraverse();
    }
}		/*end makepointmap*/

/*****************************************************************************/
/*                                                                           */
/*  preciselocate()   Find a triangle or edge containing a given point.      */
/*                                                                           */
/*  Begins its search from `searchtri'.  It is important that `searchtri'    */
/*  be a handle with the property that `searchpoint' is strictly to the left */
/*  of the edge denoted by `searchtri', or is collinear with that edge and   */
/*  does not intersect that edge.  (In particular, `searchpoint' should not  */
/*  be the origin or destination of that edge.)                              */
/*                                                                           */
/*  These conditions are imposed because preciselocate() is normally used in */
/*  one of two situations:                                                   */
/*                                                                           */
/*  (1)  To try to find the location to insert a new point.  Normally, we    */
/*       know an edge that the point is strictly to the left of.  In the     */
/*       incremental Delaunay algorithm, that edge is a bounding box edge.   */
/*       In Ruppert's Delaunay refinement algorithm for quality meshing,     */
/*       that edge is the shortest edge of the triangle whose circumcenter   */
/*       is being inserted.                                                  */
/*                                                                           */
/*  (2)  To try to find an existing point.  In this case, any edge on the    */
/*       convex hull is a good starting edge.  The possibility that the      */
/*       vertex one seeks is an endpoint of the starting edge must be        */
/*       screened out before preciselocate() is called.                      */
/*                                                                           */
/*  On completion, `searchtri' is a triangle that contains `searchpoint'.    */
/*                                                                           */
/*  This implementation differs from that given by Guibas and Stolfi.  It    */
/*  walks from triangle to triangle, crossing an edge only if `searchpoint'  */
/*  is on the other side of the line containing that edge.  After entering   */
/*  a triangle, there are two edges by which one can leave that triangle.    */
/*  If both edges are valid (`searchpoint' is on the other side of both      */
/*  edges), one of the two is chosen by drawing a line perpendicular to      */
/*  the entry edge (whose endpoints are `forg' and `fdest') passing through  */
/*  `fapex'.  Depending on which side of this perpendicular `searchpoint'    */
/*  falls on, an exit edge is chosen.                                        */
/*                                                                           */
/*  This implementation is empirically faster than the Guibas and Stolfi     */
/*  point location routine (which I originally used), which tends to spiral  */
/*  in toward its target.                                                    */
/*                                                                           */
/*  Returns ONVERTEX if the point lies on an existing vertex.  `searchtri'   */
/*  is a handle whose origin is the existing vertex.                         */
/*                                                                           */
/*  Returns ONEDGE if the point lies on a mesh edge.  `searchtri' is a       */
/*  handle whose primary edge is the edge on which the point lies.           */
/*                                                                           */
/*  Returns INTRIANGLE if the point lies strictly within a triangle.         */
/*  `searchtri' is a handle on the triangle that contains the point.         */
/*                                                                           */
/*  Returns OUTSIDE if the point lies outside the mesh.  `searchtri' is a    */
/*  handle whose primary edge the point is to the right of.  This might      */
/*  occur when the circumcenter of a triangle falls just slightly outside    */
/*  the mesh due to floating-point roundoff error.  It also occurs when      */
/*  seeking a hole or region point that a foolish user has placed outside    */
/*  the mesh.                                                                */
/*                                                                           */
/*  WARNING:  This routine is designed for convex triangulations, and will   */
/*  not generally work after the holes and concavities have been carved.     */
/*  However, it can still be used to find the circumcenter of a triangle, as */
/*  long as the search is begun from the triangle in question.               */
/*                                                                           */
/*****************************************************************************/

LOCAL locateresult preciselocate(
    point searchpoint,
    triedge *searchtri)
{
    triedge backtracktri;
    point forg, fdest, fapex;
    point swappoint;
    double orgorient, destorient;
    int moveleft;
    triangle ptr;                       /* Temporary variable used by sym(). */

    /* Where are we? */
    org(*searchtri, forg);
    dest(*searchtri, fdest);
    apex(*searchtri, fapex);
    while (1)
    {
	/* Check whether the apex is the point we seek. */
	if ((fapex[0] == searchpoint[0]) && (fapex[1] == searchpoint[1]))
	{
	    lprevself(*searchtri);
	    return ONVERTEX;
	}
	/* Does the point lie on the other side of the line defined by the */
	/*   triangle edge opposite the triangle's destination?            */
	destorient = counterclockwise(forg, fapex, searchpoint);
	/* Does the point lie on the other side of the line defined by the */
	/*   triangle edge opposite the triangle's origin?                 */
	orgorient = counterclockwise(fapex, fdest, searchpoint);
	if (destorient > 0.0)
	{
	    if (orgorient > 0.0)
	    {
		/* Move left if the inner product of (fapex - searchpoint) and  */
		/*   (fdest - forg) is positive.  This is equivalent to drawing */
		/*   a line perpendicular to the line (forg, fdest) passing     */
		/*   through `fapex', and determining which side of this line   */
		/*   `searchpoint' falls on.                                    */
		moveleft = (fapex[0] - searchpoint[0]) * (fdest[0] - forg[0]) +
		    (fapex[1] - searchpoint[1]) * (fdest[1] - forg[1]) > 0.0;
	    }
	    else
	    {
		moveleft = 1;
	    }
	}
	else
	{
	    if (orgorient > 0.0)
	    {
		moveleft = 0;
	    }
	    else
	    {
		/* The point we seek must be on the boundary of or inside this */
		/*   triangle.                                                 */
		if (destorient == 0.0)
		{
		    lprevself(*searchtri);
		    return ONEDGE;
		}
		if (orgorient == 0.0)
		{
		    lnextself(*searchtri);
		    return ONEDGE;
		}
		return INTRIANGLE;
	    }
	}

	/* Move to another triangle.  Leave a trace `backtracktri' in case */
	/*   floating-point roundoff or some such bogey causes us to walk  */
	/*   off a boundary of the triangulation.  We can just bounce off  */
	/*   the boundary as if it were an elastic band.                   */
	if (moveleft)
	{
	    lprev(*searchtri, backtracktri);
	    fdest = fapex;
	}
	else
	{
	    lnext(*searchtri, backtracktri);
	    forg = fapex;
	}
	sym(backtracktri, *searchtri);

	/* Check for walking off the edge. */
	if (searchtri->tri == dummytri)
	{
	    /* Turn around. */
	    triedgecopy(backtracktri, *searchtri);
	    swappoint = forg;
	    forg = fdest;
	    fdest = swappoint;
	    apex(*searchtri, fapex);
	    /* Check if the point really is beyond the */
	    /* triangulation boundary. */
	    destorient = counterclockwise(forg, fapex, searchpoint);
	    orgorient = counterclockwise(fapex, fdest, searchpoint);
	    if ((orgorient < 0.0) && (destorient < 0.0))
	    {
		return OUTSIDE;
	    }
	}
	else
	{
	    apex(*searchtri, fapex);
	}
    }
}		/*end preciselocate*/

/*****************************************************************************/
/*                                                                           */
/*  locate()   Find a triangle or edge containing a given point.             */
/*                                                                           */
/*  Searching begins from one of:  the input `searchtri', a recently         */
/*  encountered triangle `recenttri', or from a triangle chosen from a       */
/*  random sample.  The choice is made by determining which triangle's       */
/*  origin is closest to the point we are searcing for.  Normally,           */
/*  `searchtri' should be a handle on the convex hull of the triangulation.  */
/*                                                                           */
/*  Details on the random sampling method can be found in the Mucke, Saias,  */
/*  and Zhu paper cited in the header of this code.                          */
/*                                                                           */
/*  On completion, `searchtri' is a triangle that contains `searchpoint'.    */
/*                                                                           */
/*  Returns ONVERTEX if the point lies on an existing vertex.  `searchtri'   */
/*  is a handle whose origin is the existing vertex.                         */
/*                                                                           */
/*  Returns ONEDGE if the point lies on a mesh edge.  `searchtri' is a       */
/*  handle whose primary edge is the edge on which the point lies.           */
/*                                                                           */
/*  Returns INTRIANGLE if the point lies strictly within a triangle.         */
/*  `searchtri' is a handle on the triangle that contains the point.         */
/*                                                                           */
/*  Returns OUTSIDE if the point lies outside the mesh.  `searchtri' is a    */
/*  handle whose primary edge the point is to the right of.  This might      */
/*  occur when the circumcenter of a triangle falls just slightly outside    */
/*  the mesh due to floating-point roundoff error.  It also occurs when      */
/*  seeking a hole or region point that a foolish user has placed outside    */
/*  the mesh.                                                                */
/*                                                                           */
/*  WARNING:  This routine is designed for convex triangulations, and will   */
/*  not generally work after the holes and concavities have been carved.     */
/*                                                                           */
/*****************************************************************************/

LOCAL locateresult locate(
    point searchpoint,
    triedge *searchtri)
{
    PoolBlock *sampleblock;
    triangle *firsttri;
    triedge sampletri;
    point torg, tdest;
    double searchdist, dist;
    double ahead;
    long sampleblocks, samplesperblock;
    size_t samplenum;
    long triblocks;
    long i, j;
    triangle ptr;                       /* Temporary variable used by sym(). */

    /* Record the distance from the suggested starting triangle to the */
    /*   point we seek.                                                */
    org(*searchtri, torg);
    searchdist = (searchpoint[0] - torg[0]) * (searchpoint[0] - torg[0])
        + (searchpoint[1] - torg[1]) * (searchpoint[1] - torg[1]);

    /* If a recently encountered triangle has been recorded and has not been */
    /*   deallocated, test it as a good starting point.                      */
    if (recenttri.tri != NULL)
    {
	if (recenttri.tri[3] != NULL)
	{
	    org(recenttri, torg);
	    if ((torg[0] == searchpoint[0]) && (torg[1] == searchpoint[1]))
	    {
		triedgecopy(recenttri, *searchtri);
		return ONVERTEX;
	    }
	    dist = (searchpoint[0] - torg[0]) * (searchpoint[0] - torg[0])
	        + (searchpoint[1] - torg[1]) * (searchpoint[1] - torg[1]);
	    if (dist < searchdist)
	    {
		triedgecopy(recenttri, *searchtri);
		searchdist = dist;
	    }
	}
    }

    /* The number of random samples taken is proportional to the cube root of */
    /*   the number of triangles in the mesh.  The next bit of code assumes   */
    /*   that the number of triangles increases monotonically.                */
    while (SAMPLEFACTOR * samples * samples * samples < triangles.items)
    {
	samples++;
    }
    triblocks = (triangles.maxitems + TRIPERBLOCK - 1) / TRIPERBLOCK;
    samplesperblock = 1 + (samples / triblocks);
    sampleblocks = samples / samplesperblock;
    sampleblock = triangles.firstblock;
    sampletri.orient = 0;
    for (i = 0; i < sampleblocks; i++)
    {
	firsttri = (triangle *) FirstItemInBlock(sampleblock);
	for (j = 0; j < samplesperblock; j++)
	{
	    if (i == triblocks - 1)
	    {
		samplenum = randomnation((triangles.maxitems-(i*TRIPERBLOCK)));
	    }
	    else
	    {
		samplenum = randomnation(TRIPERBLOCK);
	    }
	    sampletri.tri = (triangle *)
	        (firsttri + (samplenum * triangles.itemwords));
	    if (sampletri.tri[3] != NULL)
	    {
		org(sampletri, torg);
		dist = (searchpoint[0] - torg[0]) * (searchpoint[0] - torg[0])
		    + (searchpoint[1] - torg[1]) * (searchpoint[1] - torg[1]);
		if (dist < searchdist)
		{
		    triedgecopy(sampletri, *searchtri);
		    searchdist = dist;
		}
	    }
	}
	sampleblock = sampleblock->next;
    }
    /* Where are we? */
    org(*searchtri, torg);
    dest(*searchtri, tdest);
    /* Check the starting triangle's vertices. */
    if ((torg[0] == searchpoint[0]) && (torg[1] == searchpoint[1]))
    {
	return ONVERTEX;
    }
    if ((tdest[0] == searchpoint[0]) && (tdest[1] == searchpoint[1]))
    {
	lnextself(*searchtri);
	return ONVERTEX;
    }
    /* Orient `searchtri' to fit the preconditions of calling preciselocate(). */
    ahead = counterclockwise(torg, tdest, searchpoint);
    if (ahead < 0.0)
    {
	/* Turn around so that `searchpoint' is to the left of the */
	/*   edge specified by `searchtri'.                        */
	symself(*searchtri);
    }
    else if (ahead == 0.0)
    {
	/* Check if `searchpoint' is between `torg' and `tdest'. */
	if (((torg[0] < searchpoint[0]) == (searchpoint[0] < tdest[0]))
	    && ((torg[1] < searchpoint[1]) == (searchpoint[1] < tdest[1])))
        {
	    return ONEDGE;
	}
    }
    return preciselocate(searchpoint, searchtri);
}		/*end locate*/

/**                                                                         **/
/**                                                                         **/
/********* Point location routines end here                          *********/

/********* Mesh transformation routines begin here                   *********/
/**                                                                         **/
/**                                                                         **/

/*****************************************************************************/
/*                                                                           */
/*  insertshelle()   Create a new shell edge and insert it between two       */
/*                   triangles.                                              */
/*                                                                           */
/*  The new shell edge is inserted at the edge described by the handle       */
/*  `tri'.  Its vertices are properly initialized.  The marker `shellemark'  */
/*  is applied to the shell edge and, if appropriate, its vertices.          */
/*                                                                           */
/*****************************************************************************/

LOCAL void insertshelle(
    triedge *tri,
    int shellemark)
{
    triedge oppotri;
    edge newshelle;
    point triorg, tridest;
    triangle ptr;                     /* Temporary variable used by sym(). */
    shelle sptr;                      /* Temporary variable used by tspivot(). */

    /* Mark points if possible. */
    org(*tri, triorg);
    dest(*tri, tridest);
    if (pointmark(triorg) == 0)
    {
	setpointmark(triorg, shellemark);
    }
    if (pointmark(tridest) == 0)
    {
	setpointmark(tridest, shellemark);
    }
    /* Check if there's already a shell edge here. */
    tspivot(*tri, newshelle);
    if (newshelle.sh == dummysh)
    {
	/* Make new shell edge and initialize its vertices. */
	makeshelle(&newshelle);
	setsorg(newshelle, tridest);
	setsdest(newshelle, triorg);
	/* Bond new shell edge to the two triangles it is sandwiched between. */
	/*   Note that the facing triangle `oppotri' might be equal to        */
	/*   `dummytri' (outer space), but the new shell edge is bonded to it */
	/*   all the same.                                                    */
	tsbond(*tri, newshelle);
	sym(*tri, oppotri);
	ssymself(newshelle);
	tsbond(oppotri, newshelle);
	setmark(newshelle, shellemark);
    }
    else
    {
	if (mark(newshelle) == 0)
	{
	    setmark(newshelle, shellemark);
	}
    }
}		/*end insertshelle*/

/*****************************************************************************/
/*                                                                           */
/*  Terminology                                                              */
/*                                                                           */
/*  A "local transformation" replaces a small set of triangles with another  */
/*  set of triangles.  This may or may not involve inserting or deleting a   */
/*  point.                                                                   */
/*                                                                           */
/*  The term "casing" is used to describe the set of triangles that are      */
/*  attached to the triangles being transformed, but are not transformed     */
/*  themselves.  Think of the casing as a fixed hollow structure inside      */
/*  which all the action happens.  A "casing" is only defined relative to    */
/*  a single transformation; each occurrence of a transformation will        */
/*  involve a different casing.                                              */
/*                                                                           */
/*  A "shell" is similar to a "casing".  The term "shell" describes the set  */
/*  of shell edges (if any) that are attached to the triangles being         */
/*  transformed.  However, I sometimes use "shell" to refer to a single      */
/*  shell edge, so don't get confused.                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  flip()   Transform two triangles to two different triangles by flipping  */
/*           an edge within a quadrilateral.                                 */
/*                                                                           */
/*  Imagine the original triangles, abc and bad, oriented so that the        */
/*  shared edge ab lies in a horizontal plane, with the point b on the left  */
/*  and the point a on the right.  The point c lies below the edge, and the  */
/*  point d lies above the edge.  The `flipedge' handle holds the edge ab    */
/*  of triangle abc, and is directed left, from vertex a to vertex b.        */
/*                                                                           */
/*  The triangles abc and bad are deleted and replaced by the triangles cdb  */
/*  and dca.  The triangles that represent abc and bad are NOT deallocated;  */
/*  they are reused for dca and cdb, respectively.  Hence, any handles that  */
/*  may have held the original triangles are still valid, although not       */
/*  directed as they were before.                                            */
/*                                                                           */
/*  Upon completion of this routine, the `flipedge' handle holds the edge    */
/*  dc of triangle dca, and is directed down, from vertex d to vertex c.     */
/*  (Hence, the two triangles have rotated counterclockwise.)                */
/*                                                                           */
/*  WARNING:  This transformation is geometrically valid only if the         */
/*  quadrilateral adbc is convex.  Furthermore, this transformation is       */
/*  valid only if there is not a shell edge between the triangles abc and    */
/*  bad.  This routine does not check either of these preconditions, and     */
/*  it is the responsibility of the calling routine to ensure that they are  */
/*  met.  If they are not, the streets shall be filled with wailing and      */
/*  gnashing of teeth.                                                       */
/*                                      */
/*****************************************************************************/

LOCAL void flip(
    triedge *flipedge)
{
    triedge botleft, botright;
    triedge topleft, topright;
    triedge top;
    triedge botlcasing, botrcasing;
    triedge toplcasing, toprcasing;
    edge botlshelle, botrshelle;
    edge toplshelle, toprshelle;
    point leftpoint, rightpoint, botpoint;
    point farpoint;
    triangle ptr;                     /* Temporary variable used by sym(). */
    shelle sptr;                      /* Temporary variable used by tspivot(). */

    /* Identify the vertices of the quadrilateral. */
    org(*flipedge, rightpoint);
    dest(*flipedge, leftpoint);
    apex(*flipedge, botpoint);
    sym(*flipedge, top);
    apex(top, farpoint);

    /* Identify the casing of the quadrilateral. */
    lprev(top, topleft);
    sym(topleft, toplcasing);
    lnext(top, topright);
    sym(topright, toprcasing);
    lnext(*flipedge, botleft);
    sym(botleft, botlcasing);
    lprev(*flipedge, botright);
    sym(botright, botrcasing);
    /* Rotate the quadrilateral one-quarter turn counterclockwise. */
    bond(topleft, botlcasing);
    bond(botleft, botrcasing);
    bond(botright, toprcasing);
    bond(topright, toplcasing);

    if (checksegments)
    {
	/* Check for shell edges and rebond them to the quadrilateral. */
	tspivot(topleft, toplshelle);
	tspivot(botleft, botlshelle);
	tspivot(botright, botrshelle);
	tspivot(topright, toprshelle);
	if (toplshelle.sh == dummysh)
	{
	    tsdissolve(topright);
	}
	else
	{
	    tsbond(topright, toplshelle);
	}
	if (botlshelle.sh == dummysh)
	{
	    tsdissolve(topleft);
	}
	else
	{
	    tsbond(topleft, botlshelle);
	}
	if (botrshelle.sh == dummysh)
	{
	    tsdissolve(botleft);
	}
	else
	{
	    tsbond(botleft, botrshelle);
	}
	if (toprshelle.sh == dummysh)
	{
	    tsdissolve(botright);
	}
	else
	{
	    tsbond(botright, toprshelle);
	}
    }

    /* New point ft_assignments for the rotated quadrilateral. */
    setorg(*flipedge, farpoint);
    setdest(*flipedge, botpoint);
    setapex(*flipedge, rightpoint);
    setorg(top, botpoint);
    setdest(top, farpoint);
    setapex(top, leftpoint);
}		/*end flip*/

/*****************************************************************************/
/*                                                                           */
/*  insertsite()   Insert a vertex into a Delaunay triangulation,            */
/*                 performing flips as necessary to maintain the Delaunay    */
/*                 property.                                                 */
/*                                                                           */
/*  The point `insertpoint' is located.  If `searchtri.tri' is not NULL,     */
/*  the search for the containing triangle begins from `searchtri'.  If      */
/*  `searchtri.tri' is NULL, a full point location procedure is called.      */
/*  If `insertpoint' is found inside a triangle, the triangle is split into  */
/*  three; if `insertpoint' lies on an edge, the edge is split in two,       */
/*  thereby splitting the two adjacent triangles into four.  Edge flips are  */
/*  used to restore the Delaunay property.  If `insertpoint' lies on an      */
/*  existing vertex, no action is taken, and the value DUPLICATEPOINT is     */
/*  returned.  On return, `searchtri' is set to a handle whose origin is the */
/*  existing vertex.                                                         */
/*                                                                           */
/*  Normally, the parameter `splitedge' is set to NULL, implying that no     */
/*  segment should be split.  In this case, if `insertpoint' is found to     */
/*  lie on a segment, no action is taken, and the value VIOLATINGPOINT is    */
/*  returned.  On return, `searchtri' is set to a handle whose primary edge  */
/*  is the violated segment.                                                 */
/*                                                                           */
/*  If the calling routine wishes to split a segment by inserting a point in */
/*  it, the parameter `splitedge' should be that segment.  In this case,     */
/*  `searchtri' MUST be the triangle handle reached by pivoting from that    */
/*  segment; no point location is done.                                      */
/*                                                                           */
/*  `segmentflaws' and `triflaws' are flags that indicate whether or not     */
/*  there should be checks for the creation of encroached segments or bad    */
/*  quality faces.  If a newly inserted point encroaches upon segments,      */
/*  these segments are added to the list of segments to be split if          */
/*  `segmentflaws' is set.  If bad triangles are created, these are added    */
/*  to the queue if `triflaws' is set.                                       */
/*                                                                           */
/*  If a duplicate point or violated segment does not prevent the point      */
/*  from being inserted, the return value will be ENCROACHINGPOINT if the    */
/*  point encroaches upon a segment (and checking is enabled), or            */
/*  SUCCESSFULPOINT otherwise.  In either case, `searchtri' is set to a      */
/*  handle whose origin is the newly inserted vertex.                        */
/*                                                                           */
/*  insertsite() does not use flip() for reasons of speed; some              */
/*  information can be reused from edge flip to edge flip, like the          */
/*  locations of shell edges.                                                */
/*                                                                           */
/*****************************************************************************/

/*ARGSUSED*/
LOCAL insertsiteresult insertsite(
    point insertpoint,
    triedge *searchtri,
    edge *splitedge,
    int segmentflaws,
    int triflaws)
{
    triedge horiz;
    triedge top;
    triedge botleft, botright;
    triedge topleft, topright;
    triedge newbotleft, newbotright;
    triedge newtopright;
    triedge botlcasing, botrcasing;
    triedge toplcasing, toprcasing;
    triedge testtri;
    edge botlshelle, botrshelle;
    edge toplshelle, toprshelle;
    edge brokenshelle;
    edge checkshelle;
    edge rightedge;
    edge newedge;
    edge *encroached;
    point first;
    point leftpoint, rightpoint, botpoint, toppoint, farpoint;
    double attrib;
    double area;
    insertsiteresult success;
    locateresult intersect;
    int doflip;
    int mirrorflag;
    int i;
    triangle ptr;                        /* Temporary variable used by sym(). */
    shelle sptr;        /* Temporary variable used by spivot() and tspivot(). */

    if (splitedge == NULL)
    {
	/* Find the location of the point to be inserted.  Check if a good */
	/*   starting triangle has already been provided by the caller.    */
	if (searchtri->tri == NULL)
	{
	    /* Find a boundary triangle. */
	    horiz.tri = dummytri;
	    horiz.orient = 0;
	    symself(horiz);
	    /* Search for a triangle containing `insertpoint'. */
	    intersect = locate(insertpoint, &horiz);
	}
	else
	{
	    /* Start searching from the triangle provided by the caller. */
	    triedgecopy(*searchtri, horiz);
	    intersect = preciselocate(insertpoint, &horiz);
	}
    }
    else
    {
	/* The calling routine provides the edge in which the point is inserted. */
	triedgecopy(*searchtri, horiz);
	intersect = ONEDGE;
    }
    if (intersect == ONVERTEX)
    {
	/* There's already a vertex there.  Return in `searchtri' a triangle */
	/*   whose origin is the existing vertex.                            */
	triedgecopy(horiz, *searchtri);
	triedgecopy(horiz, recenttri);
	return DUPLICATEPOINT;
    }
    if ((intersect == ONEDGE) || (intersect == OUTSIDE))
    {
	/* The vertex falls on an edge or boundary. */
	if (checksegments && (splitedge == NULL))
	{
	    /* Check whether the vertex falls on a shell edge. */
	    tspivot(horiz, brokenshelle);
	    if (brokenshelle.sh != dummysh)
	    {
		/* The vertex falls on a shell edge. */
		if (segmentflaws)
		{
		    if (!TriOpts.nobisect)
		    {
			/* Add the shell edge to the list of encroached segments. */
			encroached = (edge *) poolalloc(&badsegments);
			shellecopy(brokenshelle, *encroached);
		    }
		    else if (TriOpts.nobisect && (intersect == ONEDGE))
		    {
			/* This segment may be split only if it is an internal boundary. */
			sym(horiz, testtri);
			if (testtri.tri != dummytri)
			{
			    /* Add the shell edge to the list of encroached segments. */
			    encroached = (edge *) poolalloc(&badsegments);
			    shellecopy(brokenshelle, *encroached);
			}
		    }
		}
		/* Return a handle whose primary edge contains the point, */
		/*   which has not been inserted.                         */
		triedgecopy(horiz, *searchtri);
		triedgecopy(horiz, recenttri);
		return VIOLATINGPOINT;
	    }
	}
	/* Insert the point on an edge, dividing one triangle into two (if */
	/*   the edge lies on a boundary) or two triangles into four.      */
	lprev(horiz, botright);
	sym(botright, botrcasing);
	sym(horiz, topright);
	/* Is there a second triangle?  (Or does this edge lie on a boundary?) */
	mirrorflag = topright.tri != dummytri;
	if (mirrorflag)
	{
	    lnextself(topright);
	    sym(topright, toprcasing);
	    maketriangle(&newtopright);
	}
	else
	{
	    /* Splitting the boundary edge increases the number of boundary edges. */
	    hullsize++;
	}
	maketriangle(&newbotright);

	/* Set the vertices of changed and new triangles. */
	org(horiz, rightpoint);
	dest(horiz, leftpoint);
	apex(horiz, botpoint);
	setorg(newbotright, botpoint);
	setdest(newbotright, rightpoint);
	setapex(newbotright, insertpoint);
	setorg(horiz, insertpoint);
	for (i = 0; i < eextras; i++)
	{
	    /* Set the element attributes of a new triangle. */
	    setelemattribute(newbotright, i, elemattribute(botright, i));
	}
	if (TriOpts.vararea)
	{
	    /* Set the area constraint of a new triangle. */
	    setareabound(newbotright, areabound(botright));
	}
	if (mirrorflag)
	{
	    dest(topright, toppoint);
	    setorg(newtopright, rightpoint);
	    setdest(newtopright, toppoint);
	    setapex(newtopright, insertpoint);
	    setorg(topright, insertpoint);
	    for (i = 0; i < eextras; i++)
	    {
		/* Set the element attributes of another new triangle. */
		setelemattribute(newtopright, i, elemattribute(topright, i));
	    }
	    if (TriOpts.vararea)
	    {
		/* Set the area constraint of another new triangle. */
		setareabound(newtopright, areabound(topright));
	    }
	}

	/* There may be shell edges that need to be bonded */
	/*   to the new triangle(s).                       */
	if (checksegments)
	{
	    tspivot(botright, botrshelle);
	    if (botrshelle.sh != dummysh)
	    {
		tsdissolve(botright);
		tsbond(newbotright, botrshelle);
	    }
	    if (mirrorflag)
	    {
		tspivot(topright, toprshelle);
		if (toprshelle.sh != dummysh)
		{
		    tsdissolve(topright);
		    tsbond(newtopright, toprshelle);
		}
	    }
	}

	/* Bond the new triangle(s) to the surrounding triangles. */
	bond(newbotright, botrcasing);
	lprevself(newbotright);
	bond(newbotright, botright);
	lprevself(newbotright);
	if (mirrorflag)
	{
	    bond(newtopright, toprcasing);
	    lnextself(newtopright);
	    bond(newtopright, topright);
	    lnextself(newtopright);
	    bond(newtopright, newbotright);
	}

	if (splitedge != NULL)
	{
	    /* Split the shell edge into two. */
	    setsdest(*splitedge, insertpoint);
	    ssymself(*splitedge);
	    spivot(*splitedge, rightedge);
	    insertshelle(&newbotright, mark(*splitedge));
	    tspivot(newbotright, newedge);
	    sbond(*splitedge, newedge);
	    ssymself(newedge);
	    sbond(newedge, rightedge);
	    ssymself(*splitedge);
	}

	/* Position `horiz' on the first edge to check for */
	/*   the Delaunay property.                        */
	lnextself(horiz);
    }
    else
    {
	/* Insert the point in a triangle, splitting it into three. */
	lnext(horiz, botleft);
	lprev(horiz, botright);
	sym(botleft, botlcasing);
	sym(botright, botrcasing);
	maketriangle(&newbotleft);
	maketriangle(&newbotright);

	/* Set the vertices of changed and new triangles. */
	org(horiz, rightpoint);
	dest(horiz, leftpoint);
	apex(horiz, botpoint);
	setorg(newbotleft, leftpoint);
	setdest(newbotleft, botpoint);
	setapex(newbotleft, insertpoint);
	setorg(newbotright, botpoint);
	setdest(newbotright, rightpoint);
	setapex(newbotright, insertpoint);
	setapex(horiz, insertpoint);
	for (i = 0; i < eextras; i++)
	{
	    /* Set the element attributes of the new triangles. */
	    attrib = elemattribute(horiz, i);
	    setelemattribute(newbotleft, i, attrib);
	    setelemattribute(newbotright, i, attrib);
	}
	if (TriOpts.vararea)
	{
	    /* Set the area constraint of the new triangles. */
	    area = areabound(horiz);
	    setareabound(newbotleft, area);
	    setareabound(newbotright, area);
	}

	/* There may be shell edges that need to be bonded */
	/*   to the new triangles.                         */
	if (checksegments)
	{
	    tspivot(botleft, botlshelle);
	    if (botlshelle.sh != dummysh)
	    {
		tsdissolve(botleft);
		tsbond(newbotleft, botlshelle);
	    }
	    tspivot(botright, botrshelle);
	    if (botrshelle.sh != dummysh)
	    {
		tsdissolve(botright);
		tsbond(newbotright, botrshelle);
	    }
	}

	/* Bond the new triangles to the surrounding triangles. */
	bond(newbotleft, botlcasing);
	bond(newbotright, botrcasing);
	lnextself(newbotleft);
	lprevself(newbotright);
	bond(newbotleft, newbotright);
	lnextself(newbotleft);
	bond(botleft, newbotleft);
	lprevself(newbotright);
	bond(botright, newbotright);
    }

    /* The insertion is successful by default, unless an encroached */
    /*   edge is found.                                             */
    success = SUCCESSFULPOINT;
    /* Circle around the newly inserted vertex, checking each edge opposite */
    /*   it for the Delaunay property.  Non-Delaunay edges are flipped.     */
    /*   `horiz' is always the edge being checked.  `first' marks where to  */
    /*   stop circling.                                                     */
    org(horiz, first);
    rightpoint = first;
    dest(horiz, leftpoint);
    /* Circle until finished. */
    while (1)
    {
	/* By default, the edge will be flipped. */
	doflip = 1;
	if (checksegments)
	{
	    /* Check for a segment, which cannot be flipped. */
	    tspivot(horiz, checkshelle);
	    if (checkshelle.sh != dummysh)
	    {
		/* The edge is a segment and cannot be flipped. */
		doflip = 0;
	    }
	}
	if (doflip)
	{
	    /* Check if the edge is a boundary edge. */
	    sym(horiz, top);
	    if (top.tri == dummytri)
	    {
		/* The edge is a boundary edge and cannot be flipped. */
		doflip = 0;
	    }
	    else
	    {
		/* Find the point on the other side of the edge. */
		apex(top, farpoint);
		/* In the incremental Delaunay triangulation algorithm,    */
		/*  any of `leftpoint', `rightpoint', and `farpoint'       */
		/*  could be vertices of the triangular bounding box.      */
		/*  These vertices must be treated as if they are          */
		/*  infinitely distant, even though their "coordinates"    */
		/*  are not.                                               */
		if ((leftpoint == infpoint1) || (leftpoint == infpoint2)
		    || (leftpoint == infpoint3))
		{
		    /* `leftpoint' is infinitely distant.  Check the       */
		    /*  convexity of the boundary of the triangulation.    */
		    /*  'farpoint' might be infinite as well, but trust me,*/
		    /*  this same condition should be applied.             */
		    doflip =
			counterclockwise(insertpoint,rightpoint,farpoint) > 0.0;
		}
		else if ((rightpoint == infpoint1) || (rightpoint == infpoint2)
		    || (rightpoint == infpoint3))
		{
		    /* `rightpoint' is infinitely distant.  Check the      */
		    /*  convexity of the boundary of the triangulation.    */
		    /*  'farpoint' might be infinite as well, but trust me,*/
		    /*  this same condition should be applied.             */
		    doflip =
			counterclockwise(farpoint,leftpoint,insertpoint) > 0.0;
		}
		else if ((farpoint == infpoint1) || (farpoint == infpoint2)
		    || (farpoint == infpoint3))
		{
		    /* `farpoint' is infinitely distant and cannot be inside */
		    /*   the circumcircle of the triangle `horiz'.           */
		    doflip = 0;
		}
		else
		{
		    /* Test whether the edge is locally Delaunay. */
		    doflip =
			incircle(leftpoint,insertpoint,rightpoint,farpoint)>0.0;
		}
		if (doflip)
		{
		    /* We made it!  Flip the edge `horiz' by rotating its    */
		    /*  containing quadrilateral (the two triangles adjacent */
		    /*  to `horiz'). Identify the casing of the              */
		    /*  quadrilateral. */
		    lprev(top, topleft);
		    sym(topleft, toplcasing);
		    lnext(top, topright);
		    sym(topright, toprcasing);
		    lnext(horiz, botleft);
		    sym(botleft, botlcasing);
		    lprev(horiz, botright);
		    sym(botright, botrcasing);
		    /* Rotate the quadrilateral one-quarter turn */
		    /*  counterclockwise. */
		    bond(topleft, botlcasing);
		    bond(botleft, botrcasing);
		    bond(botright, toprcasing);
		    bond(topright, toplcasing);
		    if (checksegments)
		    {
			/* Check for shell edges and rebond them to */
			/*  the quadrilateral. */
			tspivot(topleft, toplshelle);
			tspivot(botleft, botlshelle);
			tspivot(botright, botrshelle);
			tspivot(topright, toprshelle);
			if (toplshelle.sh == dummysh)
			{
			    tsdissolve(topright);
			}
			else
			{
			    tsbond(topright, toplshelle);
			}
			if (botlshelle.sh == dummysh)
			{
			    tsdissolve(topleft);
			}
			else
			{
			    tsbond(topleft, botlshelle);
			}
			if (botrshelle.sh == dummysh)
			{
			    tsdissolve(botleft);
			}
			else
			{
			    tsbond(botleft, botrshelle);
			}
			if (toprshelle.sh == dummysh)
			{
			    tsdissolve(botright);
			}
			else
			{
			    tsbond(botright, toprshelle);
			}
		    }
		    /* New point ft_assignments for the rotated quadrilateral. */
		    setorg(horiz, farpoint);
		    setdest(horiz, insertpoint);
		    setapex(horiz, rightpoint);
		    setorg(top, insertpoint);
		    setdest(top, farpoint);
		    setapex(top, leftpoint);
		    for (i = 0; i < eextras; i++)
		    {
			/* Take the average of the two triangles' attributes. */
			attrib =
			    0.5*(elemattribute(top,i)+elemattribute(horiz,i));
			setelemattribute(top, i, attrib);
			setelemattribute(horiz, i, attrib);
		    }
		    if (TriOpts.vararea)
		    {
			if ((areabound(top)<=0.0) || (areabound(horiz)<=0.0))
			{
			    area = -1.0;
			}
			else
			{
			    /* Take the average of the two triangles' area */
			    /*   constraints. This prevents small area     */
			    /*   constraints from migrating a long way  */
			    /*   from their original location due to flips. */
			    area = 0.5 * (areabound(top) + areabound(horiz));
			}
			setareabound(top, area);
			setareabound(horiz, area);
		    }
		    /* On the next iterations, consider the two edges that */
		    /*   were exposed (this is, are now visible to the     */
		    /*   newly inserted point) by the edge flip.           */
		    lprevself(horiz);
		    leftpoint = farpoint;
		}
	    }
	}
	if (!doflip)
	{
	    /* The handle `horiz' is accepted as locally Delaunay. */
	    /* Look for the next edge around the newly inserted point. */
	    lnextself(horiz);
	    sym(horiz, testtri);
	    /* Check for finishing a complete revolution about the new */
	    /*   point, or falling off the edge of the triangulation.  */
	    /*   The latter will happen when a point is inserted at a  */
	    /*   boundary.                                             */
	    if ((leftpoint == first) || (testtri.tri == dummytri))
	    {
		/* We're done.  Return a triangle whose origin */
		/*  is the new point. */
		lnext(horiz, *searchtri);
		lnext(horiz, recenttri);
		return success;
	    }
	    /* Finish finding the next edge around the newly inserted point. */
	    lnext(testtri, horiz);
	    rightpoint = leftpoint;
	    dest(horiz, leftpoint);
	}
    }
}		/*end insertsite*/

/*****************************************************************************/
/*                                                                           */
/*  triangulatepolygon()   Find the Delaunay triangulation of a polygon that */
/*                         has a certain "nice" shape.  This includes the    */
/*                         polygons that result from deletion of a point or  */
/*                         insertion of a segment.                           */
/*                                                                           */
/*  This is a conceptually difficult routine.  The starting assumption is    */
/*  that we have a polygon with n sides.  n - 1 of these sides are currently */
/*  represented as edges in the mesh.  One side, called the "base", need not */
/*  be.                                                                      */
/*                                                                           */
/*  Inside the polygon is a structure I call a "fan", consisting of n - 1    */
/*  triangles that share a common origin.  For each of these triangles, the  */
/*  edge opposite the origin is one of the sides of the polygon.  The        */
/*  primary edge of each triangle is the edge directed from the origin to    */
/*  the destination; note that this is not the same edge that is a side of   */
/*  the polygon.  `firstedge' is the primary edge of the first triangle.     */
/*  From there, the triangles follow in counterclockwise order about the     */
/*  polygon, until `lastedge', the primary edge of the last triangle.        */
/*  `firstedge' and `lastedge' are probably connected to other triangles     */
/*  beyond the extremes of the fan, but their identity is not important, as  */
/*  long as the fan remains connected to them.                               */
/*                                                                           */
/*  Imagine the polygon oriented so that its base is at the bottom.  This    */
/*  puts `firstedge' on the far right, and `lastedge' on the far left.       */
/*  The right vertex of the base is the destination of `firstedge', and the  */
/*  left vertex of the base is the apex of `lastedge'.                       */
/*                                                                           */
/*  The challenge now is to find the right sequence of edge flips to         */
/*  transform the fan into a Delaunay triangulation of the polygon.  Each    */
/*  edge flip effectively removes one triangle from the fan, committing it   */
/*  to the polygon.  The resulting polygon has one fewer edge.  If `doflip'  */
/*  is set, the final flip will be performed, resulting in a fan of one      */
/*  (useless?) triangle.  If `doflip' is not set, the final flip is not      */
/*  performed, resulting in a fan of two triangles, and an unfinished        */
/*  triangular polygon that is not yet filled out with a single triangle.    */
/*  On completion of the routine, `lastedge' is the last remaining triangle, */
/*  or the leftmost of the last two.                                         */
/*                                                                           */
/*  Although the flips are performed in the order described above, the       */
/*  decisions about what flips to perform are made in precisely the reverse  */
/*  order.  The recursive triangulatepolygon() procedure makes a decision,   */
/*  uses up to two recursive calls to triangulate the "subproblems"          */
/*  (polygons with fewer edges), and then performs an edge flip.             */
/*                                                                           */
/*  The "decision" it makes is which vertex of the polygon should be         */
/*  connected to the base.  This decision is made by testing every possible  */
/*  vertex.  Once the best vertex is found, the two edges that connect this  */
/*  vertex to the base become the bases for two smaller polygons.  These     */
/*  are triangulated recursively.  Unfortunately, this approach can take     */
/*  O(n^2) time not only in the worst case, but in many common cases.  It's  */
/*  rarely a big deal for point deletion, where n is rarely larger than ten, */
/*  but it could be a big deal for segment insertion, especially if there's  */
/*  a lot of long segments that each cut many triangles.  I ought to code    */
/*  a faster algorithm some time.                                            */
/*                                                                           */
/*  The `edgecount' parameter is the number of sides of the polygon,         */
/*  including its base.  `triflaws' is a flag that determines whether the    */
/*  new triangles should be tested for quality, and enqueued if they are     */
/*  bad.                                                                     */
/*                                                                           */
/*****************************************************************************/

LOCAL void triangulatepolygon(
    triedge *firstedge,
    triedge *lastedge,
    int edgecount,
    int doflip,
    int triflaws)
{
    triedge testtri;
    triedge besttri;
    triedge tempedge;
    point leftbasepoint, rightbasepoint;
    point testpoint;
    point bestpoint;
    int bestnumber;
    int i;
    triangle ptr;  /* Temporary variable used by sym(), onext(), and oprev(). */

    /* Identify the base vertices. */
    apex(*lastedge, leftbasepoint);
    dest(*firstedge, rightbasepoint);
    /* Find the best vertex to connect the base to. */
    onext(*firstedge, besttri);
    dest(besttri, bestpoint);
    triedgecopy(besttri, testtri);
    bestnumber = 1;
    for (i = 2; i <= edgecount - 2; i++)
    {
	onextself(testtri);
	dest(testtri, testpoint);
	/* Is this a better vertex? */
	if (incircle(leftbasepoint, rightbasepoint, bestpoint, testpoint) > 0.0)
	{
	    triedgecopy(testtri, besttri);
	    bestpoint = testpoint;
	    bestnumber = i;
	}
    }
    if (bestnumber > 1)
    {
	/* Recursively triangulate the smaller polygon on the right. */
	oprev(besttri, tempedge);
	triangulatepolygon(firstedge, &tempedge, bestnumber + 1, 1, triflaws);
    }
    if (bestnumber < edgecount - 2)
    {
	/* Recursively triangulate the smaller polygon on the left. */
	sym(besttri, tempedge);
	triangulatepolygon(&besttri, lastedge, edgecount - bestnumber, 1,
	    triflaws);
	/* Find `besttri' again; it may have been lost to edge flips. */
	sym(tempedge, besttri);
    }
    if (doflip)
    {
	/* Do one final edge flip. */
	flip(&besttri);
    }
    /* Return the base triangle. */
    triedgecopy(besttri, *lastedge);
}		/*end triangulatepolygon*/

/*****************************************************************************/
/*                                                                           */
/*  deletesite()   Delete a vertex from a Delaunay triangulation, ensuring   */
/*                 that the triangulation remains Delaunay.                  */
/*                                                                           */
/*  The origin of `deltri' is deleted.  The union of the triangles adjacent  */
/*  to this point is a polygon, for which the Delaunay triangulation is      */
/*  found.  Two triangles are removed from the mesh.                         */
/*                                                                           */
/*  Only interior points that do not lie on segments (shell edges) or        */
/*  boundaries may be deleted.                                               */
/*                                                                           */
/*****************************************************************************/


/********* Divide-and-conquer Delaunay triangulation begins here     *********/
/**                                                                         **/
/**                                                                         **/

/*****************************************************************************/
/*                                                                           */
/*  The divide-and-conquer bounding box                                      */
/*                                                                           */
/*  I originally implemented the divide-and-conquer and incremental Delaunay */
/*  triangulations using the edge-based data structure presented by Guibas   */
/*  and Stolfi.  Switching to a triangle-based data structure doubled the    */
/*  speed.  However, I had to think of a few extra tricks to maintain the    */
/*  elegance of the original algorithms.                                     */
/*                                                                           */
/*  The "bounding box" used by my variant of the divide-and-conquer          */
/*  algorithm uses one triangle for each edge of the convex hull of the      */
/*  triangulation.  These bounding triangles all share a common apical       */
/*  vertex, which is represented by NULL and which represents nothing.       */
/*  The bounding triangles are linked in a circular fan about this NULL      */
/*  vertex, and the edges on the convex hull of the triangulation appear     */
/*  opposite the NULL vertex.  You might find it easiest to imagine that     */
/*  the NULL vertex is a point in 3D space behind the center of the          */
/*  triangulation, and that the bounding triangles form a sort of cone.      */
/*                                                                           */
/*  This bounding box makes it easy to represent degenerate cases.  For      */
/*  instance, the triangulation of two vertices is a single edge.  This edge */
/*  is represented by two bounding box triangles, one on each "side" of the  */
/*  edge.  These triangles are also linked together in a fan about the NULL  */
/*  vertex.                                                                  */
/*                                                                           */
/*  The bounding box also makes it easy to traverse the convex hull, as the  */
/*  divide-and-conquer algorithm needs to do.                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  pointsort()   Sort an array of points by x-coordinate, using the         */
/*                y-coordinate as a secondary k*/
/**/
/**/
/**/
/**/
/*****************************************************************************/

LOCAL void pointsort(
    point *sortarray,
    size_t arraysize)
{
    long left, right;
    size_t pivot;
    double pivotx, pivoty;
    point temp;

    if (arraysize == 2)
    {
	/* Recursive base case. */
	if ((sortarray[0][0] > sortarray[1][0]) ||
	    ((sortarray[0][0] == sortarray[1][0]) &&
	    (sortarray[0][1] > sortarray[1][1])))
	{
	    temp = sortarray[1];
	    sortarray[1] = sortarray[0];
	    sortarray[0] = temp;
	}
	return;
    }
    /* Choose a random pivot to split the array. */
    pivot = randomnation(arraysize);
    pivotx = sortarray[pivot][0];
    pivoty = sortarray[pivot][1];
    /* Split the array. */
    left = -1;
    right = (long) arraysize;
    while (left < right)
    {
	/* Search for a point whose x-coordinate is too large for the left. */
	do
	{
	    left++;
	} while ((left <= right) && ((sortarray[left][0] < pivotx) ||
	    ((sortarray[left][0] == pivotx) &&
	    (sortarray[left][1] < pivoty))));
	/* Search for a point whose x-coordinate is too small for the right. */
	do
	{
	    right--;
	} while ((left <= right) && ((sortarray[right][0] > pivotx) ||
	    ((sortarray[right][0] == pivotx) &&
	    (sortarray[right][1] > pivoty))));
	if (left < right)
	{
	    /* Swap the left and right points. */
	    temp = sortarray[left];
	    sortarray[left] = sortarray[right];
	    sortarray[right] = temp;
	}
    }
    if (left > 1)
    {
	/* Recursively sort the left subset. */
	pointsort(sortarray, left);
    }
    if (right < arraysize - 2)
    {
	/* Recursively sort the right subset. */
	pointsort(&sortarray[right + 1], arraysize - right - 1);
    }
}		/*end pointsort*/

/*****************************************************************************/
/*                                                                           */
/*  pointmedian()   An order statistic algorithm, almost.  Shuffles an array */
/*                  of points so that the first `median' points occur        */
/*                  lexicographically before the remaining points.           */
/*                                                                           */
/*  Uses the x-coordinate as the primary key if axis == 0; the y-coordinate  */
/*  if axis == 1.  Very similar to the pointsort() procedure, but runs in    */
/*  randomized linear time.                                                  */
/*                                                                           */
/*****************************************************************************/

LOCAL void pointmedian(
    point *sortarray,
    int arraysize,
    int median,
    int axis)
{
    int left, right;
    int pivot;
    double pivot1, pivot2;
    point temp;

    if (arraysize == 2)
    {
	/* Recursive base case. */
	if ((sortarray[0][axis] > sortarray[1][axis]) ||
	    ((sortarray[0][axis] == sortarray[1][axis]) &&
	    (sortarray[0][1 - axis] > sortarray[1][1 - axis])))
	{
	    temp = sortarray[1];
	    sortarray[1] = sortarray[0];
	    sortarray[0] = temp;
	}
	return;
    }
    /* Choose a random pivot to split the array. */
    pivot = (int) randomnation(arraysize);
    pivot1 = sortarray[pivot][axis];
    pivot2 = sortarray[pivot][1 - axis];
    /* Split the array. */
    left = -1;
    right = arraysize;
    while (left < right)
    {
	/* Search for a point whose x-coordinate is too large for the left. */
	do
	{
	    left++;
	} while ((left <= right) && ((sortarray[left][axis] < pivot1) ||
	    ((sortarray[left][axis] == pivot1) &&
	    (sortarray[left][1 - axis] < pivot2))));
	/* Search for a point whose x-coordinate is too small for the right. */
	do
	{
	    right--;
	} while ((left <= right) && ((sortarray[right][axis] > pivot1) ||
	    ((sortarray[right][axis] == pivot1) &&
	    (sortarray[right][1 - axis] > pivot2))));
	if (left < right)
	{
	    /* Swap the left and right points. */
	    temp = sortarray[left];
	    sortarray[left] = sortarray[right];
	    sortarray[right] = temp;
	}
    }
    /* Unlike in pointsort(), at most one of the following */
    /*   conditionals is TRUE.                             */
    if (left > median)
    {
	/* Recursively shuffle the left subset. */
	pointmedian(sortarray, left, median, axis);
    }
    if (right < median - 1)
    {
	/* Recursively shuffle the right subset. */
	pointmedian(&sortarray[right + 1], arraysize - right - 1,
	    median - right - 1, axis);
    }
}		/*end pointmedian*/

/*****************************************************************************/
/*                                                                           */
/*  alternateaxes()   Sorts the points as appropriate for the divide-and-    */
/*                    conquer algorithm with alternating cuts.               */
/*                                                                           */
/*  Partitions by x-coordinate if axis == 0; by y-coordinate if axis == 1.   */
/*  For the base case, subsets containing only two or three points are       */
/*  always sorted by x-coordinate.                                           */
/*                                                                           */
/*****************************************************************************/

LOCAL void alternateaxes(
    point *sortarray,
    int arraysize,
    int axis)
{
    int divider;

    divider = arraysize >> 1;
    if (arraysize <= 3)
    {
	/* Recursive base case:  subsets of two or three points will be      */
	/*   handled specially, and should always be sorted by x-coordinate. */
	axis = 0;
    }
    /* Partition with a horizontal or vertical cut. */
    pointmedian(sortarray, arraysize, divider, axis);
    /* Recursively partition the subsets with a cross cut. */
    if (arraysize - divider >= 2)
    {
	if (divider >= 2)
	{
	    alternateaxes(sortarray, divider, 1 - axis);
	}
	alternateaxes(&sortarray[divider], arraysize - divider, 1 - axis);
    }
}		/*end alternateaxes*/

/*****************************************************************************/
/*                                                                           */
/*  mergehulls()   Merge two adjacent Delaunay triangulations into a         */
/*                 single Delaunay triangulation.                            */
/*                                                                           */
/*  This is similar to the algorithm given by Guibas and Stolfi, but uses    */
/*  a triangle-based, rather than edge-based, data structure.                */
/*                                                                           */
/*  The algorithm walks up the gap between the two triangulations, knitting  */
/*  them together.  As they are merged, some of their bounding triangles     */
/*  are converted into real triangles of the triangulation.  The procedure   */
/*  pulls each hull's bounding triangles apart, then knits them together     */
/*  like the teeth of two gears.  The Delaunay property determines, at each  */
/*  step, whether the next "tooth" is a bounding triangle of the left hull   */
/*  or the right.  When a bounding triangle becomes real, its apex is        */
/*  changed from NULL to a real point.                                       */
/*                                                                           */
/*  Only two new triangles need to be allocated.  These become new bounding  */
/*  triangles at the top and bottom of the seam.  They are used to connect   */
/*  the remaining bounding triangles (those that have not been converted     */
/*  into real triangles) into a single fan.                                  */
/*                                                                           */
/*  On entry, `farleft' and `innerleft' are bounding triangles of the left   */
/*  triangulation.  The origin of `farleft' is the leftmost vertex, and      */
/*  the destination of `innerleft' is the rightmost vertex of the            */
/*  triangulation.  Similarly, `innerright' and `farright' are bounding      */
/*  triangles of the right triangulation.  The origin of `innerright' and    */
/*  destination of `farright' are the leftmost and rightmost vertices.       */
/*                                                                           */
/*  On completion, the origin of `farleft' is the leftmost vertex of the     */
/*  merged triangulation, and the destination of `farright' is the rightmost */
/*  vertex.                                                                  */
/*                                                                           */
/*****************************************************************************/

LOCAL void mergehulls(
    triedge *farleft,
    triedge *innerleft,
    triedge *innerright,
    triedge *farright,
    int axis)
{
    triedge leftcand, rightcand;
    triedge baseedge;
    triedge nextedge;
    triedge sidecasing, topcasing, outercasing;
    triedge checkedge;
    point innerleftdest;
    point innerrightorg;
    point innerleftapex, innerrightapex;
    point farleftpt, farrightpt;
    point farleftapex, farrightapex;
    point lowerleft, lowerright;
    point upperleft, upperright;
    point nextapex;
    point checkvertex;
    int changemade;
    int badedge;
    int leftfinished, rightfinished;
    triangle ptr;                         /* Temporary variable used by sym(). */

    dest(*innerleft, innerleftdest);
    apex(*innerleft, innerleftapex);
    org(*innerright, innerrightorg);
    apex(*innerright, innerrightapex);
    /* Special treatment for horizontal cuts. */
    if (dwyer && (axis == 1))
    {
	org(*farleft, farleftpt);
	apex(*farleft, farleftapex);
	dest(*farright, farrightpt);
	apex(*farright, farrightapex);
	/* The pointers to the extremal points are shifted to point to the */
	/*   topmost and bottommost point of each hull, rather than the    */
	/*   leftmost and rightmost points.                                */
	while (farleftapex[1] < farleftpt[1])
	{
	    lnextself(*farleft);
	    symself(*farleft);
	    farleftpt = farleftapex;
	    apex(*farleft, farleftapex);
	}
	sym(*innerleft, checkedge);
	apex(checkedge, checkvertex);
	while (checkvertex[1] > innerleftdest[1])
	{
	    lnext(checkedge, *innerleft);
	    innerleftapex = innerleftdest;
	    innerleftdest = checkvertex;
	    sym(*innerleft, checkedge);
	    apex(checkedge, checkvertex);
	}
	while (innerrightapex[1] < innerrightorg[1])
	{
	    lnextself(*innerright);
	    symself(*innerright);
	    innerrightorg = innerrightapex;
	    apex(*innerright, innerrightapex);
	}
	sym(*farright, checkedge);
	apex(checkedge, checkvertex);
	while (checkvertex[1] > farrightpt[1])
	{
	    lnext(checkedge, *farright);
	    farrightapex = farrightpt;
	    farrightpt = checkvertex;
	    sym(*farright, checkedge);
	    apex(checkedge, checkvertex);
	}
    }
    /* Find a line tangent to and below both hulls. */
    do
    {
	changemade = 0;
	/* Make innerleftdest the "bottommost" point of the left hull. */
	if (counterclockwise(innerleftdest, innerleftapex, innerrightorg) > 0.0)
	{
	    lprevself(*innerleft);
	    symself(*innerleft);
	    innerleftdest = innerleftapex;
	    apex(*innerleft, innerleftapex);
	    changemade = 1;
	}
	/* Make innerrightorg the "bottommost" point of the right hull. */
	if (counterclockwise(innerrightapex, innerrightorg, innerleftdest) > 0.0)
	{
	    lnextself(*innerright);
	    symself(*innerright);
	    innerrightorg = innerrightapex;
	    apex(*innerright, innerrightapex);
	    changemade = 1;
	}
    } while (changemade);
    /* Find the two candidates to be the next "gear tooth". */
    sym(*innerleft, leftcand);
    sym(*innerright, rightcand);
    /* Create the bottom new bounding triangle. */
    maketriangle(&baseedge);
    /* Connect it to the bounding boxes of the left and right triangulations. */
    bond(baseedge, *innerleft);
    lnextself(baseedge);
    bond(baseedge, *innerright);
    lnextself(baseedge);
    setorg(baseedge, innerrightorg);
    setdest(baseedge, innerleftdest);
    /* Apex is intentionally left NULL. */
    /* Fix the extreme triangles if necessary. */
    org(*farleft, farleftpt);
    if (innerleftdest == farleftpt)
    {
	lnext(baseedge, *farleft);
    }
    dest(*farright, farrightpt);
    if (innerrightorg == farrightpt)
    {
	lprev(baseedge, *farright);
    }
    /* The vertices of the current knitting edge. */
    lowerleft = innerleftdest;
    lowerright = innerrightorg;
    /* The candidate vertices for knitting. */
    apex(leftcand, upperleft);
    apex(rightcand, upperright);
    /* Walk up the gap between the two triangulations, knitting them together. */
    while (1)
    {
	/* Have we reached the top?  (This isn't quite the right question,       */
	/*   because even though the left triangulation might seem finished now, */
	/*   moving up on the right triangulation might reveal a new point of    */
	/*   the left triangulation.  And vice-versa.)                           */
	leftfinished = counterclockwise(upperleft, lowerleft, lowerright) <= 0.0;
	rightfinished = counterclockwise(upperright, lowerleft, lowerright) <= 0.0;
	if (leftfinished && rightfinished)
	{
	    /* Create the top new bounding triangle. */
	    maketriangle(&nextedge);
	    setorg(nextedge, lowerleft);
	    setdest(nextedge, lowerright);
	    /* Apex is intentionally left NULL. */
	    /* Connect it to the bounding boxes of the two triangulations. */
	    bond(nextedge, baseedge);
	    lnextself(nextedge);
	    bond(nextedge, rightcand);
	    lnextself(nextedge);
	    bond(nextedge, leftcand);
	    /* Special treatment for horizontal cuts. */
	    if (dwyer && (axis == 1))
	    {
		org(*farleft, farleftpt);
		apex(*farleft, farleftapex);
		dest(*farright, farrightpt);
		apex(*farright, farrightapex);
		sym(*farleft, checkedge);
		apex(checkedge, checkvertex);
		/* The pointers to the extremal points are restored to the leftmost */
		/*   and rightmost points (rather than topmost and bottommost).     */
		while (checkvertex[0] < farleftpt[0])
		{
		    lprev(checkedge, *farleft);
		    farleftapex = farleftpt;
		    farleftpt = checkvertex;
		    sym(*farleft, checkedge);
		    apex(checkedge, checkvertex);
		}
		while (farrightapex[0] > farrightpt[0])
		{
		    lprevself(*farright);
		    symself(*farright);
		    farrightpt = farrightapex;
		    apex(*farright, farrightapex);
		}
	    }
	    return;
	}
	/* Consider eliminating edges from the left triangulation. */
	if (!leftfinished)
	{
	    /* What vertex would be exposed if an edge were deleted? */
	    lprev(leftcand, nextedge);
	    symself(nextedge);
	    apex(nextedge, nextapex);
	    /* If nextapex is NULL, then no vertex would be exposed; the */
	    /*   triangulation would have been eaten right through.      */
	    if (nextapex != NULL)
	    {
		/* Check whether the edge is Delaunay. */
		badedge = incircle(lowerleft, lowerright, upperleft, nextapex) > 0.0;
		while (badedge)
		{
		    /* Eliminate the edge with an edge flip.  As a result, the    */
		    /*   left triangulation will have one more boundary triangle. */
		    lnextself(nextedge);
		    sym(nextedge, topcasing);
		    lnextself(nextedge);
		    sym(nextedge, sidecasing);
		    bond(nextedge, topcasing);
		    bond(leftcand, sidecasing);
		    lnextself(leftcand);
		    sym(leftcand, outercasing);
		    lprevself(nextedge);
		    bond(nextedge, outercasing);
		    /* Correct the vertices to reflect the edge flip. */
		    setorg(leftcand, lowerleft);
		    setdest(leftcand, NULL);
		    setapex(leftcand, nextapex);
		    setorg(nextedge, NULL);
		    setdest(nextedge, upperleft);
		    setapex(nextedge, nextapex);
		    /* Consider the newly exposed vertex. */
		    upperleft = nextapex;
		    /* What vertex would be exposed if another edge were deleted? */
		    triedgecopy(sidecasing, nextedge);
		    apex(nextedge, nextapex);
		    if (nextapex != NULL)
		    {
			/* Check whether the edge is Delaunay. */
			badedge = incircle(lowerleft, lowerright, upperleft, nextapex)
			    > 0.0;
		    }
		    else
		    {
			/* Avoid eating right through the triangulation. */
			badedge = 0;
		    }
		}
	    }
	}
	/* Consider eliminating edges from the right triangulation. */
	if (!rightfinished)
	{
	    /* What vertex would be exposed if an edge were deleted? */
	    lnext(rightcand, nextedge);
	    symself(nextedge);
	    apex(nextedge, nextapex);
	    /* If nextapex is NULL, then no vertex would be exposed; the */
	    /*   triangulation would have been eaten right through.      */
	    if (nextapex != NULL)
	    {
		/* Check whether the edge is Delaunay. */
		badedge = incircle(lowerleft, lowerright, upperright, nextapex) > 0.0;
		while (badedge)
		{
		    /* Eliminate the edge with an edge flip.  As a result, the     */
		    /*   right triangulation will have one more boundary triangle. */
		    lprevself(nextedge);
		    sym(nextedge, topcasing);
		    lprevself(nextedge);
		    sym(nextedge, sidecasing);
		    bond(nextedge, topcasing);
		    bond(rightcand, sidecasing);
		    lprevself(rightcand);
		    sym(rightcand, outercasing);
		    lnextself(nextedge);
		    bond(nextedge, outercasing);
		    /* Correct the vertices to reflect the edge flip. */
		    setorg(rightcand, NULL);
		    setdest(rightcand, lowerright);
		    setapex(rightcand, nextapex);
		    setorg(nextedge, upperright);
		    setdest(nextedge, NULL);
		    setapex(nextedge, nextapex);
		    /* Consider the newly exposed vertex. */
		    upperright = nextapex;
		    /* What vertex would be exposed if another edge were deleted? */
		    triedgecopy(sidecasing, nextedge);
		    apex(nextedge, nextapex);
		    if (nextapex != NULL)
		    {
			/* Check whether the edge is Delaunay. */
			badedge = incircle(lowerleft, lowerright, upperright, nextapex)
			    > 0.0;
		    }
		    else
		    {
			/* Avoid eating right through the triangulation. */
			badedge = 0;
		    }
		}
	    }
	}
	if (leftfinished || (!rightfinished &&
	    (incircle(upperleft, lowerleft, lowerright, upperright) > 0.0)))
	{
	    /* Knit the triangulations, adding an edge from `lowerleft' */
	    /*   to `upperright'.                                       */
	    bond(baseedge, rightcand);
	    lprev(rightcand, baseedge);
	    setdest(baseedge, lowerleft);
	    lowerright = upperright;
	    sym(baseedge, rightcand);
	    apex(rightcand, upperright);
	}
	else
	{
	    /* Knit the triangulations, adding an edge from `upperleft' */
	    /*   to `lowerright'.                                       */
	    bond(baseedge, leftcand);
	    lnext(leftcand, baseedge);
	    setorg(baseedge, lowerright);
	    lowerleft = upperleft;
	    sym(baseedge, leftcand);
	    apex(leftcand, upperleft);
	}
    }
}		/*end mergehulls*/

/*****************************************************************************/
/*                                                                           */
/*  divconqrecurse()   Recursively form a Delaunay triangulation by the      */
/*                     divide-and-conquer method.                            */
/*                                                                           */
/*  Recursively breaks down the problem into smaller pieces, which are       */
/*  knitted together by mergehulls().  The base cases (problems of two or    */
/*  three points) are handled specially here.                                */
/*                                                                           */
/*  On completion, `farleft' and `farright' are bounding triangles such that */
/*  the origin of `farleft' is the leftmost vertex (breaking ties by         */
/*  choosing the highest leftmost vertex), and the destination of            */
/*  `farright' is the rightmost vertex (breaking ties by choosing the        */
/*  lowest rightmost vertex).                                                */
/*                                                                           */
/*****************************************************************************/

LOCAL void divconqrecurse(
    point *sortarray,
    int vertices,
    int axis,
    triedge *farleft,
    triedge *farright)
{
    triedge midtri, tri1, tri2, tri3;
    triedge innerleft, innerright;
    double area;
    int divider;

    if (vertices == 2)
    {
	/* The triangulation of two vertices is an edge.  An edge is */
	/*   represented by two bounding triangles.                  */
	maketriangle(farleft);
	setorg(*farleft, sortarray[0]);
	setdest(*farleft, sortarray[1]);
	/* The apex is intentionally left NULL. */
	maketriangle(farright);
	setorg(*farright, sortarray[1]);
	setdest(*farright, sortarray[0]);
	/* The apex is intentionally left NULL. */
	bond(*farleft, *farright);
	lprevself(*farleft);
	lnextself(*farright);
	bond(*farleft, *farright);
	lprevself(*farleft);
	lnextself(*farright);
	bond(*farleft, *farright);
	/* Ensure that the origin of `farleft' is sortarray[0]. */
	lprev(*farright, *farleft);
	return;
    }
    else if (vertices == 3)
    {
	/* The triangulation of three vertices is either a triangle (with */
	/*   three bounding triangles) or two edges (with four bounding   */
	/*   triangles).  In either case, four triangles are created.     */
	maketriangle(&midtri);
	maketriangle(&tri1);
	maketriangle(&tri2);
	maketriangle(&tri3);
	area = counterclockwise(sortarray[0], sortarray[1], sortarray[2]);
	if (area == 0.0)
	{
	    /* Three collinear points; the triangulation is two edges. */
	    setorg(midtri, sortarray[0]);
	    setdest(midtri, sortarray[1]);
	    setorg(tri1, sortarray[1]);
	    setdest(tri1, sortarray[0]);
	    setorg(tri2, sortarray[2]);
	    setdest(tri2, sortarray[1]);
	    setorg(tri3, sortarray[1]);
	    setdest(tri3, sortarray[2]);
	    /* All apices are intentionally left NULL. */
	    bond(midtri, tri1);
	    bond(tri2, tri3);
	    lnextself(midtri);
	    lprevself(tri1);
	    lnextself(tri2);
	    lprevself(tri3);
	    bond(midtri, tri3);
	    bond(tri1, tri2);
	    lnextself(midtri);
	    lprevself(tri1);
	    lnextself(tri2);
	    lprevself(tri3);
	    bond(midtri, tri1);
	    bond(tri2, tri3);
	    /* Ensure that the origin of `farleft' is sortarray[0]. */
	    triedgecopy(tri1, *farleft);
	    /* Ensure that the destination of `farright' is sortarray[2]. */
	    triedgecopy(tri2, *farright);
	}
	else
	{
	    /* The three points are not collinear; the triangulation is one */
	    /*   triangle, namely `midtri'.                                 */
	    setorg(midtri, sortarray[0]);
	    setdest(tri1, sortarray[0]);
	    setorg(tri3, sortarray[0]);
	    /* Apices of tri1, tri2, and tri3 are left NULL. */
	    if (area > 0.0)
	    {
		/* The vertices are in counterclockwise order. */
		setdest(midtri, sortarray[1]);
		setorg(tri1, sortarray[1]);
		setdest(tri2, sortarray[1]);
		setapex(midtri, sortarray[2]);
		setorg(tri2, sortarray[2]);
		setdest(tri3, sortarray[2]);
	    }
	    else
	    {
		/* The vertices are in clockwise order. */
		setdest(midtri, sortarray[2]);
		setorg(tri1, sortarray[2]);
		setdest(tri2, sortarray[2]);
		setapex(midtri, sortarray[1]);
		setorg(tri2, sortarray[1]);
		setdest(tri3, sortarray[1]);
	    }
	    /* The topology does not depend on how the vertices are ordered. */
	    bond(midtri, tri1);
	    lnextself(midtri);
	    bond(midtri, tri2);
	    lnextself(midtri);
	    bond(midtri, tri3);
	    lprevself(tri1);
	    lnextself(tri2);
	    bond(tri1, tri2);
	    lprevself(tri1);
	    lprevself(tri3);
	    bond(tri1, tri3);
	    lnextself(tri2);
	    lprevself(tri3);
	    bond(tri2, tri3);
	    /* Ensure that the origin of `farleft' is sortarray[0]. */
	    triedgecopy(tri1, *farleft);
	    /* Ensure that the destination of `farright' is sortarray[2]. */
	    if (area > 0.0)
	    {
		triedgecopy(tri2, *farright);
	    }
	    else
	    {
		lnext(*farleft, *farright);
	    }
	}
	return;
    }
    else
    {
	/* Split the vertices in half. */
	divider = vertices >> 1;
	/* Recursively triangulate each half. */
	divconqrecurse(sortarray, divider, 1 - axis, farleft, &innerleft);
	divconqrecurse(&sortarray[divider], vertices - divider, 1 - axis,
	    &innerright, farright);
	/* Merge the two triangulations into one. */
	mergehulls(farleft, &innerleft, &innerright, farright, axis);
    }
}		/*end divconqrecurse*/

LOCAL long removeghosts(
    triedge *startghost)
{
    triedge searchedge;
    triedge dissolveedge;
    triedge deadtri;
    point markorg;
    long hullsize;
    triangle ptr;                       /* Temporary variable used by sym(). */

    /* Find an edge on the convex hull to start point location from. */
    lprev(*startghost, searchedge);
    symself(searchedge);
    dummytri[0] = encode(searchedge);
    /* Remove the bounding box and count the convex hull edges. */
    triedgecopy(*startghost, dissolveedge);
    hullsize = 0;
    do
    {
	hullsize++;
	lnext(dissolveedge, deadtri);
	lprevself(dissolveedge);
	symself(dissolveedge);
	/* If no PSLG is involved, set the boundary markers of all the points */
	/*   on the convex hull.  If a PSLG is used, this step is done later. */
	if (!TriOpts.poly)
	{
	    /* Watch out for the case where all the input points are collinear. */
	    if (dissolveedge.tri != dummytri)
	    {
		org(dissolveedge, markorg);
		if (pointmark(markorg) == 0)
		{
		    setpointmark(markorg, 1);
		}
	    }
	}
	/* Remove a bounding triangle from a convex hull triangle. */
	dissolve(dissolveedge);
	/* Find the next bounding triangle. */
	sym(deadtri, dissolveedge);
	/* Delete the bounding triangle. */
	triangledealloc(deadtri.tri);
    } while (!triedgeequal(dissolveedge, *startghost));
    return hullsize;
}		/*end removeghosts*/

/*****************************************************************************/
/*                                                                           */
/*  divconqdelaunay()   Form a Delaunay triangulation by the divide-and-     */
/*                      conquer method.                                      */
/*                                                                           */
/*  Sorts the points, calls a recursive procedure to triangulate them, and   */
/*  removes the bounding box, setting boundary markers as appropriate.       */
/*                                                                           */
/*****************************************************************************/

LOCAL long divconqdelaunay(void)
{
    static point* sortarray = NULL;
    static size_t size_sortarray;
    triedge       hullleft, hullright;
    int           divider;
    int           i, j;

    /* Allocate an array of pointers to points for sorting. */
    sortarray = (point *) allocate_storage(sortarray,&size_sortarray,
					   inpoints*sizeof(point));
    traversalinit(&points);
    for (i = 0; i < inpoints; i++)
    {
	sortarray[i] = pointtraverse();
    }
    /* Sort the points. */
    pointsort(sortarray, inpoints);
    /* Discard duplicate points, which can really mess up the algorithm. */
    i = 0;
    for (j = 1; j < inpoints; j++)
    {
	if ((sortarray[i][0] != sortarray[j][0])
	    || (sortarray[i][1] != sortarray[j][1]))
	{
	    i++;
	    sortarray[i] = sortarray[j];
	}
    }
    i++;
    if (dwyer)
    {
	/* Re-sort the array of points to accommodate alternating cuts. */
	divider = i >> 1;
	if (i - divider >= 2)
	{
	    if (divider >= 2)
	    {
		alternateaxes(sortarray, divider, 1);
	    }
	    alternateaxes(&sortarray[divider], i - divider, 1);
	}
    }
    /* Form the Delaunay triangulation. */
    divconqrecurse(sortarray, i, 0, &hullleft, &hullright);
    /*free(sortarray);JWG*/

    return removeghosts(&hullleft);
}		/*end divconqdelaunay*/

/**                                                                         **/
/**                                                                         **/
/********* Divide-and-conquer Delaunay triangulation ends here       *********/

/********* Incremental Delaunay triangulation begins here            *********/
/**                                                                         **/
/**                                                                         **/

/*****************************************************************************/
/*                                                                           */
/*  delaunay()   Form a Delaunay triangulation.                              */
/*                                                                           */
/*****************************************************************************/

LOCAL long delaunay(void)
{
    eextras = 0;
    initializetrisegpools();

    return divconqdelaunay();
}		/*end delaunay*/

/**                                                                         **/
/**                                                                         **/
/********* General mesh construction routines end here               *********/

/********* Segment (shell edge) insertion begins here                *********/
/**                                                                         **/
/**                                                                         **/

/*****************************************************************************/
/*                                                                           */
/*  finddirection()   Find the first triangle on the path from one point     */
/*                    to another.                                            */
/*                                                                           */
/*  Finds the triangle that intersects a line segment drawn from the         */
/*  origin of `searchtri' to the point `endpoint', and returns the result    */
/*  in `searchtri'.  The origin of `searchtri' does not change, even though  */
/*  the triangle returned may differ from the one passed in.  This routine   */
/*  is used to find the direction to move in to get from one point to        */
/*  another.                                                                 */
/*                                                                           */
/*  The return value notes whether the destination or apex of the found      */
/*  triangle is collinear with the two points in question.                   */
/*                                                                           */
/*****************************************************************************/

LOCAL finddirectionresult finddirection(
    triedge *searchtri,
    point endpoint)
{
    triedge checktri;
    point startpoint;
    point leftpoint, rightpoint;
    double leftccw, rightccw;
    int leftflag, rightflag;
    triangle ptr;           /* Temporary variable used by onext() and oprev(). */

    org(*searchtri, startpoint);
    dest(*searchtri, rightpoint);
    apex(*searchtri, leftpoint);
    /* Is `endpoint' to the left? */
    leftccw = counterclockwise(endpoint, startpoint, leftpoint);
    leftflag = leftccw > 0.0;
    /* Is `endpoint' to the right? */
    rightccw = counterclockwise(startpoint, endpoint, rightpoint);
    rightflag = rightccw > 0.0;
    if (leftflag && rightflag)
    {
	/* `searchtri' faces directly away from `endpoint'.  We could go */
	/*   left or right.  Ask whether it's a triangle or a boundary   */
	/*   on the left.                                                */
	onext(*searchtri, checktri);
	if (checktri.tri == dummytri)
	{
	    leftflag = 0;
	}
	else
	{
	    rightflag = 0;
	}
    }
    while (leftflag)
    {
	/* Turn left until satisfied. */
	onextself(*searchtri);
	if (searchtri->tri == dummytri)
	{
	    (void) printf("Internal error in finddirection():  "
			  "Unable to find a\n"
	                  "  triangle leading from (%.12g, %.12g) to"
	                  "  (%.12g, %.12g).\n",
			  startpoint[0],startpoint[1],
	                  endpoint[0],endpoint[1]);
	    internalerror();
	}
	apex(*searchtri, leftpoint);
	rightccw = leftccw;
	leftccw = counterclockwise(endpoint, startpoint, leftpoint);
	leftflag = leftccw > 0.0;
    }
    while (rightflag)
    {
	/* Turn right until satisfied. */
	oprevself(*searchtri);
	if (searchtri->tri == dummytri)
	{
	    (void) printf("Internal error in finddirection():  "
			  "Unable to find a\n"
	                  "  triangle leading from (%.12g, %.12g) to"
	                  "  (%.12g, %.12g).\n",
			  startpoint[0],startpoint[1],endpoint[0], endpoint[1]);
	    internalerror();
	}
	dest(*searchtri, rightpoint);
	leftccw = rightccw;
	rightccw = counterclockwise(startpoint, endpoint, rightpoint);
	rightflag = rightccw > 0.0;
    }
    if (leftccw == 0.0)
	return LEFTCOLLINEAR;
    else if (rightccw == 0.0)
	return RIGHTCOLLINEAR;
    else
	return WITHIN;
}		/*end finddirection*/

/*****************************************************************************/
/*                                                                           */
/*  segmentintersection()   Find the intersection of an existing segment     */
/*                          and a segment that is being inserted.  Insert    */
/*                          a point at the intersection, splitting an        */
/*                          existing shell edge.                             */
/*                                                                           */
/*  The segment being inserted connects the apex of splittri to endpoint2.   */
/*  splitshelle is the shell edge being split, and MUST be opposite          */
/*  splittri.  Hence, the edge being split connects the origin and           */
/*  destination of splittri.                                                 */
/*                                                                           */
/*  On completion, splittri is a handle having the newly inserted            */
/*  intersection point as its origin, and endpoint1 as its destination.      */
/*                                                                           */
/*****************************************************************************/

LOCAL void segmentintersection(
    triedge *splittri,
    edge *splitshelle,
    point endpoint2)
{
    point endpoint1;
    point torg, tdest;
    point leftpoint, rightpoint;
    point newpoint;
    insertsiteresult success;
    double ex, ey;
    double tx, ty;
    double etx, ety;
    double split, denom;
    int i;
    triangle ptr;                       /* Temporary variable used by onext(). */

    /* Find the other three segment endpoints. */
    apex(*splittri, endpoint1);
    org(*splittri, torg);
    dest(*splittri, tdest);
    /* Segment intersection formulae; see the Antonio reference. */
    tx = tdest[0] - torg[0];
    ty = tdest[1] - torg[1];
    ex = endpoint2[0] - endpoint1[0];
    ey = endpoint2[1] - endpoint1[1];
    etx = torg[0] - endpoint2[0];
    ety = torg[1] - endpoint2[1];
    denom = ty * ex - tx * ey;
    if (denom == 0.0)
    {
	(void) printf("Internal error in segmentintersection():"
	              "  Attempt to find intersection of parallel segments.\n");
	internalerror();
    }
    split = (ey * etx - ex * ety) / denom;
    /* Create the new point. */
    newpoint = (point) poolalloc(&points);
    /* Interpolate its coordinate and attributes. */
    for (i = 0; i < 2 + nextras; i++)
    {
	newpoint[i] = torg[i] + split * (tdest[i] - torg[i]);
    }
    setpointmark(newpoint, mark(*splitshelle));
    /* Insert the intersection point.  This should always succeed. */
    success = insertsite(newpoint, splittri, splitshelle, 0, 0);
    if (success != SUCCESSFULPOINT)
    {
	(void) printf("Internal error in segmentintersection():\n"
	              "  Failure to split a segment.\n");
	internalerror();
    }
    if (steinerleft > 0)
	steinerleft--;

    /* Inserting the point may have caused edge flips.  We wish to rediscover */
    /*   the edge connecting endpoint1 to the new intersection point.         */
    (void) finddirection(splittri, endpoint1);
    dest(*splittri, rightpoint);
    apex(*splittri, leftpoint);
    if ((leftpoint[0] == endpoint1[0]) && (leftpoint[1] == endpoint1[1]))
    {
	onextself(*splittri);
    }
    else if ((rightpoint[0]!=endpoint1[0]) || (rightpoint[1]!=endpoint1[1]))
    {
	(void) printf("Internal error in segmentintersection():\n"
	              "  Topological inconsistency after splitting "
		      "a segment.\n");
	internalerror();
    }
    /* `splittri' should have destination endpoint1. */
}		/*end segmentintersection*/

/*****************************************************************************/
/*                                                                           */
/*  scoutsegment()   Scout the first triangle on the path from one endpoint  */
/*                   to another, and check for completion (reaching the      */
/*                   second endpoint), a collinear point, and the            */
/*                   intersection of two segments.                           */
/*                                                                           */
/*  Returns one if the entire segment is successfully inserted, and zero if  */
/*  the job must be finished by conformingedge() or constrainededge().       */
/*                                                                           */
/*  If the first triangle on the path has the second endpoint as its         */
/*  destination or apex, a shell edge is inserted and the job is done.       */
/*                                                                           */
/*  If the first triangle on the path has a destination or apex that lies on */
/*  the segment, a shell edge is inserted connecting the first endpoint to   */
/*  the collinear point, and the search is continued from the collinear      */
/*  point.                                                                   */
/*                                                                           */
/*  If the first triangle on the path has a shell edge opposite its origin,  */
/*  then there is a segment that intersects the segment being inserted.      */
/*  Their intersection point is inserted, splitting the shell edge.          */
/*                                                                           */
/*  Otherwise, return zero.                                                  */
/*                                                                           */
/*****************************************************************************/

LOCAL int scoutsegment(
    triedge *searchtri,
     point endpoint2,
     int newmark)
{
    triedge crosstri;
    edge crossedge;
    point leftpoint, rightpoint;
    finddirectionresult collinear;
    shelle sptr;                    /* Temporary variable used by tspivot(). */

    collinear = finddirection(searchtri, endpoint2);
    dest(*searchtri, rightpoint);
    apex(*searchtri, leftpoint);
    if (((leftpoint[0] == endpoint2[0]) && (leftpoint[1] == endpoint2[1])) ||
        ((rightpoint[0] == endpoint2[0]) && (rightpoint[1] == endpoint2[1])))
    {
	/* The segment is already an edge in the mesh. */
	if ((leftpoint[0] == endpoint2[0]) && (leftpoint[1] == endpoint2[1]))
	{
	    lprevself(*searchtri);
	}
	/* Insert a shell edge, if there isn't already one there. */
	insertshelle(searchtri, newmark);
	return 1;
    }
    else if (collinear == LEFTCOLLINEAR)
    {
	/* We've collided with a point between the segment's endpoints. */
	/* Make the collinear point be the triangle's origin. */
	lprevself(*searchtri);
	insertshelle(searchtri, newmark);
	/* Insert the remainder of the segment. */
	return scoutsegment(searchtri, endpoint2, newmark);
    }
    else if (collinear == RIGHTCOLLINEAR)
    {
	/* We've collided with a point between the segment's endpoints. */
	insertshelle(searchtri, newmark);
	/* Make the collinear point be the triangle's origin. */
	lnextself(*searchtri);
	/* Insert the remainder of the segment. */
	return scoutsegment(searchtri, endpoint2, newmark);
    }
    else
    {
	lnext(*searchtri, crosstri);
	tspivot(crosstri, crossedge);
	/* Check for a crossing segment. */
	if (crossedge.sh == dummysh)
	{
	    return 0;
	}
	else
	{
	    (void) (*searchtri).tri[plus1mod3[(*searchtri).orient] + 3];
	    /* Insert a point at the intersection. */
	    segmentintersection(&crosstri, &crossedge, endpoint2);
	    triedgecopy(crosstri, *searchtri);
	    insertshelle(searchtri, newmark);
	    /* Insert the remainder of the segment. */
	    return scoutsegment(searchtri, endpoint2, newmark);
	}
    }
}		/*end scoutsegment*/

/*****************************************************************************/
/*                                                                           */
/*  delaunayfixup()   Enforce the Delaunay condition at an edge, fanning out */
/*                    recursively from an existing point.  Pay special       */
/*                    attention to stacking inverted triangles.              */
/*                                                                           */
/*  This is a support routine for inserting segments into a constrained      */
/*  Delaunay triangulation.                                                  */
/*                                                                           */
/*  The origin of fixuptri is treated as if it has just been inserted, and   */
/*  the local Delaunay condition needs to be enforced.  It is only enforced  */
/*  in one sector, however, that being the angular range defined by          */
/*  fixuptri.                                                                */
/*                                                                           */
/*  This routine also needs to make decisions regarding the "stacking" of    */
/*  triangles.  (Read the description of constrainededge() below before      */
/*  reading on here, so you understand the algorithm.)  If the position of   */
/*  the new point (the origin of fixuptri) indicates that the vertex before  */
/*  it on the polygon is a reflex vertex, then "stack" the triangle by       */
/*  doing nothing.  (fixuptri is an inverted triangle, which is how stacked  */
/*  triangles are identified.)                                               */
/*                                                                           */
/*  Otherwise, check whether the vertex before that was a reflex vertex.     */
/*  If so, perform an edge flip, thereby eliminating an inverted triangle    */
/*  (popping it off the stack).  The edge flip may result in the creation    */
/*  of a new inverted triangle, depending on whether or not the new vertex   */
/*  is visible to the vertex three edges behind on the polygon.              */
/*                                                                           */
/*  If neither of the two vertices behind the new vertex are reflex          */
/*  vertices, fixuptri and fartri, the triangle opposite it, are not         */
/*  inverted; hence, ensure that the edge between them is locally Delaunay.  */
/*                                                                           */
/*  `leftside' indicates whether or not fixuptri is to the left of the       */
/*  segment being inserted.  (Imagine that the segment is pointing up from   */
/*  endpoint1 to endpoint2.)                                                 */
/*                                                                           */
/*****************************************************************************/

LOCAL void delaunayfixup(
    triedge *fixuptri,
    int leftside)
{
    triedge neartri;
    triedge fartri;
    edge faredge;
    point nearpoint, leftpoint, rightpoint, farpoint;
    triangle ptr;                         /* Temporary variable used by sym(). */
    shelle sptr;                      /* Temporary variable used by tspivot(). */

    lnext(*fixuptri, neartri);
    sym(neartri, fartri);
    /* Check if the edge opposite the origin of fixuptri can be flipped. */
    if (fartri.tri == dummytri)
    {
	return;
    }
    tspivot(neartri, faredge);
    if (faredge.sh != dummysh)
    {
	return;
    }
    /* Find all the relevant vertices. */
    apex(neartri, nearpoint);
    org(neartri, leftpoint);
    dest(neartri, rightpoint);
    apex(fartri, farpoint);
    /* Check whether the previous polygon vertex is a reflex vertex. */
    if (leftside)
    {
	if (counterclockwise(nearpoint, leftpoint, farpoint) <= 0.0)
	{
	    /* leftpoint is a reflex vertex too.  Nothing can */
	    /*   be done until a convex section is found.     */
	    return;
	}
    }
    else
    {
	if (counterclockwise(farpoint, rightpoint, nearpoint) <= 0.0)
	{
	    /* rightpoint is a reflex vertex too.  Nothing can */
	    /*   be done until a convex section is found.      */
	    return;
	}
    }
    if (counterclockwise(rightpoint, leftpoint, farpoint) > 0.0)
    {
	/* fartri is not an inverted triangle, and farpoint is not a reflex */
	/*   vertex.  As there are no reflex vertices, fixuptri isn't an    */
	/*   inverted triangle, either.  Hence, test the edge between the   */
	/*   triangles to ensure it is locally Delaunay.                    */
	if (incircle(leftpoint, farpoint, rightpoint, nearpoint) <= 0.0)
	{
	    return;
	}
	/* Not locally Delaunay; go on to an edge flip. */
    }    /* else fartri is inverted; remove it from the stack by flipping. */
    flip(&neartri);
    lprevself(*fixuptri);/* Restore the origin of fixuptri after the flip. */
    /* Recursively process the two triangles that result from the flip. */
    delaunayfixup(fixuptri, leftside);
    delaunayfixup(&fartri, leftside);
}		/*end delaunayfixup*/

/*****************************************************************************/
/*                                                                           */
/*  constrainededge()   Force a segment into a constrained Delaunay          */
/*                      triangulation by deleting the triangles it           */
/*                      intersects, and triangulating the polygons that      */
/*                      form on each side of it.                             */
/*                                                                           */
/*  Generates a single edge connecting `endpoint1' to `endpoint2'.  The      */
/*  triangle `starttri' has `endpoint1' as its origin.  `newmark' is the     */
/*  boundary marker of the segment.                                          */
/*                                                                           */
/*  To insert a segment, every triangle whose interior intersects the        */
/*  segment is deleted.  The union of these deleted triangles is a polygon   */
/*  (which is not necessarily monotone, but is close enough), which is       */
/*  divided into two polygons by the new segment.  This routine's task is    */
/*  to generate the Delaunay triangulation of these two polygons.            */
/*                                                                           */
/*  You might think of this routine's behavior as a two-step process.  The   */
/*  first step is to walk from endpoint1 to endpoint2, flipping each edge    */
/*  encountered.  This step creates a fan of edges connected to endpoint1,   */
/*  including the desired edge to endpoint2.  The second step enforces the   */
/*  Delaunay condition on each side of the segment in an incremental manner: */
/*  proceeding along the polygon from endpoint1 to endpoint2 (this is done   */
/*  independently on each side of the segment), each vertex is "enforced"    */
/*  as if it had just been inserted, but affecting only the previous         */
/*  vertices.  The result is the same as if the vertices had been inserted   */
/*  in the order they appear on the polygon, so the result is Delaunay.      */
/*                                                                           */
/*  In truth, constrainededge() interleaves these two steps.  The procedure  */
/*  walks from endpoint1 to endpoint2, and each time an edge is encountered  */
/*  and flipped, the newly exposed vertex (at the far end of the flipped     */
/*  edge) is "enforced" upon the previously flipped edges, usually affecting */
/*  only one side of the polygon (depending upon which side of the segment   */
/*  the vertex falls on).                                                    */
/*                                                                           */
/*  The algorithm is complicated by the need to handle polygons that are not */
/*  convex.  Although the polygon is not necessarily monotone, it can be     */
/*  triangulated in a manner similar to the stack-based algorithms for       */
/*  monotone polygons.  For each reflex vertex (local concavity) of the      */
/*  polygon, there will be an inverted triangle formed by one of the edge    */
/*  flips.  (An inverted triangle is one with negative area - that is, its   */
/*  vertices are arranged in clockwise order - and is best thought of as a   */
/*  wrinkle in the fabric of the mesh.)  Each inverted triangle can be       */
/*  thought of as a reflex vertex pushed on the stack, waiting to be fixed   */
/*  later.                                                                   */
/*                                                                           */
/*  A reflex vertex is popped from the stack when a vertex is inserted that  */
/*  is visible to the reflex vertex.  (However, if the vertex behind the     */
/*  reflex vertex is not visible to the reflex vertex, a new inverted        */
/*  triangle will take its place on the stack.)  Th*/
/**/
/**/
/*****************************************************************************/

LOCAL void constrainededge(
    triedge *starttri,
    point endpoint2,
    int newmark)
{
    triedge fixuptri, fixuptri2;
    edge fixupedge;
    point endpoint1;
    point farpoint;
    double area;
    int collision;
    int done;
    triangle ptr;             /* Temporary variable used by sym() and oprev(). */
    shelle sptr;                      /* Temporary variable used by tspivot(). */

    org(*starttri, endpoint1);
    lnext(*starttri, fixuptri);
    flip(&fixuptri);
    /* `collision' indicates whether we have found a point directly */
    /*   between endpoint1 and endpoint2.                           */
    collision = 0;
    done = 0;
    do
    {
	org(fixuptri, farpoint);
	/* `farpoint' is the extreme point of the polygon we are "digging" */
	/*   to get from endpoint1 to endpoint2.                           */
	if ((farpoint[0] == endpoint2[0]) && (farpoint[1] == endpoint2[1]))
	{
	    oprev(fixuptri, fixuptri2);
	    /* Enforce the Delaunay condition around endpoint2. */
	    delaunayfixup(&fixuptri, 0);
	    delaunayfixup(&fixuptri2, 1);
	    done = 1;
	}
	else
	{
	    /* Check whether farpoint is to the left or right of the segment */
	    /*   being inserted, to decide which edge of fixuptri to dig     */
	    /*   through next.                                               */
	    area = counterclockwise(endpoint1, endpoint2, farpoint);
	    if (area == 0.0)
	    {
		/* We've collided with a point between endpoint1 and endpoint2. */
		collision = 1;
		oprev(fixuptri, fixuptri2);
		/* Enforce the Delaunay condition around farpoint. */
		delaunayfixup(&fixuptri, 0);
		delaunayfixup(&fixuptri2, 1);
		done = 1;
	    }
	    else
	    {
		if (area > 0.0)
		{         /* farpoint is to the left of the segment. */
		    oprev(fixuptri, fixuptri2);
		    /* Enforce the Delaunay condition around farpoint, on the */
		    /*   left side of the segment only.                       */
		    delaunayfixup(&fixuptri2, 1);
		    /* Flip the edge that crosses the segment.  After the edge is */
		    /*   flipped, one of its endpoints is the fan vertex, and the */
		    /*   destination of fixuptri is the fan vertex.               */
		    lprevself(fixuptri);
		}
		else
		{                 /* farpoint is to the right of the segment. */
		    delaunayfixup(&fixuptri, 0);
		    /* Flip the edge that crosses the segment.  After the edge is */
		    /*   flipped, one of its endpoints is the fan vertex, and the */
		    /*   destination of fixuptri is the fan vertex.               */
		    oprevself(fixuptri);
		}
		/* Check for two intersecting segments. */
		tspivot(fixuptri, fixupedge);
		if (fixupedge.sh == dummysh)
		{
		    flip(&fixuptri);   /* May create an inverted triangle on the left. */
		}
		else
		{
		    /* We've collided with a segment between endpoint1 and endpoint2. */
		    collision = 1;
		    /* Insert a point at the intersection. */
		    segmentintersection(&fixuptri, &fixupedge, endpoint2);
		    done = 1;
		}
	    }
	}
    } while (!done);
    /* Insert a shell edge to make the segment permanent. */
    insertshelle(&fixuptri, newmark);
    /* If there was a collision with an interceding vertex, install another */
    /*   segment connecting that vertex with endpoint2.                     */
    if (collision)
    {
	/* Insert the remainder of the segment. */
	if (!scoutsegment(&fixuptri, endpoint2, newmark))
	{
	    constrainededge(&fixuptri, endpoint2, newmark);
	}
    }
}		/*end constrainededge*/

/*****************************************************************************/
/*                                                                           */
/*  insertsegment()   Insert a PSLG segment into a triangulation.            */
/*                                                                           */
/*****************************************************************************/

LOCAL void insertsegment(
    point endpoint1,
    point endpoint2,
    int newmark)
{
    triedge searchtri1, searchtri2;
    triangle encodedtri;
    point checkpoint;
    triangle ptr;                        /* Temporary variable used by sym(). */

    /* Find a triangle whose origin is the segment's first endpoint. */
    checkpoint = NULL;
    encodedtri = point2tri(endpoint1);
    if (encodedtri != NULL)
    {
	decode(encodedtri, searchtri1);
	org(searchtri1, checkpoint);
    }
    if (checkpoint != endpoint1)
    {
	/* Find a boundary triangle to search from. */
	searchtri1.tri = dummytri;
	searchtri1.orient = 0;
	symself(searchtri1);
	/* Search for the segment's first endpoint by point location. */
	if (locate(endpoint1, &searchtri1) != ONVERTEX)
	{
	    (void) printf("Internal error in insertsegment():  "
			  "Unable to locate PSLG point\n"
	                  "  (%.12g, %.12g) in triangulation.\n",
	                  endpoint1[0], endpoint1[1]);
	    internalerror();
	}
    }
    /* Remember this triangle to improve subsequent point location. */
    triedgecopy(searchtri1, recenttri);
    /* Scout the beginnings of a path from the first endpoint */
    /*   toward the second.                                   */
    if (scoutsegment(&searchtri1, endpoint2, newmark))
    {
	/* The segment was easily inserted. */
	return;
    }
    /* The first endpoint may have changed if a collision with an intervening */
    /*   vertex on the segment occurred.                                      */
    org(searchtri1, endpoint1);

    /* Find a triangle whose origin is the segment's second endpoint. */
    checkpoint = NULL;
    encodedtri = point2tri(endpoint2);
    if (encodedtri != NULL)
    {
	decode(encodedtri, searchtri2);
	org(searchtri2, checkpoint);
    }
    if (checkpoint != endpoint2)
    {
	/* Find a boundary triangle to search from. */
	searchtri2.tri = dummytri;
	searchtri2.orient = 0;
	symself(searchtri2);
	/* Search for the segment's second endpoint by point location. */
	if (locate(endpoint2, &searchtri2) != ONVERTEX)
	{
	    (void) printf("Internal error in insertsegment():  "
			  "Unable to locate PSLG point\n"
	                  "  (%.12g, %.12g) in triangulation.\n",
	                  endpoint2[0], endpoint2[1]);
	    internalerror();
	}
    }
    /* Remember this triangle to improve subsequent point location. */
    triedgecopy(searchtri2, recenttri);
    /* Scout the beginnings of a path from the second endpoint */
    /*   toward the first.                                     */
    if (scoutsegment(&searchtri2, endpoint1, newmark))
    {
	/* The segment was easily inserted. */
	return;
    }
    /* The second endpoint may have changed if a collision with an */
    /* intervening vertex on the segment occurred.                 */
    org(searchtri2, endpoint2);

    /* Insert the segment directly into the triangulation. */
    constrainededge(&searchtri1, endpoint2, newmark);
}		/*end insertsegment*/

/*****************************************************************************/
/*                                                                           */
/*  markhull()   Cover the convex hull of a triangulation with shell edges.  */
/*                                                                           */
/*****************************************************************************/

LOCAL void markhull(void)
{
    triedge hulltri;
    triedge nexttri;
    triedge starttri;
    triangle ptr;           /* Temporary variable used by sym() and oprev(). */

    /* Find a triangle handle on the hull. */
    hulltri.tri = dummytri;
    hulltri.orient = 0;
    symself(hulltri);
    /* Remember where we started so we know when to stop. */
    triedgecopy(hulltri, starttri);
    /* Go once counterclockwise around the convex hull. */
    do
    {
	/* Create a shell edge if there isn't already one here. */
	insertshelle(&hulltri, 1);
	/* To find the next hull edge, go clockwise around the next vertex. */
	lnextself(hulltri);
	oprev(hulltri, nexttri);
	while (nexttri.tri != dummytri)
	{
	    triedgecopy(nexttri, hulltri);
	    oprev(hulltri, nexttri);
	}
    } while (!triedgeequal(hulltri, starttri));
}		/*end markhull*/

/*****************************************************************************/
/*                                                                           */
/*  formskeleton()   Create the shell edges of a triangulation, including    */
/*                   PSLG edges and edges on the convex hull.                */
/*                                                                           */
/*  The PSLG edges are read from a .poly file.  The return value is the      */
/*  number of segments in the file.                                          */
/*                                                                           */
/*****************************************************************************/


LOCAL size_t formskeleton(
    int *segmentlist,
    int *segmentmarkerlist,
    size_t numberofsegments)
{
    char polyfilename[6];
    int index;
    point endpoint1, endpoint2;
    size_t segments;
    int segmentmarkers;
    int end1, end2;
    int boundmarker;
    int i;

    if (TriOpts.poly)
    {
	strcpy(polyfilename, "input");
	segments = numberofsegments;
	segmentmarkers = (segmentmarkerlist != NULL);
	index = 0;
	/* If segments are to be inserted, compute a mapping */
	/*   from points to triangles.                       */
	if (segments > 0)
	{
	    makepointmap();
	}

	boundmarker = 0;
	/* Read and insert the segments. */
	for (i = 1; i <= segments; i++)
	{
	    end1 = segmentlist[index++];
	    end2 = segmentlist[index++];
	    if (segmentmarkers)
	    {
		boundmarker = segmentmarkerlist[i - 1];
	    }
	    if ((end1 < TriOpts.firstnumber) ||
		(end1 >= TriOpts.firstnumber + inpoints))
	    {
		(void) printf("WARNING in formskeleton(), "
			      "Invalid first endpoint of segment "
		              "%d in %s.\n", i,polyfilename);
	    }
	    else if ((end2 < TriOpts.firstnumber) ||
		     (end2 >= TriOpts.firstnumber + inpoints))
	    {
		(void) printf("WARNING in Invalid second endpoint of segment "
		              "%d in %s.\n", i,polyfilename);
	    }
	    else
	    {
		endpoint1 = getpoint(end1);
		endpoint2 = getpoint(end2);
		if ((endpoint1[0] == endpoint2[0]) &&
		    (endpoint1[1] == endpoint2[1]))
		{
		    (void) printf("WARNING in Endpoints of segment %d are "
			          "coincident in %s.\n",i, polyfilename);
		}
		else
		{
		    insertsegment(endpoint1, endpoint2, boundmarker);
		}
	    }
	}
    }
    else
    {
	segments = 0;
    }
    if (TriOpts.convex || !TriOpts.poly)
    {
	/* Enclose the convex hull with shell edges. */
	markhull();
    }
    return segments;
}		/*end formskeleton*/

/**                                                                         **/
/**                                                                         **/
/********* Segment (shell edge) insertion ends here                  *********/

/********* Carving out holes and concavities begins here             *********/
/**                                                                         **/
/**                                                                         **/

/*****************************************************************************/
/*                                                                           */
/*  infecthull()   Virally infect all of the triangles of the convex hull    */
/*                 that are not protected by shell edges.  Where there are   */
/*                 shell edges, set boundary markers as appropriate.         */
/*                                                                           */
/*****************************************************************************/

LOCAL void infecthull(void)
{
    triedge hulltri;
    triedge nexttri;
    triedge starttri;
    edge hulledge;
    triangle **deadtri;
    point horg, hdest;
    triangle ptr;                       /* Temporary variable used by sym(). */
    shelle sptr;                     /* Temporary variable used by tspivot(). */

    /* Find a triangle handle on the hull. */
    hulltri.tri = dummytri;
    hulltri.orient = 0;
    symself(hulltri);
    /* Remember where we started so we know when to stop. */
    triedgecopy(hulltri, starttri);
    /* Go once counterclockwise around the convex hull. */
    do
    {
	/* Ignore triangles that are already infected. */
	if (!infected(hulltri))
	{
	    /* Is the triangle protected by a shell edge? */
	    tspivot(hulltri, hulledge);
	    if (hulledge.sh == dummysh)
	    {
		/* The triangle is not protected; infect it. */
		infect(hulltri);
		deadtri = (triangle **) poolalloc(&viri);
		*deadtri = hulltri.tri;
	    }
	    else
	    {
		/* The triangle is protected; set boundary markers if appropriate. */
		if (mark(hulledge) == 0)
		{
		    setmark(hulledge, 1);
		    org(hulltri, horg);
		    dest(hulltri, hdest);
		    if (pointmark(horg) == 0)
		    {
			setpointmark(horg, 1);
		    }
		    if (pointmark(hdest) == 0)
		    {
			setpointmark(hdest, 1);
		    }
		}
	    }
	}
	/* To find the next hull edge, go clockwise around the next vertex. */
	lnextself(hulltri);
	oprev(hulltri, nexttri);
	while (nexttri.tri != dummytri)
	{
	    triedgecopy(nexttri, hulltri);
	    oprev(hulltri, nexttri);
	}
    } while (!triedgeequal(hulltri, starttri));
}		/*end infecthull*/

/*****************************************************************************/
/*                                                                           */
/*  plague()   Spread the virus from all infected triangles to any neighbors */
/*             not protected by shell edges.  Delete all infected triangles. */
/*                                                                           */
/*  This is the procedure that actually creates holes and concavities.       */
/*                                                                           */
/*  This procedure operates in two phases.  The first phase identifies all   */
/*  the triangles that will die, and marks them as infected.  They are       */
/*  marked to ensure that each triangle is added to the virus pool only      */
/*  once, so the procedure will terminate.                                   */
/*                                                                           */
/*  The second phase actually eliminates the infected triangles.  It also    */
/*  eliminates orphaned points.                                              */
/*                                                                           */
/*****************************************************************************/

LOCAL void plague(void)
{
    triedge testtri;
    triedge neighbor;
    triangle **virusloop;
    triangle **deadtri;
    edge neighborshelle;
    point testpoint;
    point norg, ndest;
    int killorg;
    triangle ptr;            /* Temporary variable used by sym() and onext(). */
    shelle sptr;                     /* Temporary variable used by tspivot(). */

    /* Loop through all the infected triangles, spreading the virus to */
    /*   their neighbors, then to their neighbors' neighbors.          */
    traversalinit(&viri);
    virusloop = (triangle **) traverse(&viri);
    while (virusloop != NULL)
    {
	testtri.tri = *virusloop;
	/* A triangle is marked as infected by messing with one of its shell */
	/*   edges, setting it to an illegal value.  Hence, we have to       */
	/*   temporarily uninfect this triangle so that we can examine its   */
	/*   adjacent shell edges.                                           */
	uninfect(testtri);
	/* Check each of the triangle's three neighbors. */
	for (testtri.orient = 0; testtri.orient < 3; testtri.orient++)
	{
	    /* Find the neighbor. */
	    sym(testtri, neighbor);
	    /* Check for a shell between the triangle and its neighbor. */
	    tspivot(testtri, neighborshelle);
	    /* Check if the neighbor is nonexistent or already infected. */
	    if ((neighbor.tri == dummytri) || infected(neighbor))
	    {
		if (neighborshelle.sh != dummysh)
		{
		    /* There is a shell edge separating the triangle from its */
		    /*   neighbor, but both triangles are dying, so the shell */
		    /*   edge dies too.                                       */
		    shelledealloc(neighborshelle.sh);
		    if (neighbor.tri != dummytri)
		    {
			/* Make sure the shell edge doesn't get deallocated again */
			/*   later when the infected neighbor is visited.         */
			uninfect(neighbor);
			tsdissolve(neighbor);
			infect(neighbor);
		    }
		}
	    }
	    else
	    {                   /* The neighbor exists and is not infected. */
		if (neighborshelle.sh == dummysh)
		{
		    /* There is no shell edge protecting the neighbor, so */
		    /*   the neighbor becomes infected.                   */
		    infect(neighbor);
		    /* Ensure that the neighbor's neighbors will be infected. */
		    deadtri = (triangle **) poolalloc(&viri);
		    *deadtri = neighbor.tri;
		}
		else
		{               /* The neighbor is protected by a shell edge. */
		    /* Remove this triangle from the shell edge. */
		    stdissolve(neighborshelle);
		    /* The shell edge becomes a boundary.  Set markers accordingly. */
		    if (mark(neighborshelle) == 0)
		    {
			setmark(neighborshelle, 1);
		    }
		    org(neighbor, norg);
		    dest(neighbor, ndest);
		    if (pointmark(norg) == 0)
		    {
			setpointmark(norg, 1);
		    }
		    if (pointmark(ndest) == 0)
		    {
			setpointmark(ndest, 1);
		    }
		}
	    }
	}
	/* Remark the triangle as infected, so it doesn't get added to the */
	/*   virus pool again.                                             */
	infect(testtri);
	virusloop = (triangle **) traverse(&viri);
    }

    traversalinit(&viri);
    virusloop = (triangle **) traverse(&viri);
    while (virusloop != NULL)
    {
	testtri.tri = *virusloop;

	/* Check each of the three corners of the triangle for elimination. */
	/*   This is done by walking around each point, checking if it is   */
	/*   still connected to at least one live triangle.                 */
	for (testtri.orient = 0; testtri.orient < 3; testtri.orient++)
	{
	    org(testtri, testpoint);
	    /* Check if the point has already been tested. */
	    if (testpoint != NULL)
	    {
		killorg = 1;
		/* Mark the corner of the triangle as having been tested. */
		setorg(testtri, NULL);
		/* Walk counterclockwise about the point. */
		onext(testtri, neighbor);
		/* Stop upon reaching a boundary or the starting triangle. */
		while ((neighbor.tri != dummytri)
		    && (!triedgeequal(neighbor, testtri)))
		{
		    if (infected(neighbor))
		    {
			/* Mark the corner of this triangle as having been tested. */
			setorg(neighbor, NULL);
		    }
		    else
		    {
			/* A live triangle.  The point survives. */
			killorg = 0;
		    }
		    /* Walk counterclockwise about the point. */
		    onextself(neighbor);
		}
		/* If we reached a boundary, we must walk clockwise as well. */
		if (neighbor.tri == dummytri)
		{
		    /* Walk clockwise about the point. */
		    oprev(testtri, neighbor);
		    /* Stop upon reaching a boundary. */
		    while (neighbor.tri != dummytri)
		    {
			if (infected(neighbor))
			{
			    /* Mark the corner of this triangle as having been tested. */
			    setorg(neighbor, NULL);
			}
			else
			{
			    /* A live triangle.  The point survives. */
			    killorg = 0;
			}
			/* Walk clockwise about the point. */
			oprevself(neighbor);
		    }
		}
		if (killorg)
		{
		    pointdealloc(testpoint);
		}
	    }
	}

	/* Record changes in the number of boundary edges, and disconnect */
	/*   dead triangles from their neighbors.                         */
	for (testtri.orient = 0; testtri.orient < 3; testtri.orient++)
	{
	    sym(testtri, neighbor);
	    if (neighbor.tri == dummytri)
	    {
		/* There is no neighboring triangle on this edge, so this edge    */
		/*   is a boundary edge.  This triangle is being deleted, so this */
		/*   boundary edge is deleted.                                    */
		hullsize--;
	    }
	    else
	    {
		/* Disconnect the triangle from its neighbor. */
		dissolve(neighbor);
		/* There is a neighboring triangle on this edge, so this edge */
		/*   becomes a boundary edge when this triangle is deleted.   */
		hullsize++;
	    }
	}
	/* Return the dead triangle to the pool of triangles. */
	triangledealloc(testtri.tri);
	virusloop = (triangle **) traverse(&viri);
    }
    /* Empty the virus pool. */
    poolrestart(&viri);
}		/*end plague*/

/*****************************************************************************/
/*                                                                           */
/*  regionplague()   Spread regional attributes and/or area constraints      */
/*                   (from a .poly file) throughout the mesh.                */
/*                                                                           */
/*  This procedure operates in two phases.  The first phase spreads an       */
/*  attribute and/or an area constraint through a (segment-bounded) region.  */
/*  The triangles are marked to ensure that each triangle is added to the    */
/*  virus pool only once, so the procedure will terminate.                   */
/*                                                                           */
/*  The second phase uninfects all infected triangles, returning them to     */
/*  normal.                                                                  */
/*                                                                           */
/*****************************************************************************/

LOCAL void regionplague(
    double attribute,
    double area)
{
    triedge testtri;
    triedge neighbor;
    triangle **virusloop;
    triangle **regiontri;
    edge neighborshelle;
    triangle ptr;            /* Temporary variable used by sym() and onext(). */
    shelle sptr;                     /* Temporary variable used by tspivot(). */

    /* Loop through all the infected triangles, spreading the attribute      */
    /*   and/or area constraint to their neighbors, then to their neighbors' */
    /*   neighbors.                                                          */
    traversalinit(&viri);
    virusloop = (triangle **) traverse(&viri);
    while (virusloop != NULL)
    {
	testtri.tri = *virusloop;
	/* A triangle is marked as infected by messing with one of its shell */
	/*   edges, setting it to an illegal value.  Hence, we have to       */
	/*   temporarily uninfect this triangle so that we can examine its   */
	/*   adjacent shell edges.                                           */
	uninfect(testtri);
	if (TriOpts.regionattrib)
	{
	    /* Set an attribute. */
	    setelemattribute(testtri, eextras, attribute);
	}
	if (TriOpts.vararea)
	{
	    /* Set an area constraint. */
	    setareabound(testtri, area);
	}
	/* Check each of the triangle's three neighbors. */
	for (testtri.orient = 0; testtri.orient < 3; testtri.orient++)
	{
	    /* Find the neighbor. */
	    sym(testtri, neighbor);
	    /* Check for a shell between the triangle and its neighbor. */
	    tspivot(testtri, neighborshelle);
	    /* Make sure the neighbor exists, is not already infected, and */
	    /*   isn't protected by a shell edge.                          */
	    if ((neighbor.tri != dummytri) && !infected(neighbor)
	        && (neighborshelle.sh == dummysh))
	    {
		/* Infect the neighbor. */
		infect(neighbor);
		/* Ensure that the neighbor's neighbors will be infected. */
		regiontri = (triangle **) poolalloc(&viri);
		*regiontri = neighbor.tri;
	    }
	}
	/* Remark the triangle as infected, so it doesn't get added to the */
	/*   virus pool again.                                             */
	infect(testtri);
	virusloop = (triangle **) traverse(&viri);
    }

    /* Uninfect all triangles. */
    traversalinit(&viri);
    virusloop = (triangle **) traverse(&viri);
    while (virusloop != NULL)
    {
	testtri.tri = *virusloop;
	uninfect(testtri);
	virusloop = (triangle **) traverse(&viri);
    }
    /* Empty the virus pool. */
    poolrestart(&viri);
}		/*end regionplague*/

/*****************************************************************************/
/*                                                                           */
/*  carveholes()   Find the holes and infect them.  Find the area            */
/*                 constraints and infect them.  Infect the convex hull.     */
/*                 Spread the infection and kill triangles.  Spread the      */
/*                 area constraints.                                         */
/*                                                                           */
/*  This routine mainly calls other routines to carry out all these          */
/*  functions.                                                               */
/*                                                                           */
/*****************************************************************************/

LOCAL void carveholes(
    double *holelist,
    int   holes,
    double *regionlist,
    int   regions)
{
    static triedge* regiontris = NULL;
    triedge searchtri;
    triedge triangleloop;
    triangle **holetri;
    triangle **regiontri;
    point searchorg, searchdest;
    locateresult intersect;
    int i;
    triangle ptr;                        /* Temporary variable used by sym(). */

    if (regions > 0)
    {
        static size_t   size_regiontris;
        regiontris = (triedge *) allocate_storage(regiontris,&size_regiontris,
					          regions*sizeof(triedge));
    }

    if (((holes > 0) && !TriOpts.noholes) || !TriOpts.convex || (regions > 0))
    {
	/* Initialize a pool of viri to be used for holes, concavities, */
	/*   regional attributes, and/or regional area constraints.     */
	poolinit(&viri,sizeof(triangle *),VIRUSPERBLOCK,sizeof(void*));
    }

    if (!TriOpts.convex)
    {
	/* Mark as infected any unprotected triangles on the boundary. */
	/*   This is one way by which concavities are created.         */
	infecthull();
    }

    if ((holes > 0) && !TriOpts.noholes)
    {
	/* Infect each triangle in which a hole lies. */
	for (i = 0; i < 2 * holes; i += 2)
	{
	    /* Ignore holes that aren't within the bounds of the mesh. */
	    if ((holelist[i] >= xmin) && (holelist[i] <= xmax)
	        && (holelist[i + 1] >= ymin) && (holelist[i + 1] <= ymax))
	    {
		/* Start searching from some triangle on the outer boundary. */
		searchtri.tri = dummytri;
		searchtri.orient = 0;
		symself(searchtri);
		/* Ensure that the hole is to the left of this boundary edge; */
		/*   otherwise, locate() will FALSEly report that the hole    */
		/*   falls within the starting triangle.                      */
		org(searchtri, searchorg);
		dest(searchtri, searchdest);
		if (counterclockwise(searchorg, searchdest, &holelist[i]) > 0.0)
		{
		    /* Find a triangle that contains the hole. */
		    intersect = locate(&holelist[i], &searchtri);
		    if ((intersect != OUTSIDE) && (!infected(searchtri)))
		    {
			/* Infect the triangle.  This is done by marking the triangle */
			/*   as infect and including the triangle in the virus pool.  */
			infect(searchtri);
			holetri = (triangle **) poolalloc(&viri);
			*holetri = searchtri.tri;
		    }
		}
	    }
	}
    }

    /* Now, we have to find all the regions BEFORE we carve the holes,     */
    /*   because locate() won't work when the triangulation is no longer   */
    /*   convex. (Incidentally, this is the reason why regional attributes */
    /*   and area constraints can't be used when refining a preexisting    */
    /*   mesh, which might not be convex; they can only be used with a     */
    /*   freshly triangulated PSLG.)                                       */
    if (regions > 0)
    {
	/* Find the starting triangle for each region. */
	for (i = 0; i < regions; i++)
	{
	    regiontris[i].tri = dummytri;
	    /* Ignore region points that aren't within the bounds of the mesh. */
	    if ((regionlist[4 * i] >= xmin) && (regionlist[4 * i] <= xmax) &&
	        (regionlist[4 * i + 1] >= ymin) && (regionlist[4 * i + 1] <= ymax))
	    {
		/* Start searching from some triangle on the outer boundary. */
		searchtri.tri = dummytri;
		searchtri.orient = 0;
		symself(searchtri);
		/* Ensure that the region point is to the left of this boundary */
		/*   edge; otherwise, locate() will FALSEly report that the     */
		/*   region point falls within the starting triangle.           */
		org(searchtri, searchorg);
		dest(searchtri, searchdest);
		if (counterclockwise(searchorg, searchdest, &regionlist[4 * i]) >
		    0.0)
		    {
		    /* Find a triangle that contains the region point. */
		    intersect = locate(&regionlist[4 * i], &searchtri);
		    if ((intersect != OUTSIDE) && (!infected(searchtri))) {
			/* Record the triangle for processing after the */
			/*   holes have been carved.                    */
			triedgecopy(searchtri, regiontris[i]);
		    }
		}
	    }
	}
    }

    if (viri.items > 0)
    {
	/* Carve the holes and concavities. */
	plague();
    }
    /* The virus pool should be empty now. */

    if (regions > 0)
    {
	if (TriOpts.regionattrib && !TriOpts.refine)
	{
	    /* Assign every triangle a regional attribute of zero. */
	    traversalinit(&triangles);
	    triangleloop.orient = 0;
	    triangleloop.tri = triangletraverse();
	    while (triangleloop.tri != NULL)
	    {
		setelemattribute(triangleloop, eextras, 0.0);
		triangleloop.tri = triangletraverse();
	    }
	}
	for (i = 0; i < regions; i++)
	{
	    if (regiontris[i].tri != dummytri)
	    {
		/* Make sure the triangle under consideration still exists. */
		/*   It may have been eaten by the virus.                   */
		if (regiontris[i].tri[3] != NULL)
		{
		    /* Put one triangle in the virus pool. */
		    infect(regiontris[i]);
		    regiontri = (triangle **) poolalloc(&viri);
		    *regiontri = regiontris[i].tri;
		    /* Apply one region's attribute and/or area constraint. */
		    regionplague(regionlist[4 * i + 2], regionlist[4 * i + 3]);
		    /* The virus pool should be empty now. */
		}
	    }
	}
	if (TriOpts.regionattrib && !TriOpts.refine)
	{
	    /* Note the fact that each triangle has an additional attribute. */
	    eextras++;
	}
    }

    /* Free up memory. */
    if (((holes > 0) && !TriOpts.noholes) || !TriOpts.convex || (regions > 0))
	pooldeinit(&viri);
}		/*end carveholes*/

/*****************************************************************************/
/*                                                                           */
/*  findcircumcenter()   Find the circumcenter of a triangle.                */
/*                                                                           */
/*  The result is returned both in terms of x-y coordinates and xi-eta       */
/*  coordinates.  The xi-eta coordinate system is defined in terms of the    */
/*  triangle:  the origin of the triangle is the origin of the coordinate    */
/*  system; the destination of the triangle is one unit along the xi axis;   */
/*  and the apex of the triangle is one unit along the eta axis.             */
/*                                                                           */
/*  The return value indicates which edge of the triangle is shortest.       */
/*                                                                           */
/*****************************************************************************/

LOCAL circumcenterresult findcircumcenter(
    point torg,
    point tdest,
    point tapex,
    point circumcenter,
    double *xi,
    double *eta)
{
    double xdo, ydo, xao, yao, xad, yad;
    double dodist, aodist, addist;
    double denominator;
    double dx, dy;

    circumcentercount++;

    /* Compute the circumcenter of the triangle. */
    xdo = tdest[0] - torg[0];
    ydo = tdest[1] - torg[1];
    xao = tapex[0] - torg[0];
    yao = tapex[1] - torg[1];
    dodist = xdo * xdo + ydo * ydo;
    aodist = xao * xao + yao * yao;
    if (TriOpts.noexact)
	denominator = 0.5 / (xdo * yao - xao * ydo);
    else
    {
	/* Use the counterclockwise() routine to ensure a positive (and */
	/*   reasonably accurate) result, avoiding any possibility of   */
	/*   division by zero.                                          */
	denominator = 0.5 / counterclockwise(tdest, tapex, torg);
	/* Don't count the above as an orientation test. */
	counterclockcount--;
    }
    circumcenter[0] = torg[0] - (ydo * aodist - yao * dodist) * denominator;
    circumcenter[1] = torg[1] + (xdo * aodist - xao * dodist) * denominator;

    /* To interpolate point attributes for the new point inserted at  */
    /*   the circumcenter, define a coordinate system with a xi-axis, */
    /*   directed from the triangle's origin to its destination, and  */
    /*   an eta-axis, directed from its origin to its apex.           */
    /*   Calculate the xi and eta coordinates of the circumcenter.    */
    dx = circumcenter[0] - torg[0];
    dy = circumcenter[1] - torg[1];
    *xi = (dx * yao - xao * dy) * (2.0 * denominator);
    *eta = (xdo * dy - dx * ydo) * (2.0 * denominator);

    xad = tapex[0] - tdest[0];
    yad = tapex[1] - tdest[1];
    addist = xad * xad + yad * yad;
    if ((addist < dodist) && (addist < aodist))
	return OPPOSITEORG;
    else if (dodist < aodist)
	return OPPOSITEAPEX;
    else
	return OPPOSITEDEST;
}		/*end findcircumcenter*/

/**                                                                         **/
/**                                                                         **/
/********* Carving out holes and concavities ends here               *********/

/********* Mesh quality maintenance begins here                      *********/
/**                                                                         **/
/**                                                                         **/

/*****************************************************************************/
/*                                                                           */
/*  tallyencs()   Traverse the entire list of shell edges, check each edge   */
/*                to see if it is encroached.  If so, add it to the list.    */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  precisionerror()  Print an error message for precision problems.         */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  repairencs()   Find and repair all the encroached segments.              */
/*                                                                           */
/*  Encroached segments are repaired by splitting them by inserting a point  */
/*  at or near their centers.                                                */
/*                                                                           */
/*  `flaws' is a flag that specifies whether one should take note of new     */
/*  encroached segments and bad triangles that result from inserting points  */
/*  to repair existing encroached segments.                                  */
/*                                                                           */
/*  When a segment is split, the two resulting subsegments are always        */
/*  tested to see if they are encroached upon, regardless of the value       */
/*  of `flaws'.                                                              */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  splittriangle()   Inserts a point at the circumcenter of a triangle.     */
/*                    Deletes the newly inserted point if it encroaches upon */
/*                    a segment.                                             */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  enforcequality()   Remove all the encroached edges and bad triangles     */
/*                     from the triangulation.                               */
/*                                                                           */
/*****************************************************************************/


/**                                                                         **/
/**                                                                         **/
/********* Mesh quality maintenance ends here                        *********/

/*****************************************************************************/
/*                                                                           */
/*  highorder()   Create extra nodes for quadratic subparametric elements.   */
/*                                                                           */
/*****************************************************************************/

LOCAL void highorder(void)
{
    triedge triangleloop, trisym;
    edge checkmark;
    point newpoint;
    point torg, tdest;
    int i;
    triangle ptr;                  /* Temporary variable used by sym(). */
    shelle sptr;                   /* Temporary variable used by tspivot(). */

    /* The following line ensures that dead items in the pool of nodes    */
    /*   cannot be allocated for the extra nodes associated with high     */
    /*   order elements.  This ensures that the primary nodes (at the     */
    /*   corners of elements) will occur earlier in the output files, and */
    /*   have lower indices, than the extra nodes.                        */
    points.deaditemstack = NULL;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    /* To loop over the set of edges, loop over all triangles, and look at   */
    /*   the three edges of each triangle.  If there isn't another triangle  */
    /*   adjacent to the edge, operate on the edge.  If there is another     */
    /*   adjacent triangle, operate on the edge only if the current triangle */
    /*   has a smaller pointer than its neighbor.  This way, each edge is    */
    /*   considered only once.                                               */
    while (triangleloop.tri != NULL)
    {
	for (triangleloop.orient = 0; triangleloop.orient < 3;
	    triangleloop.orient++)
	    {
	    sym(triangleloop, trisym);
	    if ((triangleloop.tri < trisym.tri) || (trisym.tri == dummytri))
	    {
		org(triangleloop, torg);
		dest(triangleloop, tdest);
		/* Create a new node in the middle of the edge.  Interpolate */
		/*   its attributes.                                         */
		newpoint = (point) poolalloc(&points);
		for (i = 0; i < 2 + nextras; i++)
		{
		    newpoint[i] = 0.5 * (torg[i] + tdest[i]);
		}
		/* Set the new node's marker to zero or one, depending on */
		/*   whether it lies on a boundary.                       */
		setpointmark(newpoint, trisym.tri == dummytri);
		if (useshelles)
		{
		    tspivot(triangleloop, checkmark);
		    /* If this edge is a segment, transfer the marker     */
		    /*   to the new node. */
		    if (checkmark.sh != dummysh)
		    {
			setpointmark(newpoint, mark(checkmark));
		    }
		}
		/* Record the new node in the (one or two) adjacent elements. */
		triangleloop.tri[highorderindex + triangleloop.orient] =
		    (triangle) newpoint;
		if (trisym.tri != dummytri)
		{
		    trisym.tri[highorderindex + trisym.orient] =
			(triangle) newpoint;
		}
	    }
	}
	triangleloop.tri = triangletraverse();
    }
}		/*end highorder*/

/*****************************************************************************/
/*                                                                           */
/*  transfernodes()   Read the points from memory.                           */
/*                                                                           */
/*****************************************************************************/


LOCAL void transfernodes(
    double  *pointlist,
    double  *pointattriblist,
    int    *pointmarkerlist,
    size_t numberofpoints,
    int    numberofpointattribs)
{
    point pointloop;
    double x, y;
    int i, j;
    int coordindex;
    int attribindex;

    inpoints = numberofpoints;
    mesh_dim = 2;
    nextras = numberofpointattribs;
    readnodefile = 0;
    if (inpoints < 3)
    {
	screen("ERROR in transfernodes(), "
	       "Input must have at least three input points.\n");
	(void) printf("inpoints = %lu\n",inpoints);
	clean_up(ERROR);
    }

    initializepointpool();

    /* Read the points. */
    coordindex = 0;
    attribindex = 0;
    for (i = 0; i < inpoints; i++)
    {
	pointloop = (point) poolalloc(&points);
	/* Read the point coordinates. */
	x = pointloop[0] = pointlist[coordindex++];
	y = pointloop[1] = pointlist[coordindex++];
	/* Read the point attributes. */
	for (j = 0; j < numberofpointattribs; j++)
	{
	    pointloop[2 + j] = pointattriblist[attribindex++];
	}
	if (pointmarkerlist != NULL)
	{
	    /* Read a point marker. */
	    setpointmark(pointloop, pointmarkerlist[i]);
	}
	else
	{
	    /* If no markers are specified, they default to zero. */
	    setpointmark(pointloop, 0);
	}
	x = pointloop[0];
	y = pointloop[1];
	/* Determine the smallest and largest x and y coordinates. */
	if (i == 0)
	{
	    xmin = xmax = x;
	    ymin = ymax = y;
	}
	else
	{
	    xmin = (x < xmin) ? x : xmin;
	    xmax = (x > xmax) ? x : xmax;
	    ymin = (y < ymin) ? y : ymin;
	    ymax = (y > ymax) ? y : ymax;
	}
    }
}		/*end transfernodes*/


LOCAL void* allocate_storage(
    void   *storage,
    size_t *storage_size,
    size_t new_size)
{
    if (new_size > *storage_size)
    {
	if (storage != NULL)
	    free(storage);
	*storage_size = new_size;
	scalar(&storage,new_size);
	if (storage == NULL)
	{
	    screen("ERROR in allocate_storage(),  Out of memory.\n");
	    clean_up(ERROR);
	}
    }
    return storage;
}		/*end allocate_storage*/

/*****************************************************************************/
/*                                                                           */
/*  writenodes()   Number the points and write them to a .node file.         */
/*                                                                           */
/*  To save memory, the point numbers are written over the shell markers     */
/*  after the points are written to a file.                                  */
/*                                                                           */
/*****************************************************************************/

LOCAL void writenodes(
    triangulateio* out)
{
    double *plist;
    double *palist;
    int *pmlist;
    int coordindex;
    int attribindex;
    point pointloop;
    int pointnumber;
    int i;

    /* Allocate memory for output points if necessary. */
    out->pointlist = (double *) allocate_storage(out->pointlist,
					       &out->size_pointlist,
					       points.items*2*FLOAT);
    /* Allocate memory for output point attributes if necessary. */
    if (nextras > 0)
    {
        out->pointattributelist =
	    (double *) allocate_storage(out->pointattributelist,
				      &out->size_pointattributelist,
				      points.items * nextras * FLOAT);
    }
    /* Allocate memory for output point markers if necessary. */
    if (!TriOpts.nobound)
    {
        out->pointmarkerlist =
	    (int *) allocate_storage(out->pointmarkerlist,
				     &out->size_pointmarkerlist,
				     points.items*INT);
    }
    plist = out->pointlist;
    palist = out->pointattributelist;
    pmlist = out->pointmarkerlist;
    coordindex = 0;
    attribindex = 0;

    traversalinit(&points);
    pointloop = pointtraverse();
    pointnumber = TriOpts.firstnumber;
    while (pointloop != NULL)
    {
	/* X and y coordinates. */
	plist[coordindex++] = pointloop[0];
	plist[coordindex++] = pointloop[1];
	/* Point attributes. */
	for (i = 0; i < nextras; i++)
	{
	    palist[attribindex++] = pointloop[2 + i];
	}
	if (!TriOpts.nobound)
	{
	    /* Copy the boundary marker. */
	    pmlist[pointnumber - TriOpts.firstnumber] = pointmark(pointloop);
	}

	setpointmark(pointloop, pointnumber);
	pointloop = pointtraverse();
	pointnumber++;
    }

}		/*end writenodes*/

/*****************************************************************************/
/*                                                                           */
/*  numbernodes()   Number the points.                                       */
/*                                                                           */
/*  Each point is ft_assigned a marker equal to its number.                     */
/*                                                                           */
/*  Used when writenodes() is not called because no .node file is written.   */
/*                                                                           */
/*****************************************************************************/

LOCAL void numbernodes(void)
{
    point pointloop;
    int pointnumber;

    traversalinit(&points);
    pointloop = pointtraverse();
    pointnumber = TriOpts.firstnumber;
    while (pointloop != NULL)
    {
	setpointmark(pointloop, pointnumber);
	pointloop = pointtraverse();
	pointnumber++;
    }
}		/*end numbernodes*/

/*****************************************************************************/
/*                                                                           */
/*  writeelements()   Write the triangles to an .ele file.                   */
/*                                                                           */
/*****************************************************************************/


LOCAL void writeelements(
    triangulateio* out)
{
    int *tlist;
    double *talist;
    int pointindex;
    int attribindex;
    triedge triangleloop;
    point p1, p2, p3;
    point mid1, mid2, mid3;
    int elementnumber;
    int i;

    /* Allocate memory for output triangles if necessary. */
    out->trianglelist =
	(int *) allocate_storage(out->trianglelist,&out->size_trianglelist,
				 triangles.items*((order+1)*(order+2)/2)*INT);
    tlist = out->trianglelist;
    /* Allocate memory for output triangle attributes if necessary. */
    if (eextras > 0)
    {
        out->triangleattributelist =
	    (double *) allocate_storage(out->triangleattributelist,
				      &out->size_triangleattributelist,
				      triangles.items*eextras*FLOAT);
    }
    talist = out->triangleattributelist;
    pointindex = 0;
    attribindex = 0;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    triangleloop.orient = 0;
    elementnumber = TriOpts.firstnumber;
    while (triangleloop.tri != NULL)
    {
	org(triangleloop, p1);
	dest(triangleloop, p2);
	apex(triangleloop, p3);
	if (order == 1)
	{
	    tlist[pointindex++] = pointmark(p1);
	    tlist[pointindex++] = pointmark(p2);
	    tlist[pointindex++] = pointmark(p3);
	}
	else
	{
	    mid1 = (point) triangleloop.tri[highorderindex + 1];
	    mid2 = (point) triangleloop.tri[highorderindex + 2];
	    mid3 = (point) triangleloop.tri[highorderindex];
	    tlist[pointindex++] = pointmark(p1);
	    tlist[pointindex++] = pointmark(p2);
	    tlist[pointindex++] = pointmark(p3);
	    tlist[pointindex++] = pointmark(mid1);
	    tlist[pointindex++] = pointmark(mid2);
	    tlist[pointindex++] = pointmark(mid3);
	}

	for (i = 0; i < eextras; i++)
	{
	    talist[attribindex++] = elemattribute(triangleloop, i);
	}

	triangleloop.tri = triangletraverse();
	elementnumber++;
    }

}		/*end writeelements*/

/*****************************************************************************/
/*                                                                           */
/*  writepoly()   Write the segments and holes to a .poly file.              */
/*                                                                           */
/*****************************************************************************/


LOCAL void writepoly(
    triangulateio* out)
{
    int *slist;
    int *smlist;
    int index;
    edge shelleloop;
    point endpoint1, endpoint2;
    int shellenumber;

    /* Allocate memory for output segments if necessary. */
    out->segmentlist = (int *) allocate_storage(out->segmentlist,
	                                        &out->size_segmentlist,
						shelles.items*2*INT);
    /* Allocate memory for output segment markers if necessary. */
    if (!TriOpts.nobound)
    {
        out->segmentmarkerlist =
	    (int *) allocate_storage(out->segmentmarkerlist,
				     &out->size_segmentmarkerlist,
				     shelles.items*INT);
    }
    slist = out->segmentlist;
    smlist = out->segmentmarkerlist;
    index = 0;

    traversalinit(&shelles);
    shelleloop.sh = shelletraverse();
    shelleloop.shorient = 0;
    shellenumber = TriOpts.firstnumber;
    while (shelleloop.sh != NULL)
    {
	sorg(shelleloop, endpoint1);
	sdest(shelleloop, endpoint2);
	/* Copy indices of the segment's two endpoints. */
	slist[index++] = pointmark(endpoint1);
	slist[index++] = pointmark(endpoint2);
	if (!TriOpts.nobound)
	{
	    /* Copy the boundary marker. */
	    smlist[shellenumber - TriOpts.firstnumber] = mark(shelleloop);
	}

	shelleloop.sh = shelletraverse();
	shellenumber++;
    }

}		/*end writepoly*/

/*****************************************************************************/
/*                                                                           */
/*  writeedges()   Write the edges to a .edge file.                          */
/*                                                                           */
/*****************************************************************************/


LOCAL void writeedges(
    triangulateio *out)
{
    int *elist;
    int *emlist;
    int index;
    triedge triangleloop, trisym;
    edge checkmark;
    point p1, p2;
    int edgenumber;
    triangle ptr;                        /* Temporary variable used by sym(). */
    shelle sptr;                     /* Temporary variable used by tspivot(). */

    /* Allocate memory for edges if necessary. */
    out->edgelist = (int *) allocate_storage(out->edgelist,&out->size_edgelist,
					     edges*2*INT);
    /* Allocate memory for edge markers if necessary. */
    if (!TriOpts.nobound)
    {
        out->edgemarkerlist =
	    (int *) allocate_storage(out->edgemarkerlist,
				     &out->size_edgemarkerlist,
				     edges*INT);
    }
    elist = out->edgelist;
    emlist = out->edgemarkerlist;
    index = 0;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    edgenumber = TriOpts.firstnumber;
    /* To loop over the set of edges, loop over all triangles, and look at   */
    /*   the three edges of each triangle.  If there isn't another triangle  */
    /*   adjacent to the edge, operate on the edge.  If there is another     */
    /*   adjacent triangle, operate on the edge only if the current triangle */
    /*   has a smaller pointer than its neighbor.  This way, each edge is    */
    /*   considered only once.                                               */
    while (triangleloop.tri != NULL)
    {
	for (triangleloop.orient = 0; triangleloop.orient < 3;
	    triangleloop.orient++)
	    {
	    sym(triangleloop, trisym);
	    if ((triangleloop.tri < trisym.tri) || (trisym.tri == dummytri))
	    {
		org(triangleloop, p1);
		dest(triangleloop, p2);
		elist[index++] = pointmark(p1);
		elist[index++] = pointmark(p2);
		if (! TriOpts.nobound)
		{
		    /*
		     * Edge number, indices of two endpoints, and a
		     * boundary marker. If there's no shell edge, the
		     * boundary marker is zero.
		     */
		    if (useshelles)
		    {
			tspivot(triangleloop, checkmark);
			if (checkmark.sh == dummysh)
			{
			    emlist[edgenumber - TriOpts.firstnumber] = 0;
			}
			else
			{
			    emlist[edgenumber - TriOpts.firstnumber] =
				mark(checkmark);
			}
		    }
		    else
		    {
			emlist[edgenumber - TriOpts.firstnumber] =
			    trisym.tri == dummytri;
		    }
		}
		edgenumber++;
	    }
	}
	triangleloop.tri = triangletraverse();
    }

}		/*end writeedges*/

/*****************************************************************************/
/*                                                                           */
/*  writevoronoi()   Write the Voronoi diagram to a .v.node and .v.edge      */
/*                   file.                                                   */
/*                                                                           */
/*  The Voronoi diagram is the geometric dual of the Delaunay triangulation. */
/*  Hence, the Voronoi vertices are listed by traversing the Delaunay        */
/*  triangles, and the Voronoi edges are listed by traversing the Delaunay   */
/*  edges.                                                                   */
/*                                                                           */
/*  WARNING:  In order to assign numbers to the Voronoi vertices, this       */
/*  procedure messes up the shell edges or the extra nodes of every          */
/*  element.  Hence, you should call this procedure last.                    */
/*                                                                           */
/*****************************************************************************/

LOCAL void writevoronoi(
    triangulateio *vorout)
{
    double *plist;
    double *palist;
    int *elist;
    double *normlist;
    int coordindex;
    int attribindex;
    triedge triangleloop, trisym;
    point torg, tdest, tapex;
    double circumcenter[2];
    double xi, eta;
    int vnodenumber, vedgenumber;
    int p1, p2;
    int i;
    triangle ptr;                       /* Temporary variable used by sym(). */

    /* Allocate memory for Voronoi vertices if necessary. */
    vorout->pointlist =
	(double *) allocate_storage(vorout->pointlist,&vorout->size_pointlist,
			          triangles.items*2*FLOAT);
    /* Allocate memory for Voronoi vertex attributes if necessary. */
    vorout->pointattributelist =
	    (double *) allocate_storage(vorout->pointattributelist,
				      &vorout->size_pointattributelist,
				      triangles.items*nextras*FLOAT);
    vorout->pointmarkerlist = NULL;
    plist = vorout->pointlist;
    palist = vorout->pointattributelist;
    coordindex = 0;
    attribindex = 0;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    triangleloop.orient = 0;
    vnodenumber = TriOpts.firstnumber;
    while (triangleloop.tri != NULL)
    {
	org(triangleloop, torg);
	dest(triangleloop, tdest);
	apex(triangleloop, tapex);
	findcircumcenter(torg, tdest, tapex, circumcenter, &xi, &eta);
	/* X and y coordinates. */
	plist[coordindex++] = circumcenter[0];
	plist[coordindex++] = circumcenter[1];
	for (i = 2; i < 2 + nextras; i++)
	{
	    /* Interpolate the point attributes at the circumcenter. */
	    palist[attribindex++] = torg[i] + xi * (tdest[i] - torg[i])
	        + eta * (tapex[i] - torg[i]);
	}

	* (int *) (triangleloop.tri + 6) = vnodenumber;
	triangleloop.tri = triangletraverse();
	vnodenumber++;
    }

    /* Allocate memory for output Voronoi edges if necessary. */
    vorout->edgelist =
	(int *) allocate_storage(vorout->edgelist,&vorout->size_edgelist,
			          edges*2*INT);
    vorout->edgemarkerlist = NULL;
    /* Allocate memory for output Voronoi norms if necessary. */
    vorout->normlist =
	(double *) allocate_storage(vorout->normlist,&vorout->size_normlist,
			          edges*2*FLOAT);
    elist = vorout->edgelist;
    normlist = vorout->normlist;
    coordindex = 0;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    vedgenumber = TriOpts.firstnumber;
    /* To loop over the set of edges, loop over all triangles, and look at   */
    /*   the three edges of each triangle.  If there isn't another triangle  */
    /*   adjacent to the edge, operate on the edge.  If there is another     */
    /*   adjacent triangle, operate on the edge only if the current triangle */
    /*   has a smaller pointer than its neighbor.  This way, each edge is    */
    /*   considered only once.                                               */
    while (triangleloop.tri != NULL)
    {
	for (triangleloop.orient = 0; triangleloop.orient < 3;
	    triangleloop.orient++)
	    {
	    sym(triangleloop, trisym);
	    if ((triangleloop.tri < trisym.tri) || (trisym.tri == dummytri))
	    {
		/* Find the number of this triangle (and Voronoi vertex). */
		p1 = * (int *) (triangleloop.tri + 6);
		if (trisym.tri == dummytri)
		{
		    org(triangleloop, torg);
		    dest(triangleloop, tdest);
		    /* Copy an infinite ray.  Index of one endpoint, and -1. */
		    elist[coordindex] = p1;
		    normlist[coordindex++] = tdest[1] - torg[1];
		    elist[coordindex] = -1;
		    normlist[coordindex++] = torg[0] - tdest[0];
		}
		else
		{
		    /* Find the number of the adjacent triangle (and Voronoi vertex). */
		    p2 = * (int *) (trisym.tri + 6);
		    /* Finite edge.  Write indices of two endpoints. */
		    elist[coordindex] = p1;
		    normlist[coordindex++] = 0.0;
		    elist[coordindex] = p2;
		    normlist[coordindex++] = 0.0;
		}
		vedgenumber++;
	    }
	}
	triangleloop.tri = triangletraverse();
    }

}		/*end writevoronoi*/

LOCAL void writeneighbors(
    triangulateio *out)
{
    int *nlist;
    int index;
    triedge triangleloop, trisym;
    int elementnumber;
    int neighbor1, neighbor2, neighbor3;
    triangle ptr;                       /* Temporary variable used by sym(). */

    /* Allocate memory for neighbors if necessary. */
    out->neighborlist = (int *) allocate_storage(out->neighborlist,
						 &out->size_neighborlist,
						 triangles.items*3*INT);
    nlist = out->neighborlist;
    index = 0;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    triangleloop.orient = 0;
    elementnumber = TriOpts.firstnumber;
    while (triangleloop.tri != NULL)
    {
	* (int *) (triangleloop.tri + 6) = elementnumber;
	triangleloop.tri = triangletraverse();
	elementnumber++;
    }
    * (int *) (dummytri + 6) = -1;

    traversalinit(&triangles);
    triangleloop.tri = triangletraverse();
    elementnumber = TriOpts.firstnumber;
    while (triangleloop.tri != NULL)
    {
	triangleloop.orient = 1;
	sym(triangleloop, trisym);
	neighbor1 = * (int *) (trisym.tri + 6);
	triangleloop.orient = 2;
	sym(triangleloop, trisym);
	neighbor2 = * (int *) (trisym.tri + 6);
	triangleloop.orient = 0;
	sym(triangleloop, trisym);
	neighbor3 = * (int *) (trisym.tri + 6);
	nlist[index++] = neighbor1;
	nlist[index++] = neighbor2;
	nlist[index++] = neighbor3;

	triangleloop.tri = triangletraverse();
	elementnumber++;
    }

}		/*end writeneighbors*/

/*****************************************************************************/
/*                                                                           */
/*  triangulate()   Gosh, do everything.                                     */
/*                                                                           */
/*  The sequence is roughly as follows.  Many of these steps can be skipped, */
/*  depending on the command line switches.                                  */
/*                                                                           */
/*  - Initialize constants and parse the command line.                       */
/*  - Read the points from a file and either                                 */
/*    - triangulate them (no -r), or                                         */
/*    - read an old mesh from files and reconstruct it (-r).                 */
/*  - Insert the PSLG segments (-p), and possibly segments on the convex     */
/*      hull (-c).                                                           */
/*  - Read the holes (-p), regional attributes (-pA), and regional area      */
/*      constraints (-pa).  Carve the holes and concavities, and spread the  */
/*      regional attributes and area constraints.                            */
/*  - Enforce the constraints on minimum angle (-q) and maximum area (-a).   */
/*      Also enforce the conforming Delaunay property (-q and -a).           */
/*  - Compute the number of edges in the resulting mesh.                     */
/*  - Promote the mesh's linear triangles to higher order elements (-o).     */
/*  - Write the output files and print the statistics.                       */
/*  - Check the consistency and Delaunay property of the mesh (-C).          */
/*                                                                           */
/*****************************************************************************/


EXPORT void triangulate(
    triangulateio *in,
    triangulateio *out,
    triangulateio *vorout)
{
    double *holearray;                                      /* Array of holes. */
    double *regionarray; /* Array of regional attributes and area constraints. */
    /* Variables for timing the performance of Triangle.  The types are */
    /*   defined in sys/time.h.                                         */

    debug_print("triangul","Entered triangulate()\n");

    triangleinit();
    out->Opts = in->Opts;
    if (vorout != NULL)
	vorout->Opts = in->Opts;

    parsecommandline(&in->Opts);

    transfernodes(in->pointlist,in->pointattributelist,in->pointmarkerlist,
                  in->numberofpoints,in->numberofpointattributes);

    hullsize = delaunay();                       /* Triangulate the points. */

    /* Ensure that no point can be mistaken for a triangular bounding */
    /*   box point in insertsite().                                   */
    infpoint1 = NULL;
    infpoint2 = NULL;
    infpoint3 = NULL;

    if (useshelles)
    {
	checksegments = 1;              /* Segments will be introduced next. */
	if (!TriOpts.refine)
	{
	    /* Insert PSLG segments and/or convex hull segments. */
	    (void) formskeleton(in->segmentlist, in->segmentmarkerlist,
	                        in->numberofsegments);
	}
    }

    if (TriOpts.poly)
    {
	holearray = in->holelist;
	holes = in->numberofholes;
	regionarray = in->regionlist;
	regions = in->numberofregions;
	if (!TriOpts.refine)
	{
	    /* Carve out holes and concavities. */
	    carveholes(holearray, holes, regionarray, regions);
	}
    }
    else
    {
	/* Without a PSLG, there can be no holes or regional attributes   */
	/*   or area constraints.  The following are set to zero to avoid */
	/*   an accidental free() later.                                  */
	holes = 0;
	regions = 0;
    }

    /* Compute the number of edges. */
    edges = (3 * triangles.items + hullsize) / 2;

    if (order > 1)
	highorder();         /* Promote elements to higher polynomial order. */

    out->numberofpoints = points.items;
    out->numberofpointattributes = nextras;
    out->numberoftriangles = triangles.items;
    out->numberofcorners = (order+1)*(order+2)/2;
    out->numberoftriangleattributes = eextras;
    out->numberofedges = edges;
    if (useshelles)
    {
	out->numberofsegments = shelles.items;
    }
    else
    {
	out->numberofsegments = hullsize;
    }
    /* If not using iteration numbers, don't write a .node file if one was */
    /*   read, because the original one would be overwritten!              */
    if (TriOpts.nonodewritten || (TriOpts.noiterationnum && readnodefile))
	numbernodes();              /* We must remember to number the points. */
    else
	writenodes(out);
    if (!TriOpts.noelewritten)
	writeelements(out);
    /* The -c switch (convex switch) causes a PSLG to be written */
    /*   even if none was read.                                  */
    if (TriOpts.poly || TriOpts.convex)
    {
	/* If not using iteration numbers, don't overwrite the .poly file. */
	if (!TriOpts.nopolywritten && !TriOpts.noiterationnum)
	{
	    writepoly(out);
	    out->numberofholes = holes;
	    out->numberofregions = regions;
	    if (TriOpts.poly)
	    {
		out->holelist = in->holelist;
		out->regionlist = in->regionlist;
	    }
	    else
	    {
		out->holelist = NULL;
		out->regionlist = NULL;
	    }
	}
    }
    if (TriOpts.edgesout)
	writeedges(out);
    if (TriOpts.voronoi)
	writevoronoi(vorout);
    if (TriOpts.neighbors)
	writeneighbors(out);

    triangledeinit();

    if (debugging("triangul"))
    {
	(void) printf("\nInput triangulateio structure\n");
        print_triangulateio(in);

	(void) printf("\nOutput triangulateio structure\n");
        print_triangulateio(out);
    }
    debug_print("triangul","Left triangulate()\n");
}		/*end triangulate*/

EXPORT	void	print_triangulateio(
	triangulateio *tio)
{
	int i;

	(void) printf("triangulateio structure 0x%p\n",tio);
        (void) printf("\tnumberofpoints = %d\n",tio->numberofpoints);
        for (i = 0; i < tio->numberofpoints; ++i)
        {
	    (void) printf("\tpoint %d = %g %g\n",i,
		          tio->pointlist[2*i],tio->pointlist[2*i+1]);
        }
	if (tio->numberofsegments > 0)
	{
            (void) printf("\tnumberofsegments = %ld\n",tio->numberofsegments);
            for (i = 0; i < tio->numberofsegments; ++i)
            {
	        (void) printf("\tsegment %d = %d %d\n",i,
		              tio->segmentlist[2*i],tio->segmentlist[2*i+1]);
            }
	}
	if (tio->numberoftriangles > 0)
	{
            (void) printf("\tnumberoftriangles = %d\n",tio->numberoftriangles);
            for (i = 0; i < tio->numberoftriangles; ++i)
            {
	        (void) printf("\ttriangle %d = %d %d %d, "
	                      "neighbors = %d %d %d\n",i,
		              tio->trianglelist[3*i],
			      tio->trianglelist[3*i+1],
		              tio->trianglelist[3*i+2],
		              tio->neighborlist[3*i],
			      tio->neighborlist[3*i+1],
		              tio->neighborlist[3*i+2]);
            }
	}
	(void) printf("end triangulateio structure 0x%p\n",tio);
}		/*end print_triangulateio*/

