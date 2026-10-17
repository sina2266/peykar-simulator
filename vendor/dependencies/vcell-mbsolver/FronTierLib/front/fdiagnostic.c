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
*				fdiagnostic.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#if defined(THREED)

#include <front/fdecs.h>

LOCAL 	void 	summary_of_nodes(INTERFACE*);
LOCAL 	void	summary_of_curves(INTERFACE*);
LOCAL	void	summary_of_surfaces(INTERFACE*);


LOCAL  	void 	summary_of_nodes(
	INTERFACE *intfc)
{
    	int 	  i, j;
	NODE 	  **n;
	CURVE     **c;

	(void) printf("NOD | bdry   obj 0x    hsb 0x   intfc 0x       "
		      "x              y         ");
	if (intfc->dim == 3)
	    (void) printf("      z           curves\n");
	else
	    (void) printf("  curves\n");

	(void) printf("===   ====  ========  ========  ========  "
		      "=============  =============  ");
	if (intfc->dim == 3)
	    (void) printf("=============  ==========\n");
	else
	    (void) printf(" ==========\n");
	
	for (i = 0, n = intfc->nodes; n && *n; n++, i++)
	{
	    (void) printf("%3d | %3d   %llu  %llu  %llu",
			  i,Boundary(*n),node_number((*n)->obj),
			  hypersurface_boundary_number(Hyper_surf_bdry(*n)),
			  interface_number(intfc));

	    for (j = 0; j < intfc->dim; j++) 
	        (void) printf("  %g",Coords((*n)->posn)[j]); 

	    (void) printf("   in[");
	    for (c = (*n)->in_curves; c && *c; c++)
	        (void) printf(" %d",index_of_pointer((POINTER*)intfc->curves,
						     (POINTER)*c));
	    (void) printf(" ] out[");
	    for (c = (*n)->out_curves; c && *c; c++)
	        (void) printf(" %d",index_of_pointer((POINTER*)intfc->curves,
						     (POINTER)*c));
	    (void) printf(" ]\n");
	}
	fflush(stdout);
} 		/*end summary_of_nodes*/

LOCAL	void 	summary_of_curves(
	INTERFACE *intfc)
{
    	int 	i;
	CURVE 	**c;
	SURFACE **s;
	int  	dim;

	dim = intfc->dim;
	(void) printf("CUR | bdry   obj 0x %10s   hsb 0x   intfc 0x   pts   "
		      "type   s->e   %10s %10s %12s\n",
		      "  hs 0x "," first "," last  ",
		      dim == 2 ? "" : " surfaces");
	(void) printf("===   ====  ========%10s  ========  ========  ====== "
		      "====  ======  %10s %10s %12s\n",
		      "========","========","========",
		      dim == 2 ? "" : "============");
	for (i = 0, c = intfc->curves; c && *c; c++,i++)
	{
	    (void) printf("%3d | %3d  %llu  %llu  %llu",
			  i,Boundary(*c),curve_number((*c)->obj),
			  (dim==2) ?
			     hypersurface_number(Hyper_surf(*c)) :
			     hypersurface_boundary_number(Hyper_surf_bdry(*c)),
			  interface_number(intfc));
	    (void) printf(" %6d",(*c)->num_points);
	    
	    if (dim==2)
		print_wave_type(" ",wave_type(*c),"",intfc);
	    else if (dim==3)
		print_hsbdry_type(" ",hsbdry_type(*c),"",intfc);
	    
	    (void) printf(" %3d->%-3d %llu %llu %4s",
			  index_of_pointer((POINTER*)intfc->nodes,
					   (POINTER)(*c)->start),
			  index_of_pointer((POINTER*)intfc->nodes,
					   (POINTER)(*c)->end),
			  bond_number((*c)->first,intfc),
			  bond_number((*c)->last,intfc),
			  (dim == 2) ? "" : "pos[");
	    if (intfc->dim == 3)
            {
		for (s = (*c)->pos_surfaces; s && *s; s++)
		    (void) printf(" %d",
				  index_of_pointer((POINTER*)
						   (*s)->interface->surfaces,
						   (POINTER)*s));
		(void) printf(" ] neg[");
		for (s = (*c)->neg_surfaces; s && *s; s++)
		    (void) printf(" %d",
				  index_of_pointer((POINTER*)
						   (*s)->interface->surfaces,
						   (POINTER)*s));
		(void) printf(" ]\n");
	    }
	    else (void) printf("\n");
	}
	fflush(stdout);
}		/*end summary_of_curves*/

