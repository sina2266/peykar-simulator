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
*			isect2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains the routine intersections() and its support routines.
*/


#if defined(TWOD)
#include <intfc/iloc.h>


/*
*				i_intersections2d():
*
*	Determines all points of intersection of CURVES of an INTERFACE,
*	guarding against various degeneracies that can occur.   The
*	boundary CURVES, if any, of INTERFACE are not included in 
*	checking for intersections unless the variable bdry = YES.
*
*	Returns a linked list of CROSS structures describing the 
*	intersection points.  Each intersection is described first in
*	terms of an equivalent NODE (with Position, in_curves, out_curves
*	filled in) and then in terms of the POINT of crossing, the BONDS
*	that cross and the CURVES which cross at that point.   This
*	information is highly redundant.   The NODE is not an actual node
*	on the INTERFACE (yet), and the in_curves, out_curves of the NODE 
*	are those currently crossing at the point - thus the NODE is really
*	just one more way to represent the crossing points.
*
*	The CROSS structure is defined in file int.h.
*
*	The CROSS linked list is returned in the CROSS 
*	pointer whose address is supplied in the call:
*	If there are no intersections, then these pointers are set to
*	NULL.
*
*	Usage:
*			status = i_intersections2d(intfc,&cross,bdry);
*			INTERFACE *intfc;
*			CROSS *cross;
*			int bdry;
*
*	The return value is 1 if the call succeeds and 0 on error.
*/



LIB_LOCAL boolean i_intersections2d(
	INTERFACE	*intfc,
	CROSS		**cross,
	const boolean	bdry) /* Checks for boundary intersections if YES */
{
	int		ix , iy, nb, i,j,xmax,ymax;
	int             **number_of_bonds;
	BOND		**b;
	BOND            ****bonds_in_block;
	CURVE		**c;
	CURVE           ****curves_in_block;
	FT_POINT		*p;
	FT_POINT		pc;
	INTERFACE	*oldintfc;
	COMPONENT       **compon2d;
	CROSS		*cr, Cr;	/* Cr is a Dummy Place Holder
					   for Head of CROSS list */
	CROSS		*cr1,*cr2;

	if (DEBUG)
	    (void) printf("Entered i_intersections2d()\n");
	start_clock("intersections");
	oldintfc = current_interface();
	set_current_interface(intfc);

	if (intfc->modified || intfc->table->new_grid)
	{
	    if (make_bond_comp_lists(intfc) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in i_intersections2d(), "
	    	              "make_bond_comp_lists() failed\n");
	    	stop_clock("intersections");
	    	return FUNCTION_FAILED;
	    }
	}
	compon2d = intfc->table->compon2d;
	number_of_bonds = intfc->table->num_of_bonds;
	bonds_in_block = intfc->table->bonds;
	curves_in_block = intfc->table->curves;

		/* Initialize the CROSS List: */
	Cr.prev = NULL;
	Cr.next = NULL;
	cr = &Cr;	

	xmax = topological_grid(intfc).gmax[0];
	ymax = topological_grid(intfc).gmax[1];

	for (iy=0; iy<ymax; iy++)
	{
	    for (ix=0; ix<xmax; ix++)
	    {
	    		/* Skip Blocks with < 2 Bonds: */

	    	if (compon2d[iy][ix] != ONFRONT)
		    continue;

	    	if ((nb = number_of_bonds[iy][ix])==1)
		    continue;

	    	b = bonds_in_block[iy][ix];
	    	c = curves_in_block[iy][ix];

	    		/* Loop over all pairs of Bonds: */

	    	for (i=0; i<nb-1; i++)
	    	{
	    	    if (is_bdry(c[i]) && !bdry)
		        continue;
	    	    if (is_subdomain_boundary(Hyper_surf(c[i])))
	    	    	continue;
	    	    for(j=i+1; j<nb; j++)
	    	    {
	    	    	if (is_bdry(c[j]) && !bdry)
	    	    	    continue;
	    	    	if (is_subdomain_boundary(Hyper_surf(c[j])))
	    		    continue;

	    			/* Skip if Adjacent Bonds */
	    		if (b[i]->next == b[j])
			    continue;
	    		if (b[i]->prev == b[j])
			    continue;

	    			/* Skip if Meet at a Point: */
	    		if (b[i]->start==b[j]->start)
			    continue;
	    		if (b[i]->end==b[j]->end)
			    continue;
	    		if (b[i]->start==b[j]->end)
			    continue;
	    		if (b[i]->end==b[j]->start)
			    continue;

	    			/* Check if they Cross: */
	    		if (!cross_bonds(b[i],b[j],&pc)) 
	    		    continue;

	    			/* Store the Data: */

	    		p = Point(Coords(&pc));

	    			/* Create the CROSS: */
	    		cr->next = (CROSS *)store(sizeof(CROSS));
	    		cr->next->prev = cr;
	    		cr = cr->next;
	    		cr->next = NULL;

	    		cr->c1 = c[i];
	    		cr->c2 = c[j];
	    		cr->b1 = b[i];
	    		cr->b2 = b[j];
	    		cr->p  = p;
	    	    }
	    	}
	    }
	}




		/* Now Delete Duplicates Caused by Degeneracies: */

	for (cr1=Cr.next; cr1!=NULL && cr1->next!=NULL; cr1=cr1->next)
	{
		
	    CROSS *crl = cr1;	/* Last value of cr2 */ 

	    for (cr2=cr1->next; cr2!=NULL; cr2=cr2->next)
	    {

	    		/* Are BONDS of cr1 && cr2 Identical: */

	    	if   ( ((cr1->b1==cr2->b1) && (cr1->b2==cr2->b2)) || 
	    		  ((cr1->b1==cr2->b2) && (cr1->b2==cr2->b1))) 
	    	{

	    	    /* Drop next CROSS */

	    	    crl->next = cr2->next;
	    	    if (cr2->next)
			cr2->next->prev = crl;
	    	}
		else
		{
		    crl = crl->next;
		}
	    }
	}

	*cross = Cr.next;
	if (*cross)
	    (*cross)->prev = NULL;
	set_current_interface(oldintfc);

	if (DEBUG) (void) printf("Leaving i_intersections2d()\n\n");
	stop_clock("intersections");
	return FUNCTION_SUCCEEDED;
}		/*end i_intersections2d*/




