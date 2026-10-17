/*<html><pre>  -<a                             href="qh-io.htm"
  >-------------------------------</a><a name="TOP">-</a>

   io.h 
   declarations of Input/Output functions

   see README, qhull.h and io.c

   copyright (c) 1993-2003, The Geometry Center
*/

#ifndef qhDEFio
#define qhDEFio 1

/*============ constants and flags ==================*/

/*-<a                             href="qh-io.htm#TOC"
  >--------------------------------</a><a name="qh_MAXfirst">-</a>
  
  qh_MAXfirst
    maximum length of first two lines of stdin
*/
#define qh_MAXfirst  200

/*-<a                             href="qh-io.htm#TOC"
  >--------------------------------</a><a name="qh_MINradius">-</a>
  
  qh_MINradius
    min radius for Gp and Gv, fraction of maxcoord
*/
#define qh_MINradius 0.02

/*-<a                             href="qh-io.htm#TOC"
  >--------------------------------</a><a name="qh_GEOMepsilon">-</a>
  
  qh_GEOMepsilon
    adjust outer planes for 'lines closer' and geomview roundoff.  
    This prevents bleed through.
*/
#define qh_GEOMepsilon 2e-3

/*-<a                             href="qh-io.htm#TOC"
  >--------------------------------</a><a name="qh_WHITESPACE">-</a>
  
  qh_WHITESPACE
    possible values of white space
*/
#define qh_WHITESPACE " \n\t\v\r\f"


/*-<a                             href="qh-io.htm#TOC"
  >--------------------------------</a><a name="RIDGE">-</a>
  
  qh_RIDGE
    to select which ridges to print in qh_eachvoronoi
*/
typedef enum
{
    qh_RIDGEall = 0, qh_RIDGEinner, qh_RIDGEouter
}
qh_RIDGE;

/*-<a                             href="qh-io.htm#TOC"
  >--------------------------------</a><a name="printvridgeT">-</a>
  
  printvridgeT
    prints results of qh_printvdiagram

  see:
    <a href="io.c#printvridge">qh_printvridge</a> for an example
*/
typedef void (*printvridgeT)(ostream* fp, vertexT *vertex, vertexT *vertexA, setT *centers, boolT unbounded);

/*============== -prototypes in alphabetical order =========*/

void    dfacet( unsigned id);
void    dvertex( unsigned id);
int	qh_compare_facetarea(const void *p1, const void *p2);
int	qh_compare_facetmerge(const void *p1, const void *p2);
int	qh_compare_facetvisit(const void *p1, const void *p2);
int	qh_compare_vertexpoint(const void *p1, const void *p2); /* not used */

void    qh_countfacets (facetT *facetlist, setT *facets, boolT printall, 
              int *numfacetsp, int *numsimplicialp, int *totneighborsp, 
              int *numridgesp, int *numcoplanarsp, int *numnumtricoplanarsp);
pointT *qh_detvnorm (vertexT *vertex, vertexT *vertexA, setT *centers, realT *offsetp);
setT   *qh_detvridge (vertexT *vertex);
setT   *qh_detvridge3 (vertexT *atvertex, vertexT *vertex);
int     qh_eachvoronoi (ostream* fp, printvridgeT printvridge, vertexT *atvertex, boolT visitall, qh_RIDGE innerouter, boolT inorder);
int     qh_eachvoronoi_all (ostream* fp, printvridgeT printvridge, boolT isupper, qh_RIDGE innerouter, boolT inorder);
void	qh_facet2point(facetT *facet, pointT **point0, pointT **point1, realT *mindist);
setT   *qh_facetvertices (facetT *facetlist, setT *facets, boolT allfacets);
void    qh_geomplanes (facetT *facet, realT *outerplane, realT *innerplane);
void    qh_markkeep (facetT *facetlist);
setT   *qh_markvoronoi (facetT *facetlist, setT *facets, boolT printall, boolT *islowerp, int *numcentersp);
void    qh_order_vertexneighbors(vertexT *vertex);
void	qh_printafacet(ostream* fp, int format, facetT *facet, boolT printall);
void    qh_printbegin (ostream* fp, int format, facetT *facetlist, setT *facets, boolT printall);
void 	qh_printcenter (ostream* fp, int format, char *string, facetT *facet);
void    qh_printcentrum (ostream* fp, facetT *facet, realT radius);
void    qh_printend (ostream* fp, int format, facetT *facetlist, setT *facets, boolT printall);
void    qh_printend4geom (ostream* fp, facetT *facet, int *num, boolT printall);
void    qh_printextremes (ostream* fp, facetT *facetlist, setT *facets, int printall);
void    qh_printextremes_2d (ostream* fp, facetT *facetlist, setT *facets, int printall);
void    qh_printextremes_d (ostream* fp, facetT *facetlist, setT *facets, int printall);
void	qh_printfacet(ostream* fp, facetT *facet);
void	qh_printfacet2math(ostream* fp, facetT *facet, int format, int notfirst);
void	qh_printfacet2geom(ostream* fp, facetT *facet, realT color[3]);
void    qh_printfacet2geom_points(ostream* fp, pointT *point1, pointT *point2,
			       facetT *facet, realT offset, realT color[3]);