LOCAL 	void 	summary_of_surfaces(
	INTERFACE 	*intfc)
{
	int	i = 0,num_points;
	SURFACE **s;
	CURVE 	**c;

	(void) printf("                                                        "
		      "wave  comps         \n");
	(void) printf("SUR | bdry   obj 0x    hs 0x    intfc 0x  points  tris  "
		      "type pos neg  curves\n");
	(void) printf("===   ====  ========  ========  ========  ====== ====== "
		      "==== === === ==========\n");
	for (i = 0, s = intfc->surfaces; s && *s; s++, i++)
	{
	    num_points = points_on_surface(*s);
	    (void) printf("%3d | %3d   %llu  %llu  %llu",i,Boundary(*s),
			  surface_number((*s)->obj),
			  hypersurface_number(Hyper_surf(*s)),
			  interface_number((*s)->interface));
	    (void) printf(" %6d %6d %4d  %3d %3d pos[",num_points,(*s)->num_tri,
			  wave_type(*s),
			  positive_component(*s),negative_component(*s));
	    for (c = (*s)->pos_curves; c && *c; c++)
	    {
		intfc = (*c)->interface;
		(void) printf(" %d",index_of_pointer((POINTER*)intfc->curves,
						     (POINTER)*c));
	    }
	    (void) printf(" ] neg[");
	    for (c = (*s)->neg_curves; c && *c; c++)
	    {
		intfc = (*c)->interface;
		(void) printf(" %d",index_of_pointer((POINTER*)intfc->curves,
						     (POINTER)*c));
	    }
	    (void) printf(" ]\n");
	}
	fflush(stdout);
}		/*end summary_of_surfaces*/

EXPORT  void summary_of_interface(
	INTERFACE 	*intfc)
{
 	int i,j;
	size_t nnodes = size_of_pointers(intfc->nodes);
	size_t ncurves = size_of_pointers(intfc->curves);
	size_t nsurfaces = size_of_pointers(intfc->surfaces);

	(void) printf("\n");
	(void) printf("INTERFACE %llu; %3lu nod; %3lu cur; %3lu sur; n-p = %d; "
		      "dim = %d  rbt = ",
		      interface_number(intfc),nnodes,ncurves,nsurfaces,
		      intfc->num_points,intfc->dim);
	for (i = 0; i < 3; i++)
	    for (j = 0; j < 2; j++)
	        (void) printf("%d ",intfc->rect_bdry_type[i][j]);
  
	(void) printf("\n");
	(void) printf("\n");
	summary_of_nodes(intfc);
	(void) printf("\n");
	summary_of_curves(intfc);
	(void) printf("\n");
	if (intfc->dim == 3)
	{
	    summary_of_surfaces(intfc);
	    (void) printf("\n");
	}
}		/*end summary_of_interface*/

EXPORT void summarize_interface(
	const char 	       *dname,
	const char 	       *fname,
	INTERFACE 	       *intfc,
	const COORDINATE_PLANE proj,
	const char 	       *function_name,
	const char 	       *msg)
{
    	char gvname[256];
	(void) printf("summarize_interface() called by %s(), %s  intfc = %p\n",
		      function_name,msg,intfc);
	if (intfc == NULL)
	    return;
	summary_of_interface(intfc);
	xgraph_RECT_GRID(dname,computational_grid(intfc));
	if (intfc->dim == 3)
	    xgraph_interface_surfaces(dname,fname,intfc,proj);
	xgraph_interface_curves(dname,fname,intfc,proj);
	xgraph_interface_nodes(dname,fname,intfc,proj);
	if (intfc->dim == 3) 
	{
	    (void) sprintf(gvname,"%s/%s_gv",dname,fname);
	    gview_plot_interface(gvname,intfc);
	}
}		/*end summarize_interface*/