LIB_LOCAL void i_print_intersections2d(
	CROSS		*cross,
	INTERFACE	*intfc)
{
	int		num;
	CROSS		*cr;

	(void) output();
	(void) printf("INTERSECTIONS:\n\n");
	print_interface(intfc);

	(void) printf("Here are the intersections:\n\n");
	for (num = 1, cr = cross; cr; num++, cr = cr->next)
	{
	    (void) printf("Intersection %d:",num);
	    (void) printf("\nCorresponding Cross Node:\n");
	    (void) printf("Cross at position (%g, %g)\n",
			  Coords(cr->p)[0],Coords(cr->p)[1]);
	    (void) printf("Crossing bonds\n");
	    print_bond(cr->b1);
	    print_bond(cr->b2);
	    (void) printf("Crossing curves, cr->c1 = %llu, cr->c2 = %llu\n",
	    	          curve_number(cr->c1),curve_number(cr->c2));
	    if (cr->c1 == cr->c2)
	    {
	    	(void) printf("One Self-Intersecting Curve:\n\n");
	    	print_curve(cr->c1);
	    }
	    else
	    {
	    	(void) printf("Two Curves that Intersect:\n\n");
	    	print_curve(cr->c1);
	    	print_curve(cr->c2);
	    }
	}
}		/*end i_print_intersections2d*/



LIB_LOCAL	void	i_print_crossing_elements2d(
	CROSS		*cross,
	INTERFACE	*intfc)
{
	int num, dim = intfc->dim;
	CROSS *cr;

	if (cross == NULL)
		return;

	(void) printf("Crossing bonds of intersections\n");
	for (cr = cross, num = 1; cr; cr = cr->next, num++)
	{
		(void) printf("Intersection %d, ",num);
		print_general_vector("at position ",Coords(cr->p),dim,"\n");
		(void) printf("Bond cr->b1 %llu on curve cr->c1 %llu\n",
			      bond_number(cr->b1,cr->c1->interface),
			      curve_number(cr->c1));
		print_bond(cr->b1);
		(void) printf("cr->b1->start %llu, cr->b1->end %llu\n",
			      point_number(cr->b1->start),
			      point_number(cr->b1->end));
		(void) printf("Bond cr->b2 %llu on curve cr->c2 %llu\n",
			      bond_number(cr->b2,cr->c2->interface),
			      curve_number(cr->c2));
		print_bond(cr->b2);
		(void) printf("cr->b2->start %llu, cr->b2->end %llu\n",
			      point_number(cr->b2->start),
			      point_number(cr->b2->end));
	}
}	/* end print_crossing_elements2d */


/*
*		bond_crosses_curve():
*
*	Find the crossing point p of the bond b with curve c.
*	The function assumes that the memory of point p is already
*	allocated. It returns the corresponding bond bcurve on
*	c which crosses with b. If orient is POSITIVE_ORIENTATION,
*	the function searches from the first bond, otherwise it
*	starts search from the last bond. Return YES if successful,
*	NO is no crossing bond is find.
*/