void	qh_printfacet3math (ostream* fp, facetT *facet, int format, int notfirst);
void	qh_printfacet3geom_nonsimplicial(ostream* fp, facetT *facet, realT color[3]);
void	qh_printfacet3geom_points(ostream* fp, setT *points, facetT *facet, realT offset, realT color[3]);
void	qh_printfacet3geom_simplicial(ostream* fp, facetT *facet, realT color[3]);
void	qh_printfacet3vertex(ostream* fp, facetT *facet, int format);
void	qh_printfacet4geom_nonsimplicial(ostream* fp, facetT *facet, realT color[3]);
void	qh_printfacet4geom_simplicial(ostream* fp, facetT *facet, realT color[3]);
void	qh_printfacetNvertex_nonsimplicial(ostream* fp, facetT *facet, int id, int format);
void	qh_printfacetNvertex_simplicial(ostream* fp, facetT *facet, int format);
void    qh_printfacetheader(ostream* fp, facetT *facet);
void    qh_printfacetridges(ostream* fp, facetT *facet);
void	qh_printfacets(ostream* fp, int format, facetT *facetlist, setT *facets, boolT printall);
void	qh_printhelp_degenerate(ostream* fp);
void	qh_printhelp_singular(ostream* fp);
void	qh_printhyperplaneintersection(ostream* fp, facetT *facet1, facetT *facet2,
  		   setT *vertices, realT color[3]);
void	qh_printneighborhood (ostream* fp, int format, facetT *facetA, facetT *facetB, boolT printall);
void    qh_printline3geom (ostream* fp, pointT *pointA, pointT *pointB, realT color[3]);
void	qh_printpoint(ostream* fp, char *string, pointT *point);
void	qh_printpointid(ostream* fp, char *string, int dim, pointT *point, int id);
void    qh_printpoint3 (ostream* fp, pointT *point);
void    qh_printpoints_out (ostream* fp, facetT *facetlist, setT *facets, int printall);
void    qh_printpointvect (ostream* fp, pointT *point, coordT *normal, pointT *center, realT radius, realT color[3]);
void    qh_printpointvect2 (ostream* fp, pointT *point, coordT *normal, pointT *center, realT radius);
void	qh_printridge(ostream* fp, ridgeT *ridge);
void    qh_printspheres(ostream* fp, setT *vertices, realT radius);
void    qh_printvdiagram (ostream* fp, int format, facetT *facetlist, setT *facets, boolT printall);
int     qh_printvdiagram2 (ostream* fp, printvridgeT printvridge, setT *vertices, qh_RIDGE innerouter, boolT inorder);
void	qh_printvertex(ostream* fp, vertexT *vertex);
void	qh_printvertexlist (ostream* fp, char* string, facetT *facetlist,
                         setT *facets, boolT printall);
void	qh_printvertices (ostream* fp, char* string, setT *vertices);
void    qh_printvneighbors (ostream* fp, facetT* facetlist, setT *facets, boolT printall);
void    qh_printvoronoi (ostream* fp, int format, facetT *facetlist, setT *facets, boolT printall);
void    qh_printvnorm (ostream* fp, vertexT *vertex, vertexT *vertexA, setT *centers, boolT unbounded);
void    qh_printvridge (ostream* fp, vertexT *vertex, vertexT *vertexA, setT *centers, boolT unbounded);
void	qh_produce_output(void);
void    qh_projectdim3 (pointT *source, pointT *destination);
int     qh_readfeasible (int dim, char *remainder);
coordT *qh_readpoints(int *numpoints, int *dimension, boolT *ismalloc);
void    qh_setfeasible (int dim);
boolT	qh_skipfacet(facetT *facet);

#endif /* qhDEFio */