EXPORT  void detail_of_curve(
	CURVE *c)
{
	BOND      *b;
	INTERFACE *intfc = c->interface;
	int       b_cnt;
	int       dim = intfc->dim;
	SURFACE   **s;
	BOND_TRI  **btris;

	(void) printf("\n");
	(void) printf("  start of detail_of_curve( %llu )\n",
		      curve_number(c));
	(void) printf("  _boundary = %d  obj = %llu  %s %llu  "
		      "interface = %p, num_points = %d\n",
		      Boundary(c),curve_number(c),
		      (dim==2) ? "hs = " : "hsb = ",
		      (dim==2) ?
			  hypersurface_number(Hyper_surf(c)) :
		          hypersurface_boundary_number(Hyper_surf_bdry(c)),
		      c->interface,c->num_points);
	(void) printf("  c->start = %g %g %g  "
		      "c ->end = %g %g %g\n",
		      Coords(c->start->posn)[0],Coords(c->start->posn)[1],
		      Coords(c->start->posn)[2],
		      Coords(c->end->posn)[0],Coords(c->end->posn)[1],
		      Coords(c->end->posn)[2]); 
	(void) printf("  positive surfaces: ");
	for (s = c->pos_surfaces; s && *s; s++)
	    (void) printf("  %p [ %d ]",*s,
			  index_of_pointer((POINTER*)(*s)->interface->surfaces,
					   (POINTER)*s));
	(void) printf("\n");
	(void) printf("  negative surfaces: ");
	for (s = c->neg_surfaces; s && *s; s++)
	    (void) printf("  %p [ %d ]",*s,
			  index_of_pointer((POINTER*)(*s)->interface->surfaces,
					   (POINTER)*s));
	(void) printf("\n");
	(void) printf("  start of bond btris listing:\n");
	b = c->first;
	b_cnt = 0; 
	do
	{
	    if (30*(b_cnt/30) == b_cnt)
	    {
		(void) printf("\n");
		(void) printf("%4s %9s %9s %9s | %20s\n","cnt",
			      "bond ","prev ","next ","btris [ surf ]  ");
		(void) printf("%4s %9s %9s %9s | %20s\n", "====",
			      "=========","=========","=========",
			      "====================");
	    }

	    (void) printf("%4d %llu %llu %llu |",b_cnt,
			  bond_number(b,intfc),
			  bond_number(b->prev,intfc),
			  bond_number(b->next,intfc));

	    for (btris = Btris(b); btris && *btris; btris++)
	    {
		(void) printf(" %llu [ %d ]",bond_tri_number(*btris,intfc),
			     index_of_pointer((POINTER*)intfc->surfaces,
		             (POINTER)Surface_of_tri((*btris)->tri)));
	    }
	    (void) printf("\n");

	    b = b->next;
	    b_cnt++;
	}        
	while (b != NULL);

	(void) printf("\n");
	(void) printf("  start of bond length listing:\n");
	b = c->first;
	b_cnt = 0; 
	do
	{
	    if (30*(b_cnt/30) == b_cnt)
	    {
		(void) printf("\n");
		(void) printf("%4s %9s %9s %9s | %23s | %23s | %8s\n","cnt",
			"bond ","prev ","next ","b->start       ",
			      "b->end        ","length ");
		(void) printf("%4s %9s %9s %9s | %23s | %23s | %8s\n","====",
			      "=========","=========","=========",
			      "=======================",
			      "=======================","========");
	    }

	    (void) printf("%4d %llu %llu %llu | %g %g %g "
			  "| %g %g %g | %g\n",b_cnt,
			  bond_number(b,intfc),
			  bond_number(b->prev,intfc),
			  bond_number(b->next,intfc),
			  Coords(b->start)[0],Coords(b->start)[1],
			  Coords(b->start)[2],
			  Coords(b->end)[0],Coords(b->end)[1],
			  Coords(b->end)[2],b->length);
	    b = b->next;
	    b_cnt++;
	}        
	while (b != NULL);

	(void) printf("\n");
	(void) printf("  end of detail_of_curve()\n");
}                          /* end detail_of_curve */

#endif /* defined(THREED) */