EXPORT boolean bond_crosses_curve(
	BOND *b,
	CURVE *c,
	FT_POINT *p,
	BOND **bcurve,
	ORIENTATION orient)
{
	BOND *bc;
	if (orient == POSITIVE_ORIENTATION)
	{
	    for (bc = c->first; bc != NULL; bc = bc->next)
	    {
	    	if (cross_bonds(b,bc,p))
		{
		    *bcurve = bc;
		    return YES;
		}
	    }
	}
	else
	{
	    for (bc = c->last; bc != NULL; bc = bc->prev)
	    {
	    	if (cross_bonds(b,bc,p))
		{
		    *bcurve = bc;
		    return YES;
		}
	    }
	}
	return NO;
}	/* end bond_crosses_curve */

/*
*                          cross_bonds():
*
*	Determines the point of crossing of two BONDS, if any.
*	Returns the point in the POINT pointed to by p.
*
*		Usage:
*			status = cross_bonds(b1,b2,&point);
*			BOND *b1, *b2;
*			FT_POINT p;
*
*	Returns 1 if the bonds cross, 0 otherwise.
*
*/


EXPORT boolean cross_bonds(
	BOND		*b1,
	BOND		*b2,
	FT_POINT		*p)
{
	double		x1=(double)Coords(b1->start)[0];
	double		y1=(double)Coords(b1->start)[1];
	double		x2=(double)Coords(b1->end)[0];
	double		y2=(double)Coords(b1->end)[1];
	double		x3=(double)Coords(b2->start)[0];
	double		y3=(double)Coords(b2->start)[1];
	double		x4=(double)Coords(b2->end)[0];
	double		y4=(double)Coords(b2->end)[1];
	double		nor_dist_t,nor_dist_s;
	double		sinth;		/* sin of angle between bonds */
	double		xcross,ycross;	/* coord of intersection
					   of lines 12 34 */
	double		x00,y00;	/* beginning of long bond */
	double		x0,y0;		/* beginning of short bond after
					   coord translation */
	double		x,y;		/* end of long bond after
					   coord translation */
	double		dx,dy;		/* short bond end - start */
	double		t;		/* fractional distance on short bond */
	double		s;		/* fractional distance on long bond */
	double		len12;		/* length b1 * length b2 */
	double		parallel = PARALLEL(current_interface());

	if (bond_length(b1) > bond_length(b2)) 
	{
	    x00 = x1;		y00 = y1;
	    x0 = x3 - x1;	y0 = y3 - y1;
	    x  = x2 - x1;	y  = y2 - y1;
	    dx = x4 - x3;	dy = y4 - y3;
	}
	else 
	{
	    x00 = x3;		y00 = y3;
	    x0 = x1 - x3;	y0 = y1 - y3;
	    x  = x4 - x3;	y  = y4 - y3;
	    dx = x2 - x1;	dy = y2 - y1;
	}
	sinth = dx*y - dy*x;
	nor_dist_t = x0*y - y0*x;
	nor_dist_s = dx*y0 - dy*x0;
	len12 = bond_length(b1) * bond_length(b2);

	if (fabs(sinth) <= parallel * len12) 
	{
	    /* Case of parallel lines */
	    if (fabs(nor_dist_t) <= parallel * len12) 
	    {
	    	/* Lines coincide */
	    	if (Between(x0,0.0,x) && Between(y0,0.0,y)) 
	    	{
	    	    /* Cross at x0,y0 */
	    	    Coords(p)[0] = (double)(x0 + x00);
	    	    Coords(p)[1] = (double)(y0 + y00);
	    	    return YES;
	    	}
		if (Between(x0+dx,0.0,x) && Between(y0+dy,0.0,y)) 
		{
		    /* Cross at x0+dx,y0+dy */
		    Coords(p)[0] = (double)(x0 + dx + x00);
		    Coords(p)[1] = (double)(y0 + dy + y00);
		    return YES;
		}
		return NO; /* No cross; line segments don't overlap */
	    }
	    return NO; /* No cross; lines distinct although parallel */
	}

		/* Now lines are not parallel */

	t = - nor_dist_t / sinth;
	s = nor_dist_s / sinth;
	if (t <= 0.0 || t > 1.0 || s <= 0.0 || s > 1.0)
	    return NO;
	xcross = 0.5*(x0 + t*dx + s*x);
	ycross = 0.5*(y0 + t*dy + s*y);
	Coords(p)[0] = (double)(xcross + x00);
	Coords(p)[1] = (double)(ycross + y00);
	return YES;
}		/*end cross_bonds*/

#endif /* defined(TWOD) */
