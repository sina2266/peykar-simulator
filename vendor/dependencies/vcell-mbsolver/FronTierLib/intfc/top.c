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
*				top.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	This file contains routines to assist in the modification of
*	interface topology. It supplements the routines split_curve and
*	join_curves from int.c which also serve this purpose.
*/


#include <intfc/iloc.h>


	/* LOCAL Function Prototypes */
LOCAL	CURVE	*prompt_for_curve(COMPONENT, COMPONENT, NODE*, NODE*);
LOCAL	boolean	matchable_comps(COMPONENT,COMPONENT,INTERFACE*);


/*
*			points_in_strict_order():
*	This routine determines whether a point p2 on bond b2 occurs
*	later in the order on a curve than a point p1 of a bond b1,
*	both assumed to occur on the same curve.
*/

/*ARGSUSED*/
EXPORT boolean points_in_strict_order(
	FT_POINT		*p1,
	BOND		*b1,
	FT_POINT		*p2,
	BOND		*b2,
	int		dim)
{
	BOND		*b;

	if (b1 == b2)
	{
	    if (separation(p2,b1->start,dim) > separation(p1,b1->start,dim))
	    	return YES;
	    else
	    	return NO;
	}

	for (b = b1->next; b; b = b->next)
	    if (b == b2)
		return YES;

	return NO;
}		/*end points_in_strict_order*/

/*
*			bonds_in_strict_order():
*/

EXPORT boolean bonds_in_strict_order(
	BOND		*b1,
	BOND		*b2)
{
	BOND		*b;

	if (b1 == b2)
	    return NO;
	for (b = b1->next; b; b = b->next)
	    if (b == b2)
		return YES;

	return NO;
}		/*end bonds_in_strict_order*/

/*ARGSUSED*/
EXPORT void set_point_of_bond(
	FT_POINT		*p,
	BOND		*b,
	ORIENTATION	orient,
	int		dim)
{
	if (b == NULL)
	    return;
	if (orient == POSITIVE_ORIENTATION)
	    b->start = p;
	else
	    b->end = p;
	bond_length(b) = separation(b->start,b->end,dim);
}		/*end set_point_of_bond*/


EXPORT	void invert_bond(
	BOND		*b)
{
	FT_POINT		*ptmp;
	BOND		*btmp;

	ptmp = b->start;	b->start = b->end;	b->end = ptmp;
	btmp = b->prev;		b->prev = b->next;	b->next = btmp;
}		/*end invert_bond*/

/*ARGSUSED*/
EXPORT	void set_bond_length(
	BOND		*b,
	int		dim)
{
	if (b == NULL) return;
	bond_length(b) = separation(b->start,b->end,dim);
}		/*end set_bond_length*/

EXPORT	void print_bdry_side(
	const char *mesg,
	BDRY_SIDE  side,
	const char *end)
{
	(void) printf("%s%s%s",mesg,bdry_side_as_string(side),end);
}		/*end print_node_status*/

EXPORT	const char	*bdry_side_as_string(
	BDRY_SIDE side)
{
	static char s[120];
	switch (side)
	{
	case NOT_A_BDRY:
	    return "NOT_A_BDRY";
	case LEFT_BDRY:
	    return "LEFT_BDRY";
	case RIGHT_BDRY:
	    return "RIGHT_BDRY";
	case LOWER_BDRY:
	    return "LOWER_BDRY";
	case UPPER_BDRY:
	    return "UPPER_BDRY";
	case ZMIN_BDRY:
	    return "ZMIN_BDRY";
	case ZMAX_BDRY:	 
	    return "ZMAX_BDRY";
	default:		    
	    (void) sprintf(s,"UNKNOWN BDRY_SIDE value %d",side);
	    return s;
	}
}		/*end node_status_as_string*/


EXPORT	void print_bond(
	BOND		*b)
{
	fprint_bond(stdout,b);
}		/*end print_bond*/

LIB_LOCAL void fprint_bond(
	FILE *file,
	BOND *b)
{
	int	  dim;
	INTERFACE *intfc;

	if (b == NULL)
	{
	    (void) fprintf(file,"NULL bond\n");
	    return;
	}

	intfc = current_interface();
	dim = intfc->dim;
	(void) fprintf(file,"bond %llu\n",bond_number(b,intfc));
	fprint_general_vector(file,"start ",Coords(b->start),dim," ");
	(void) fprintf(file,"%llu\n",point_number(b->start));
	fprint_general_vector(file,"end   ",Coords(b->end),dim," ");
	(void) fprintf(file,"%llu\n",point_number(b->end));
	if (dim == 3)
	{
	    BOND_TRI **btris;

	    (void) fprintf(file,"btris: ");
	    for (btris = Btris(b); btris && *btris; ++btris)
	    	(void) fprintf(file,"%p ",(POINTER)*btris);
	    (void) fprintf(file,"\n");
	}
	(void) fprintf(file,"len %g  prev %llu  next %llu\n",
		            b->length,bond_number(b->prev,intfc),
		            bond_number(b->next,intfc));
}		/*end fprint_bond*/

/*
*			print_bond_list():
*
*	This routine prints the bond list for a curve c
*/

EXPORT	void print_bond_list(
	CURVE		*c)
{
	BOND		*b;
	char		endchar;
	int		i, dim;


	if (!debugging("bond_list")) return;

		/* output bond list */

	(void) printf("\nBond list for curve %llu:\n",curve_number(c));

	dim = c->interface->dim;
	(void) printf(" %llu | ",bond_number(c->first->prev,c->interface));
	for (i = 1,b = c->first; b != c->last; b = b->next,++i)
	{
		endchar = i%4 ? ' ' : '\n';
		(void) printf(" -> %llu%c",bond_number(b,c->interface),endchar);
	}
	(void) printf(" -> %llu ",bond_number(c->last,c->interface));
	(void) printf(" -> | %llu ",bond_number(c->last->next,c->interface));
	(void) printf("\n\n");

	print_general_vector(" ",Coords(c->first->start),dim,"");
	(void) printf(" | ");
	for (i = 1,b = c->first; b != c->last; b = b->next,++i)
	{
		endchar = i%4 ? ' ' : '\n';
		print_general_vector(" -> ",Coords(b->end),dim,"");
		(void) printf("%c",endchar);
	}
	print_general_vector(" -> | ",Coords(c->last->end),dim,"");
	(void) printf("\n\n");
}		/*end print_bond_list*/


/*
*			curve_length():
*
*	This routine computes the length of a curve.
*/

EXPORT	double curve_length(
	CURVE		*c)
{
	BOND		*b;
	double   	len;

	if (c == NULL) return 0.0;
	len = bond_length(c->last);

	for (b = c->first; b != c->last; b = b->next)
		len += bond_length(b);

	return len;
}		/*end curve_length*/


/*
*			i_invert_curve():
*
*	This routine reverses the direction of a curve, keeping the same
*	curve, bond and point storage locations.
*/

EXPORT	void i_invert_curve(
	CURVE		*c)
{

	reverse_curve(c);

		/* Invert comps */
	switch (c->interface->dim)
	{
	case 2:
	{
		COMPONENT compx;
		compx = negative_component(c);
		negative_component(c) = positive_component(c);
		positive_component(c) = compx;
		break;
	}
	case 3:
		break;
	}
}		/*end i_invert_curve*/

EXPORT	void i_reverse_curve(
	CURVE		*c)
{
	NODE		*nse, *nes;
	BOND		*bb, *bse, *bes;
	BOND		bx;

		/* Invert nodes */

	nse = c->start;		nes = c->end;
	if (!(delete_from_pointers(c,&nse->out_curves) &&
		 delete_from_pointers(c,&nes->in_curves)))
	{
	    screen("ERROR in i_reverse_curve(), "
	           "delete_from_pointers() failed\n");
	    clean_up(ERROR);
	}
	c->start = nes;		c->end = nse;
	if (!(add_to_pointers(c,&nes->out_curves) && 
		 add_to_pointers(c,&nse->in_curves)))
	{
	    screen("ERROR in i_reverse_curve(), "
	           "add_to_pointers() failed\n");
	    clean_up(ERROR);
	}

		/* Invert bond chains */

	bse = c->first;		bes = c->last;
	bb = bse;
	while (bb != NULL)
	{
	    bx.prev  = bb->prev;	bx.next = bb->next;
	    bx.start = bb->start;	bx.end  = bb->end;
	    bb->prev  = bx.next;	bb->next = bx.prev;
	    bb->start = bx.end;	bb->end  = bx.start;
	    bb = bx.next;
	}

	c->first = bes;		c->last = bse;
}		/*end i_reverse_curve*/

/*
*				adjacent_curve():
*
*	Finds a curve adjacent to a given curve at a given node in a
*	specified angular direction.
*/

EXPORT CURVE *adjacent_curve(
	CURVE		*curve,
	ORIENTATION	c_orient,
	ANGLE_DIRECTION	angle_dir,
	ORIENTATION	*adj_c_orient)
{
	INTERFACE  *intfc = curve->interface;
	NODE	   *n = Node_of(curve,c_orient);
	CURVE	   **c,*ans = NULL;
	BOND	   *b = Bond_at_node(curve,c_orient);
	FT_POINT	   *p;
	COMPONENT  test_comp;
	double	   t1[MAXD], t2[MAXD];
	double	   sin12,cos12,oldsin12,oldcos12;
	int	   i, dim = curve->interface->dim;
	static int nfail = 0;

	debug_print("adjacent_curve","Entered adjacent_curve()\n");
	if (debugging("adjacent_curve"))
	{
	    print_curve(curve);
	    print_orientation("c_orient = ",c_orient,", ");
	    print_angle_direction("angle_dir = ",angle_dir,"\n");
	}
	p = Point_adjacent_to_node(curve,c_orient);
	for (i = 0; i < dim; ++i)
	    t1[i] = (Coords(p)[i] - Coords(n->posn)[i]) / bond_length(b);

	test_comp = (curve_ang_oriented_l_to_r(angle_dir,c_orient)) ?
				positive_component(curve) :
				negative_component(curve);

	if (debugging("adjacent_curve"))
	    (void) printf("test_comp = %d\n",test_comp);
	for (c = n->in_curves; c && *c; ++c) 
	{
	    if (*c == curve && c_orient == NEGATIVE_ORIENTATION)
	    	continue;

	    	/* Test for consistent component */
		
	    if (((angle_dir == CLOCKWISE) &&
		    !matchable_comps(positive_component(*c),test_comp,intfc))
				 ||
		((angle_dir == COUNTER_CLOCK) &&
		    !matchable_comps(negative_component(*c),test_comp,intfc)))
			continue;

	    b = (*c)->last;
	    p = b->start;
	    for (i = 0; i < dim; ++i)
	    {
	        t2[i] = (Coords(p)[i] - Coords(n->posn)[i])/bond_length(b);
	    }
	    (void) vector_product(t1,t2,&sin12,dim);
	    cos12 = scalar_product(t1,t2,dim);
	    if (ans == NULL) 
	    {
	        oldsin12 = sin12;
	        oldcos12 = cos12;
	        ans = *c;
	        *adj_c_orient = NEGATIVE_ORIENTATION;
	        continue;
	    }
	    if (is_new_angle_smaller(sin12,cos12,oldsin12,oldcos12,angle_dir)) 
	    {
	        oldsin12 = sin12;
	        oldcos12 = cos12;
	        ans = *c;
	        *adj_c_orient = NEGATIVE_ORIENTATION;
	    }
	}

	for (c = n->out_curves; c && *c; ++c) 
	{
	    if (*c == curve && c_orient == POSITIVE_ORIENTATION)
	    	continue;

		/* Test for consistent component */
		
	    if (((angle_dir == CLOCKWISE) &&
		     !matchable_comps(negative_component(*c),test_comp,intfc))
				    ||
	        ((angle_dir == COUNTER_CLOCK) &&
		     !matchable_comps(positive_component(*c),test_comp,intfc)))
	    	continue;

	    b = (*c)->first;
	    p = b->end;
	    for (i = 0; i < dim; ++i)
	    {
	        t2[i] = (Coords(p)[i] - Coords(n->posn)[i])/bond_length(b);
	    }
	    (void) vector_product(t1,t2,&sin12,dim);
	    cos12 = scalar_product(t1,t2,dim);
	    if (ans == NULL) 
	    {
	    	oldsin12 = sin12;
	    	oldcos12 = cos12;
	    	ans = *c;
	    	*adj_c_orient = POSITIVE_ORIENTATION;
	    	continue;
	    }
	    if (is_new_angle_smaller(sin12,cos12,oldsin12,oldcos12,angle_dir)) 
	    {
	    	oldsin12 = sin12;
	    	oldcos12 = cos12;
	    	*adj_c_orient = POSITIVE_ORIENTATION;
	    	ans = *c;
	    }
	}

	if (ans == NULL) 
	{
	    (void) printf("WARNING in adjacent_curve(), returning null\n");
	    /* Returning null could be reasonable! 
	    if (nfail++ < 10) 
	    	(void) printf("WARNING in adjacent_curve(), returning null\n");
	    else
	    {
	    	screen("ERROR in adjacent_curve(), "
		       "can't find adjacent curve\n");
	    	clean_up(ERROR);
	    }
	    */
	}
	else
	    nfail = 0;
	debug_print("adjacent_curve","Leaving adjacent_curve(), ans = %d\n",ans);
	return ans;
}		/*end adjacent_curve*/

LOCAL	boolean	matchable_comps(
	COMPONENT comp1,
	COMPONENT comp2,
	INTERFACE *intfc)
{
	if (equivalent_comps(comp1,comp2,intfc))
	    return YES;
	else if (is_exterior_comp(comp1,intfc) && is_excluded_comp(comp2,intfc))
	    return YES;
	else if (is_exterior_comp(comp2,intfc) && is_excluded_comp(comp1,intfc))
	    return YES;
	else
	    return NO;
}		/*end matchable_comps*/


/*
*			adjacent_curve_along_surface():
*
*	Finds a curve adjacent to a given curve at a given node on a surface
*	with normal vector <nor> at the node in a specified angular 
*	direction.
*
*	Modification of two dimensional adjacent_curve.
*	Belongs in top.c
*/

LIB_LOCAL 	CURVE *adjacent_curve_along_surface(
	CURVE		*curve,
	ORIENTATION	c_orient,
	ANGLE_DIRECTION	angle_dir,
	ORIENTATION	*adj_c_orient,
	double		*nor,
	int		dim)
{
	NODE		*n = Node_of(curve,c_orient);
	CURVE		**c, *ans;
	CURVE		**curves;
	BOND		*b;
	FT_POINT		*p;
	double		t1[MAXD], t2[MAXD], v[MAXD];
	int		i, j;
	ORIENTATION	orient;
	double		sin12,cos12,oldsin12,oldcos12;
	static	char	fname[] = "adjacent_curve_along_surface()";

	debug_print("adjacent_curve","Entered %s\n",fname);
	if (debugging("adjacent_curve"))
	{
		print_curve(curve);
		print_orientation("c_orient = ",c_orient," ");
		print_angle_direction("angle_dir = ",angle_dir,"\n");
	}

	p = Point_adjacent_to_node(curve,c_orient);
	b = Bond_at_node(curve,c_orient);
	for (i = 0; i < dim; ++i)
		t1[i] = (Coords(p)[i] - Coords(n->posn)[i]) / bond_length(b);

	oldsin12 = oldcos12 = ERROR_FLOAT;
	ans = NULL;
	for (j = 0; j < 2; ++j)
	{
		curves = (j) ? n->in_curves : n->out_curves;
		orient = (j) ? NEGATIVE_ORIENTATION : POSITIVE_ORIENTATION;
		for (c = curves; c && *c; ++c) 
		{
			if (*c == curve && c_orient == orient)
				continue;

			b = Bond_at_node(*c,orient);
			p = Point_adjacent_to_node(*c,orient);
			for (i = 0; i < dim; ++i)
			{
				t2[i] = (Coords(p)[i] - Coords(n->posn)[i]) /
						bond_length(b);
			}
			sin12 = vector_product(t1,t2,v,dim);
			cos12 = scalar_product(t1,t2,dim);
			if (scalar_product(nor,v,dim) < 0.0)	sin12 = -sin12;
			if (ans == NULL || is_new_angle_smaller(sin12,cos12,
						oldsin12,oldcos12,angle_dir))
			{
				oldsin12 = sin12;
				oldcos12 = cos12;
				ans =*c;
				*adj_c_orient = orient;
			}
		}
	}

	if (ans == NULL) screen("WARNING: %s returning null\n",fname);
	debug_print("adjacent_curve","Leaving %s, ans = %d\n",fname,ans);
	return ans;
}		/*end adjacent_curve_along_surface*/


/*
*			i_make_fourier_curve():
*
*	i_make_fourier_curve constructs a Fourier polynomial curve with 
*	Fourier data given in the structure fpoly.
*/

EXPORT CURVE	*i_make_fourier_curve(
	int		num_points,
	double		x0,
	double		x1,
	FOURIER_POLY	*fpoly,
	COMPONENT	l_comp,
	COMPONENT	r_comp)
{
	int		i;
	double		dx, coords[MAXD];
	CURVE		*cur;
	NODE		*ns,*ne;

	coords[0] = x1;	coords[1] = fourier_poly(coords,fpoly);
	ns = make_node(Point(coords));
	coords[0] = x0;	coords[1] = fourier_poly(coords,fpoly);
	ne = make_node(Point(coords));
	cur = make_curve(l_comp,r_comp,ns,ne);

	dx = (x1 - x0)/num_points;
	for (i = 1; i < num_points; ++i)
	{
	    coords[0] = x0 + i*dx;
	    coords[1] = fourier_poly(coords,fpoly);
	    if (insert_point_in_bond(Point(coords),cur->first,cur) !=
		FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in i_make_fourier_curve(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	}

	set_is_bdry(ns);
	set_is_bdry(ne);
	return cur;
}		/*end i_make_fourier_curve*/


/*
*			make_curve_from_file():
*
*	Makes a curve by reading points
*	from the file c_file.  The points in c_file are
*	assumed to be in two columns, the first being the
*	x coordinates and the second the y coordinates
*	of the points on the curve.
*	This function will reads points from the file c_file
*	until an end of file is encountered.
*/

EXPORT CURVE *read_curve_from_file(
	COMPONENT	left,
	COMPONENT	right,
	NODE		*ns,
	NODE		*ne,
	char		*c_file)
{
	FILE		*fp;
	int		c, i, dim = 2; /*TODO: Upgrade for 3D */
	CURVE		*cur;
	double		coords[MAXD];

	if (strcmp(c_file,"stdin") == 0)
		return prompt_for_curve(left,right,ns,ne);

	if ((fp = fopen(c_file,"r")) == NULL)
	{
		screen("ERROR in make_curve_from_file()\n");
		screen("Unable to open %s\n",c_file);
		clean_up(ERROR);
	}
	for (i = 0; i < dim; ++i)
	{
		(void) fscan_float(fp,Coords(ns->posn)+i);
	}
	cur = make_curve(left,right,ns,ne);
	while ((c = getc(fp)) != EOF)
	{
	    (void) ungetc(c,fp);
	    for (i = 0; i < dim; ++i)
	    {
	        (void) fscan_float(fp,coords+i);
	    }
	    if (insert_point_in_bond(Point(coords),cur->last,cur) !=
		FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in read_curve_from_file(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	    if ((c = getc(fp)) != '\n')
		(void) ungetc(c,fp);
	}
	for (i = 0; i < dim; ++i)
	    Coords(ne->posn)[i] = coords[i];
	(void) delete_start_of_bond(cur->last,cur);
	(void) fclose(fp);
	return cur;
}		/*end read_curve_from_file*/


/*TODO: Upgrade for 3D */
LOCAL CURVE *prompt_for_curve(
	COMPONENT	left,
	COMPONENT	right,
	NODE		*ns,
	NODE		*ne)
{
	int		i,j,dim = 2,n;
	double		coords[MAXD];
	CURVE		*curve;

	screen("Enter Number of Interior Points on Curve: ");
	(void) Scanf("%d\n",&n);
	screen("Enter the start node coordinates: ");
	for (j = 0; j < dim; ++j)
		(void) Scanf("%f",&Coords(ns->posn)[j]);
	(void) Scanf("\n"); /* Grab trailing newline */
	curve = make_curve(left,right,ns,ne);

	screen("Enter %d Coordinate Points one per line:\n",n);
	for (i=0; i<n; ++i)
	{
	    screen(": ");
	    for (j = 0; j < dim; ++j)
	    	(void) Scanf("%f",coords+i);
	    (void) Scanf("\n"); /* Grad trailing newline */
	    if (insert_point_in_bond(Point(coords),curve->last,curve) !=
		FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in prompt_for_curve(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	}
	screen("Enter the end node coordinates: ");
	for (j = 0; j < dim; ++j)
	    (void) Scanf("%f",&Coords(ne->posn)[i]);
	(void) Scanf("\n"); /* Grad trailing newline */
	return curve;
}		/*end prompt_for_curve*/

/*
*			print_o_curve_family():
*/

EXPORT void print_o_curve_family(
	O_CURVE_FAMILY	*ocf)
{
	O_CURVE		*oc;

	(void) printf("\tO_curve_family %p:\n",(POINTER)ocf);
	if (!ocf)
	{
		(void) printf("\tEnd O_curve_family %p:\n\n",(POINTER)ocf);
		return;
	}
	(void) printf("first %p  last %p\n",
		      (POINTER)ocf->first,(POINTER)ocf->last);
	for (oc = ocf->first; oc; oc = oc->next)
	{
		print_o_curve(oc);
		if (oc == ocf->last) break;
	}
	(void) printf("\tEnd O_curve_family %p:\n\n",(POINTER)ocf);
}		/*end print_o_curve_family*/

/*
*			print_o_curve():
*/

EXPORT void print_o_curve(
	O_CURVE		*oc)
{
	(void) printf("O_curve = %p:\n",(POINTER)oc);
	if (!oc)
	    return;
	print_orientation("orient ",oc->orient,"  ");
	(void) printf("prev %p  next %p\n",
	       	      (POINTER)oc->prev,(POINTER)oc->next);
	print_curve(oc->curve);
	(void) printf("\n");
}		/*end print_o_curve*/

EXPORT void print_curve_with_orient(
	CURVE			*c,
	ORIENTATION		orient)
{
	print_orientation("The orientation is",orient,"\n");
	print_curve(c);
}		/*end print_curve_with_orient*/

EXPORT	void copy_o_curve(
	O_CURVE		*oc1,
	O_CURVE		*oc2)
{
	oc1->curve = oc2->curve;
	oc1->orient = oc2->orient;
}		/*end copy_o_curve*/


EXPORT	BOND	*random_bond_on_curve(
	CURVE		*c)
{
	BOND		*b;
	int		i, ith;
	double		th;

	th  = random01(c->interface);

	ith = (int) (th * c->num_points);

	for (i = 0, b = c->first;  i < ith;  ++i, b = b->next)
			;		/* ith bond chosen */

	return b;
}		/*end random_bond_on_curve*/


/*
*			i_move_closed_loop_node():
*
*	Moves node of a closed loop c from its current position to
*	the start point of bond b.
*
*	Warning: this may cause an error if applied to a partially
*	propagated curve in certain physics applications.
*/

EXPORT	boolean i_move_closed_loop_node(
	CURVE		*c,
	BOND		*b)
{
	NODE		*c_node;

	if (c->start != c->end)
	    return NO;
	if (b == c->first || b == NULL || is_bdry(c->start))
	    return YES;		/* Why call this routine for nothing */
				/* Don't move boundary node */

	c_node = c->start;

	
		/* Correctly link old node point */

	c->first->prev = c->last;
	c->last->next = c->first;

		/* reset new node */

	c_node->posn = b->start;
	c->first = b;
	c->last = b->prev;
	c->first->prev = NULL;
	c->last->next = NULL;

	c->interface->modified = YES;

	return YES;
}		/*end i_move_closed_loop_node*/

/*
*			merge_and_delete_nodes():
*
*	Moves all curves associated to n2 to node n1 and deletes n2.
*/

EXPORT	void merge_and_delete_nodes(
	NODE		*n1,
	NODE		*n2)
{
	CURVE		**c;

	if (n1 == n2 || n1 == NULL || n2 == NULL)
	{
	    return;
	}

	for (c = n2->in_curves; c && *c; c = n2->in_curves)
	    change_node_of_curve(*c,NEGATIVE_ORIENTATION,n1);

	for (c = n2->out_curves; c && *c; c = n2->out_curves)
	    change_node_of_curve(*c,POSITIVE_ORIENTATION,n1);

	(void) delete_node(n2);
}		/*end merge_and_delete_nodes*/


/*
*			update_num_points():
*
*	This routine updates the number of points on the curves of an
*	interface. The current version is only valid for 2 dimensions.
*/

EXPORT	void update_num_points(
	INTERFACE	*intfc)
{
	INTERFACE	*hold_intfc;
	INTERFACE	*current_interface(void);
	CURVE    	*c;

	hold_intfc = current_interface();
	set_current_interface(intfc);

	intfc->num_points = 0;
	(void) next_curve(intfc,NULL);
	while (next_curve(intfc,&c))
	{
		c->num_points = num_points_on_curve(c);
		intfc->num_points += c->num_points;
	}

	set_current_interface(hold_intfc);
}		/*end update_num_points*/

EXPORT	int num_points_on_curve(
	CURVE		*c)
{
	BOND		*b;
	int		npts;

	for (npts = 0, b=c->first; b!=NULL; b=b->next) ++npts;

	return npts + 1;		     /* num_pts = num_bonds+1 */
}		/*end num_points_on_curve*/



/*
*			i_attach_curve_to_node():
*
*	This routine attaches curve c1 to the node n at the point p of 
*       a bond b. If p is not an endpoint of the bond, it is inserted into
*       the bond.
*
*	If p is not the first or last point on c1 then a new curve c2 is 
*	created, and c2 is returned by the function.
*	In the opposite case, a null pointer is returned for c2.
*/

	
EXPORT	CURVE *i_attach_curve_to_node(
	CURVE		*c1,
	FT_POINT		*p,
	BOND		*b,
	NODE		*n)
{
	INTERFACE	*hold_intfc;
	INTERFACE	*intfc = c1->interface;
	NODE		*n1;
	CURVE		*c2;
	BOND		*bb;

	debug_print("top_unravel","Entered attach_curve_to_node()\n");
	if (debugging("top_unravel"))
	{
	    (void) printf("Attaching c %llu to n %llu at p (%g %g) of b %llu\n",
			  curve_number(c1),node_number(n),
			  Coords(p)[0],Coords(p)[1],
			  bond_number(b,c1->interface));
	    (void) printf("node -\n");	print_node(n);
	    (void) printf("bond -\n");	print_bond(b);
	    (void) printf("c1 -\n");	print_curve(c1);
	}
	hold_intfc = current_interface();
	set_current_interface(intfc);

		/* Break curves at node position */

	if ((b == c1->first) && (p == b->start))
	{
	    n1 = c1->start;
	    change_node_of_curve(c1,POSITIVE_ORIENTATION,n);
	    (void) delete_node(n1);
	    c2 = (CURVE *) NULL;
	}
	else if ((b == c1->last) && (p == b->end))
	{
	    n1 = c1->end;
	    change_node_of_curve(c1,NEGATIVE_ORIENTATION,n);
	    (void) delete_node(n1);
	    c2 = (CURVE *) NULL;
	}
	else
	{
	    if ((p != b->start) && (p != b->end))
	    {
	    	if (insert_point_in_bond(p,b,c1) != FUNCTION_SUCCEEDED)
	        {
	            screen("ERROR in i_attach_curve_to_node(), "
		           "insert_point_in_bond() failed\n");
	            clean_up(ERROR);
	        }
	    }
	    else if (separation(p,b->start,intfc->dim) <
	    			separation(p,b->end,intfc->dim))
	    	b = b->prev;

	    c2 = make_curve(negative_component(c1),
	    		    positive_component(c1),n,c1->end);
	    c2 = make_curve(0,0,n,c1->end);
	    c2->first = b->next;
	    c2->first->start = n->posn;
	    bond_length(c2->first) = separation(c2->first->start,
						c2->first->end,intfc->dim);
	    c2->first->prev = NULL;
	    c2->last = c1->last;
	    if (is_bdry(c1))
	    	set_is_bdry(c2);
	    else
	    	set_not_bdry(c2);

	    if (!delete_from_pointers(c1,&c1->end->in_curves))
	    {
	    	screen("ERROR in i_attach_curve_to_node(), "
	    	       "delete_from_pointers() failed\n");
		clean_up(ERROR);
	    }
	    if (!add_to_pointers(c1,&n->in_curves))
	    {
	    	screen("ERROR in i_attach_curve_to_node(), "
	    	       "add_to_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	    b->end = n->posn;
	    c1->last = b;
	    c1->end = n;
	    bond_length(c1->last) = separation(c1->last->start,
					       c1->last->end,intfc->dim);
	    b->next = NULL;

	    /* reset curve points */

	    c1->num_points = 2;
	    for (bb = c1->first;  bb != c1->last;  bb = bb->next)
	    	++c1->num_points;
	    c2->num_points = 2;
	    for (bb = c2->first;  bb != c2->last;  bb = bb->next)
	    	++c2->num_points;

	    intfc->num_points += 2;

	    if (debugging("top_unravel"))
	    {
	    	(void) printf("\nafter split curve in attach_..()\n");
	    	(void) printf("c1 -\n");	 print_curve(c1);
	    	(void) printf("c2 -\n");	 print_curve(c2);
	    	(void) printf("c1->end -\n");	 print_node(c1->end);
	    	(void) printf("c2->start -\n"); print_node(c2->start);
	    	(void) printf("node -\n");	 print_node(n);
	    	(void) printf("\n");
	    }
	
	}

	if (debugging("top_unravel"))
	{
		(void) printf("\tattach_curve_to_node()\n");
		(void) printf("node -\n");	print_node(n);
		(void) printf("c1 -\n");	print_curve(c1);
		(void) printf("c2 -\n");	print_curve(c2);
	}

	intfc->modified = YES;
	set_current_interface(hold_intfc);
	return c2;
}		/*end i_attach_curve_to_node*/

/*
*			rbl_after_move_node():
*
*	Recomputes the bond lengths of the bonds at NODE node,
*	presumably after the node position has been changed.
*	It also ensures that the first (last) bond of any out (in)
*	curve at node has start (end) point equal to node->posn.
*/


EXPORT void rbl_after_move_node(
	NODE		*node)
{
	CURVE		**c;
	int		dim = node->interface->dim;

	for (c = node->in_curves; c && *c; ++c)
	{
		(*c)->last->end = node->posn;
		set_bond_length((*c)->last,dim);
	}

	for (c = node->out_curves; c && *c; ++c)
	{
		(*c)->first->start = node->posn;
		set_bond_length((*c)->first,dim);
	}
}		/*end rbl_after_move_node*/

EXPORT int num_curves_at_node(
	NODE		*node,
	int		*pnum_in,
	int		*pnum_out)
{
	int		num_in, num_out, num_total;
	CURVE		**c, **c1;

	num_total = 0;

	for (num_in = 0, c = node->in_curves; c && *c; ++c, ++num_in)
		++num_total;
	
	for (num_out = 0, c = node->out_curves; c && *c; ++c, ++num_out)
	{
		for (c1 = node->in_curves; c1 && *c1; ++c1)
			if (*c == *c1) break;
		if ((c1 == NULL) || (*c1 != *c)) ++num_total;
	}
	if (pnum_in != NULL)
		*pnum_in = num_in;
	if (pnum_out != NULL)
		*pnum_out = num_out;
	return num_total;
}		/*end num_curves_at_node*/

/*
*		i_cut_curve():
*
**       This routine modifies a curve in the vicinity of a node by
*       shifting the node position and deleting certain nearby points.
*
*       It first removes all points of the curve that lie (strictly) between
*       the node Node_of(c,orient) and the point at the far end
*       (with respect to the orientation) of the bond bcut (which is
*       assumed to be non-NULL).  Care is taken to maintain the propagate
*       flags properly.  It then moves the node position to the position of
*       the point newp.
*
*/

EXPORT void i_cut_curve(
	FT_POINT           *newp,
	BOND            *bcut,
	CURVE           *c,
	ORIENTATION     orient)
{
	INTERFACE *intfc = c->interface;
	RECT_GRID *gr = &topological_grid(intfc);
	BOND            *b,*bcut_follower;
	NODE            *n;
	double           sc_b_len;
	double           min_sc_sep;

	min_sc_sep = MIN_SC_SEP(intfc);
	b = Bond_at_node(c,orient);
	bcut_follower = Following_bond(bcut,orient);
	while (Following_bond(b,orient) != bcut_follower)
	{
	    if (i_delete_point_adjacent_to_node(c,orient) == FUNCTION_FAILED)
	    {
	    	(void) printf("WARNING in i_cut_curve(), ");
                (void) printf("i_delete_point_adjacent_to_node failed\n");
		break;
	    }
	    b = Bond_at_node(c,orient);
	}
	        /* Shift the node position */
	n = Node_of(c,orient);
	Coords(n->posn)[0] = Coords(newp)[0];
	Coords(n->posn)[1] = Coords(newp)[1];
	b = Bond_at_node(c,orient);
	bond_length(b) = separation(b->start,b->end,intfc->dim);
	sc_b_len = scaled_bond_length(b,gr->h,intfc->dim);
	if ((sc_b_len < min_sc_sep) && (c->num_points >= 4))
	{
	    (void) i_delete_point_adjacent_to_node(c,orient);
	}
}	/* end i_cut_curve */


/*
*		change_node_of_curve():
*
*	Detaches curve from its node with respect to orient and attaches
*	curve to NODE node with orient orient.
*/

EXPORT void change_node_of_curve(
	CURVE		*curve,
	ORIENTATION	orient,
	NODE		*node)
{
	int		dim = curve->interface->dim;
	DEBUG_ENTER(change_node_of_curve)

	if (orient == POSITIVE_ORIENTATION)
	{
	    if (!delete_from_pointers(curve,&curve->start->out_curves))
	    {
	    	screen("ERROR in change_node_of_curve(), "
	    	       "delete_from_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	    if (!add_to_pointers(curve,&node->out_curves))
	    {
	    	screen("ERROR in change_node_of_curve(), "
	    	       "add_to_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	    curve->start = node;
	    curve->first->start = node->posn;
	    set_bond_length(curve->first,dim);
	}
	else
	{
	    if (!delete_from_pointers(curve,&curve->end->in_curves))
	    {
	    	screen("ERROR in change_node_of_curve(), "
	    	       "delete_from_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	    if (!add_to_pointers(curve,&node->in_curves))
	    {
	       	screen("ERROR in change_node_of_curve(), "
	    	       "add_to_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	    curve->end = node;
	    curve->last->end = node->posn;
	    set_bond_length(curve->last,dim);
	}
	DEBUG_LEAVE(change_node_of_curve)
}		/*end change_node_of_curve*/

EXPORT O_NODE *make_onode(
	NODE		*node)
{
	O_NODE		*onode;
	INTERFACE	*cur_intfc;
	CURVE		**c;
	int		num_curves;

	cur_intfc = current_interface();
	set_current_interface(node->interface);
	if ((onode = (O_NODE *) Store(sizeof(O_NODE))) == NULL)
	{
	    set_current_interface(cur_intfc);
	    return NULL;
	}
	onode->node = node;
	onode->prev = onode->next = NULL;
	num_curves = 0;
	for (c = node->in_curves; c && *c; ++c)
	    ++num_curves;
	for (c = node->out_curves; c && *c; ++c)
	    ++num_curves;
	
	onode->num_c = num_curves;
	if (num_curves == 0)
	{
	    onode->nc = NULL;
	    onode->nopp = NULL;
	    onode->pt = NULL;
	    onode->ang = NULL;
	    onode->orient = NULL;
	    set_current_interface(cur_intfc);
	    return onode;
	}

	onode->nc     = (CURVE **) Store(num_curves * sizeof(CURVE *));
	onode->nopp   = (NODE **)  Store(num_curves * sizeof(NODE *));
	onode->pt     = (FT_POINT **) Store(num_curves * sizeof(FT_POINT *));
	onode->ang    = (double *)  Store(num_curves * FLOAT);
	onode->orient = (ORIENTATION *)    Store(num_curves * INT);

	if (onode->orient == NULL) /* Out of space */
	{
	    set_current_interface(cur_intfc);
	    return NULL;
	}
	set_curves_at_onode(onode);
	set_current_interface(cur_intfc);
	return onode;
}		/*end make_onode*/


EXPORT	void print_onode_list(
	O_NODE		**on_list)
{
	O_NODE		*on;

	(void) printf("\n\t\tO_NODE list\n");
	for (on = *on_list;  on != NULL;  on = on->next)
		print_onode(on);
}		/*end print_onode_list*/

EXPORT	void print_onode(
	O_NODE		*on)
{
	FT_POINT		*p;
	int		i, dim;

	if (on == NULL)
	{
		(void) printf("O_NODE unallocated\n\n");
		return;
	}
	(void) printf("onode %p prev %p next %p\n",
		      (POINTER)on,(POINTER)on->prev,(POINTER)on->next);
	(void) printf("node - \n");	print_node(on->node);
	dim = on->node->interface->dim;
	for (i = 0;  i < on->num_c;  ++i)
	{
	    (void) printf("curve on->nc[%d] %llu ",i,curve_number(on->nc[i]));
	    print_orientation("orient",on->orient[i],"\n");
	    if (on->ang != NULL) print_angle("\tang",on->ang[i],"\n");
	    p = (on->nopp[i])->posn;
	    (void) printf("\topp_node %llu ",node_number(on->nopp[i]));
	    print_general_vector("posn ",Coords(p),dim,"\n");
	    p = on->pt[i];
	    print_general_vector("\tpt ",Coords(p),dim,"\n");
	    (void) printf("\n");
	}
	if (dim == 2)
	{
		(void) printf("Components about onode %p\n",(POINTER)on);
		for (i = 0; i < on->num_c; ++i)
		{
			if (on->orient[i] == POSITIVE_ORIENTATION)
			{
				(void) printf("%d-%d-%d ",
					      positive_component(on->nc[i]),i,
					      negative_component(on->nc[i]));
			}
			else
			{
				(void) printf("%d-%d-%d ",
					      negative_component(on->nc[i]),i,
					      positive_component(on->nc[i]));
			}
		}
	}
	(void) printf("\n\n");
}		/*end print_onode*/

LIB_LOCAL void set_curves_at_onode(
	O_NODE		*onode)
{
	NODE		*node = onode->node;
	CURVE		**c;
	double		a;
	double		*np;
	NODE		*m;
	FT_POINT		*p;
	CURVE		*c1;
	ORIENTATION	orient;
	int		num_curves;
	int		i,j;

	num_curves = onode->num_c;
	i = 0;
	np = Coords(node->posn);
	for (c = node->in_curves; c && *c; ++c)
	{
	    onode->nc[i] = *c;
	    onode->orient[i] = NEGATIVE_ORIENTATION;
	    onode->pt[i] = (*c)->last->start;
	    onode->nopp[i] = (*c)->start;
	    onode->ang[i] = angle(Coords(onode->pt[i])[0]-np[0],
			          Coords(onode->pt[i])[1]-np[1]);
	    ++i;
	}
	for (c = node->out_curves; c && *c; ++c)
	{
	    onode->nc[i] = *c;
	    onode->orient[i] = POSITIVE_ORIENTATION;
	    onode->pt[i] = (*c)->first->end;
	    onode->nopp[i] = (*c)->end;
	    onode->ang[i] = angle(Coords(onode->pt[i])[0] - np[0],
			          Coords(onode->pt[i])[1] - np[1]);
	    ++i;
	}
	for (i = 0;  i < num_curves-1;  ++i)
	{
	    for (j = i+1;  j < num_curves;  ++j)
	    {
	    	if (onode->ang[j] < onode->ang[i])
	    	{
	    	    a      = onode->ang[i];
	    	    m      = onode->nopp[i];
	    	    p      = onode->pt[i];
	    	    c1     = onode->nc[i];
	    	    orient = onode->orient[i];

	    	    onode->ang[i]    = onode->ang[j];	
	    	    onode->nopp[i]   = onode->nopp[j];
	    	    onode->pt[i]     = onode->pt[j];
	    	    onode->nc[i]     = onode->nc[j];
	    	    onode->orient[i] = onode->orient[j];

	    	    onode->ang[j]    = a;
	    	    onode->nopp[j]   = m;
	    	    onode->pt[j]     = p;
		    onode->nc[j]     = c1;
		    onode->orient[j] = orient;
		}
	    }
	}
}		/*end set_curves_at_onode*/



/*
*		intersect_ray_with_sector():
*
*	Finds the intersection (if any) of the ray through the point
*	pt0 with direction t0 and the sector containing the point
*	pt1 defined by the directions t1[0] (and t1[1] if dim = 3).
*/

EXPORT	int intersect_ray_with_sector(
	FT_POINT		*pt0,
	FT_POINT		*pt1,
	double		*t0,
	double		**t1,
	double		*coords,
	int		dim)
{
	double		*p0, *p1;
	double		dp[MAXD], num[MAXD], den, s[MAXD];
	double		v0[MAXD], v1[MAXD], vd[MAXD];
	double		weight;
	int		i;

	debug_print("iray","Entered intersect_ray_with_sector()\n");
	p0 = Coords(pt0);		p1 = Coords(pt1);
	if (debugging("iray"))
	{
		char mesg[80];
		print_general_vector("Vertex 0 = ",p0,dim,"\n");
		print_general_vector("Direction 0 = ",t0,dim,"\n");
		print_general_vector("Vertex 1 = ",p1,dim,"\n");
		for (i = 0; i < dim-1; ++i)
		{
			(void) sprintf(mesg,"Direction 1,%d = ",i);
			print_general_vector(mesg,t1[i],dim,"\n");
		}
		if (dim == 2)
		{
			double ang;
			ang = angle(t0[0],t0[1]);
			print_angle("Direction angle 0 = ",ang,"\n");
			ang = angle(t1[0][0],t1[0][1]);
			print_angle("Direction angle 1 = ",ang,"\n");
		}
	}
	for (i = 0; i < dim; ++i) dp[i] = p0[i] - p1[i];
	(void) vector_product(dp,t1[0],v0,dim);
	(void) vector_product(dp,t0,v1,dim);
	(void) vector_product(t1[0],t0,vd,dim);
	switch (dim)
	{
	case 2:
		num[0] = v0[0];
		num[1] = v1[0];
		den = vd[0];
		if (debugging("iray"))
		{
			(void) printf("num[0] = %g, num[1] = %g, den = %g\n",
				      num[0],num[1],den);
		}
		break;
	case 3:
		num[0] = scalar_product(v0,t1[1],dim);
		num[1] = scalar_product(v1,t1[1],dim);
		num[2] = scalar_product(v0,t0,dim);
		den = scalar_product(vd,t1[1],dim);
		break;
	default:
		debug_print("iray","Left intersect_ray_with_sector(), ans = NO\n");
		return NO;
	}
	for (i = 0; i < dim; ++i)
	{
		if (num[i]*den <= 0.0)
		{
			debug_print("iray",
			      "Left intersect_ray_with_sector(), ans = NO\n");
			return NO;
		}
		s[i] = num[i]/den;
		if (debugging("iray"))
			(void) printf("s[%d] = %g\n",i,s[i]);
	}
	weight = (dim == 2) ? 0.5 : 1.0/3.0;
	for (i = 0; i < dim; ++i)
		coords[i] = weight*(p0[i] + s[0]*t0[i] + p1[i] + s[1]*t1[0][i]);
	if (dim == 3)
	{
		for (i = 0; i < dim; ++i)
			coords[i] += weight * (p1[i] + s[2]*t1[1][i]);
	}
	if (debugging("iray"))
		print_general_vector("Intersection of rays = ",
				     coords,dim,"\n");
	debug_print("iray","Left intersect_ray_with_sector(), ans = YES\n");
	return YES;
}		/*end intersect_ray_with_sector*/

/*
*		intersect_ray_with_curve():
*
*	Finds the intersection if any of the ray through the point
*	p with direction v and the segment of the curve c beginning
*	at bond bs and ending at bond be.  If bs is NULL the segment
*	starts at the bond at the node of c with orientation c_orient,
*	and if be is NULL the segment terminates at the opposite end of
*	c with respect to the orientation c_orient. Returns YES if
*	successful along with the point of intersection pint and the
*	bond bint on which the intersection lies.
*
*	We express the ray as pt + t*v (pt and v uni_arrays, t scalar).
*	The bond is (1-s)*start + s*end (start and end uni_arrays, s scalar).
*	We then wish to determine s and t (giving pint) subject to the
*	constraints 0 <= s <= 1, and 0 <= t.  Setting the above expressions
*	equal and taking appropriate cross products, it is straight-forward
*	to derive the expressions s = num_s/den and t = num_t/den (see
*	below for the definitions of num_s and num_t).
*
*	Some robustness is provided at the start/end of the curve by delta and
*	eps resp.  We thus let -delta <= s <= (1+eps), with delta and eps
*	equal to zero except at the start/end of the curve.  If we subtract
*	the average of the endpoints, ie 0.5*(1 + eps - delta), from this
*	equation and replace s by num_s/den, we can recast as a single
*	test as below.
*/
/*TODO: Upgrade to 3D (...ray_with_hypersurface) */
EXPORT	int intersect_ray_with_curve(
	FT_POINT		*pt,
	double		*v,
	BOND		*bs,
	BOND		*be,
	CURVE		*c,
	ORIENTATION	c_orient,
	BOND		**bint,
	FT_POINT		*pint)
{
	BOND		*bb;
	double		s, t, tmin;
	double		q[MAXD], p[MAXD];
	double		num_s, num_t, den, den1;
	double		eps, delta;
	double		a, b, t1, t2;
	int		dim = c->interface->dim;
	int		i;
	int		status = NO;
	const double	meps = MACH_EPS;

	if (bs == NULL)
	    bs = Bond_at_node(c,c_orient);
	*bint = NULL;
	tmin = HUGE_VAL;
	for (bb = bs; bb; bb = Following_bond(bb,c_orient))
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	q[i] = Coords(bb->end)[i] - Coords(bb->start)[i];
	    	p[i] = Coords(pt)[i] - Coords(bb->start)[i];
	    }
	    (void) vector_product(q,v,&den,dim);
	    (void) vector_product(p,q,&num_t,dim);
	    if (den*num_t < 0.0)
		continue;		/* t < 0 */
	    (void) vector_product(p,v,&num_s,dim);

	    delta = (bb->prev) ? 0.0 : 0.0001; /*TOLERANCE*/
	    eps   = (bb->next) ? 0.0 : 0.0001; /*TOLERANCE*/

	    if (fabs(den) < meps)
	    {			/* bond and ray parallel */
	    	/* check for coincidence, ie bond & ray on same line*/
	    	den1 = scalar_product(v,q,dim);
	    	if (fabs(den) >= 0.0001*fabs(den1)) /*TOLERANCE*/
		    continue;

		/* dot equation with q and solve for t */
		a = scalar_product(q,q,dim)/den1;
		b = scalar_product(p,q,dim)/den1;
		t1 = -(a*delta + b);		/* take s = 0 */
		t2 = a*(1.0 + eps) - b;		/* take s = 1 */
		if (Between(0.0,t1,t2))
		{
		    /* endpoints of bond on opposite sides
		     * of start point pt, take t = 0 */
		    for (i = 0; i < dim; ++i)
		    	Coords(pint)[i] = Coords(pt)[i];
		    *bint = bb;
		    status = YES;
		    break;
		}
		else if ((t = min(t1,t2)) > 0.0)
		{
		    /* endpoints of bond on same (t > 0) side of
		     * start point, take closer (smaller) t */
		    if (status == YES && t > tmin)
			continue;
		    tmin = t;
		    for (i = 0; i < dim; ++i)
		        Coords(pint)[i] = Coords(pt)[i] + t*v[i];
		    *bint = bb;
		    status = YES;
		}
	    }


	    if (fabs(den) < meps || (
	    	fabs(num_s - 0.5*(1.0 + eps - delta)*den) >
					0.5*(1.0 + eps + delta)*fabs(den)))
	    {
	    	den1 = scalar_product(v,q,dim);
	    	if (fabs(den) >= 0.0001*fabs(den1)) /*TOLERANCE*/
		    continue;
	    	a = scalar_product(q,q,dim)/den1;
	    	b = scalar_product(p,q,dim)/den1;
	    	t1 = -a*delta -b;
	    	t2 = a*(1.0 + eps) -b;
		if (Between(0.0,t1,t2))
		{
		    for (i = 0; i < dim; ++i)
		    	Coords(pint)[i] = Coords(pt)[i];
		    *bint = bb;
		    status = YES;
		    break;
		}
		else if ((t = min(t1,t2)) > 0.0)
		{
		    if (status == YES && t > tmin)
			continue;
		    tmin = t;
		    for (i = 0; i < dim; ++i)
		        Coords(pint)[i] = Coords(pt)[i] + t*v[i];
		    *bint = bb;
		    status = YES;
		}
	    }
	    else if (fabs(num_s - 0.5*(1.0 + eps - delta)*den) <=
	    			0.5*(1.0 + eps + delta)*fabs(den))
	    {               /* make sure that s is valid */
	    	s = num_s/den;
	    	t = num_t/den;
	    	if (status == NO || t < tmin)
	    	{
	    	    tmin = t;
	    	    *bint = bb;
	    	    for (i = 0; i < dim; ++i)
	    	    {
	    	        Coords(pint)[i] = (1.0 - s) * Coords(bb->start)[i] +
	    	                                  s * Coords(bb->end)[i];
	    	    }
	    	    status = YES;
	    	}
	    }
	    if (bb == be) break;
	}
	return status;
}		/*end intersect_ray_with_curve*/

/*
*			intersect_ray_with_boundary():
*
*	Finds the intersection of the ray through the point pt with direction
*	n with the block given by L[i] <= p[i] <= U[i], i = 0,..,dim-1.
*/

EXPORT	int intersect_ray_with_boundary(
	double		*pt,
	double		*n,
	double		*L,
	double		*U,
	double		*ans,
	int		dim)
{
	int		status = NO;
	int		i, j, k, l;
	double		*B;
	double		d, s;
	double		ptmp[MAXD];
	double		m = mag_vector(n,dim);
	double		D[MAXD];

	for (i = 0; i < dim; ++i)
		D[i] = U[i] - L[i];
	d = 100.0*mag_vector(D,dim);
	for (l = 0; l < 2; ++l)
	{
	    B = (l == 0) ? L : U;
	    for (i = 0; i < dim; ++i)
	    {
	    	if ((m*fabs(B[i] - pt[i]) < d*fabs(n[i])) &&
		    ((B[i] - pt[i])*n[i] > 0.0))
	    	{
		    s = (B[i] - pt[i])/n[i];
		    for (j = 0; j < dim; ++j)
		    	ptmp[j] = pt[j] + s*n[j];
		    for (j = 1; j < dim; ++j)
		    {
		    	k = (i+j)%dim;
		    	if (ptmp[k] < L[k] || ptmp[k] > U[k])
		    	    break;
		    }
		    if (j == dim)
		    {
		    	status = YES;
		    	d = m*s;
		    	for (j = 0; j < dim; ++j)
		    	    ans[j] = ptmp[j];
		    }
		}
	    }
	}
	return status;
}		/*end intersect_ray_with_boundary*/



/*
*			nearest_boundary():
*
*	Finds the nearest boundary to the point with coordinates coords.
*	Returns LEFT_BDRY, RIGHT_BDRY, UPPER_BDRY, LOWER_BDRY, ZMIN_BDRY,
*	ZMAX_BDRY.
*	The "boundaries" are defined to be the sides of the square
*	with lower left corner gr->L[0], gr->L[1], upper right corner
*	gr->U[0], gr->U[1] in two dimensions, and a similar cube or solid
*	in three dimensions.
*/


EXPORT BDRY_SIDE nearest_boundary(
	double		*coords,
	RECT_GRID	*gr)
{
	double	  d[2*MAXD], min_dist;
	double	  *L = gr->VL, *U = gr->VU;
	int	  i, imin, dim = gr->dim;
	BDRY_SIDE side;

	imin = -1;
	side = NOT_A_BDRY;
	for (i = 0; i < dim; ++i)
	{
	    if (coords[i] > U[i])
	    {
	    	imin = 2*i;
	    	break;
	    }
	    if (coords[i] < L[i])
	    {
	    	imin = 2*i+1;
	    	break;
	    }
	}
	if (imin == -1)
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	d[2*i]   = U[i] - coords[i];
	    	d[2*i+1] = coords[i] - L[i];
	    }
	    imin = 0;
	    min_dist = d[0];
	    for (i = 1; i < 2*dim; ++i)
	    {
	    	if (d[i] < min_dist)
	    	{
	    	    min_dist = d[i];
	    	    imin = i;
	    	}
	    }
	}
	switch (imin)
	{
	case 0:
	    side = RIGHT_BDRY;
	    break;
	case 1:
	    side = LEFT_BDRY;
	    break;
	case 2:
	    side = UPPER_BDRY;
	    break;
	case 3:
	    side = LOWER_BDRY;
	    break;
	case 4:
	    side = ZMAX_BDRY;
	    break;
	case 5:
	    side = ZMIN_BDRY;
	    break;
	}
	return side;
}		/*end nearest_boundary*/

EXPORT	BDRY_SIDE	boundary_side(
	const double     *p,
	const RECT_GRID *gr,
	double           eps)
{
	static const BDRY_SIDE side[] = { LEFT_BDRY,
			                  RIGHT_BDRY,
			                  LOWER_BDRY,
			                  UPPER_BDRY,
			                  ZMIN_BDRY,
			                  ZMAX_BDRY};
	const double *L = gr->L, *U = gr->U;
	int	    i, dim = gr->dim;

	for (i = 0; i < dim; ++i)
	{
	    if (fabs(p[i] - L[i]) < eps)
		return side[2*i];
	    if (fabs(p[i] - U[i]) < eps)
		return side[2*i+1];
	}
	return NOT_A_BDRY;
}		/*end boundary_side*/


/*
*			nearest_boundary_point():
*
*	Finds the nearest boundary point to the point with coordinates
*	x, y.  Sets x_on, y_on to the nearest point.  The "boundaries" are
*	defined to be the sides of the square with lower left corner
*	gr->L[0], gr->L[1] and upper right corner gr->U[0], gr->U[1].
*/

EXPORT void nearest_boundary_point(
	double		*coords,
	double		*coords_on,
	RECT_GRID	*gr)
{
	double d[2*MAXD], min_dist;
	double *L = gr->VL, *U = gr->VU;
	int   i, imin, dim = gr->dim;
	boolean  interior = YES;

	for (i = 0; i < dim; ++i)
	{
	    if (coords[i] < L[i])
	    {
	    	coords_on[i] = L[i];
	    	interior = NO;
	    }
	    else if (coords[i] > U[i])
	    {
	    	coords_on[i] = U[i];
	    	interior = NO;
	    }
	    else
	    	coords_on[i] = coords[i];
	}
	if (interior)
	{
	    for (i = 0; i < dim; ++i)
	    {
	    	d[2*i]   = U[i] - coords[i];
	    	d[2*i+1] = coords[i] - L[i];
	    }
	    imin = 0;
	    min_dist = d[0];
	    for (i = 1; i < 2*dim; ++i)
	    {
	    	if (d[i] < min_dist)
	    	{
	    	    min_dist = d[i];
	    	    imin = i;
	    	}
	    }
	    coords_on[imin/2] = (imin%2) ? L[imin/2] : U[imin/2];
	}
}		/*end nearest_boundary_point*/

LIB_LOCAL	boolean i_is_subdomain_boundary(
	HYPER_SURF	*hs)
{
	int		i, j;
	INTERFACE	*intfc;

	if (!Boundary_hs(hs))
	    return NO;
	intfc = hs->interface;
	rect_bdry_side_for_hyper_surf(&i,&j,hs,&topological_grid(intfc));
#if defined(USE_OVERTURE)
        return (rect_boundary_type(intfc,i,j) == SUBDOMAIN_BOUNDARY ||
                rect_boundary_type(intfc,i,j) == AMR_SUBDOMAIN_BOUNDARY) ? YES : NO;
#else	
	return (rect_boundary_type(intfc,i,j) == SUBDOMAIN_BOUNDARY) ? YES : NO;
#endif /* if defined(USE_OVERTURE) */
}		/*end i_is_subdomain_boundary*/

LIB_LOCAL	boolean i_is_subdomain_node(
	NODE		*node)
{
	CURVE		**c;
	boolean		interior;
	boolean		subdomain;

	if (!is_bdry(node))
	    return NO;

	interior = NO;
	subdomain = NO;
	for (c = node->in_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c))
		interior = YES;
	    if (is_subdomain_boundary(Hyper_surf(*c)))
		subdomain = YES;
	}
	for (c = node->out_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c))
		interior = YES;
	    if (is_subdomain_boundary(Hyper_surf(*c)))
		subdomain = YES;
	}
	return ((subdomain == YES) && (interior == YES)) ? YES : NO;
}		/*end i_is_subdomain_node*/


/*TODO: delete this function*/
LIB_LOCAL	boolean i_is_virtual_fixed_node(
	NODE		*node)
{
	CURVE		**c;
	boolean		subdomain;

	if (!is_bdry(node))
	    return NO;
	for (c = node->in_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c))
		return NO;
	    if (is_subdomain_boundary(Hyper_surf(*c)))
		subdomain = YES;
	}
	for (c = node->out_curves; c && *c; ++c)
	{
	    if (!is_bdry(*c))
		return NO;
	    if (is_subdomain_boundary(Hyper_surf(*c)))
		subdomain = YES;
	}

	return subdomain;
}		/*end i_is_virtual_fixed_node*/


EXPORT void delete_list(
	POINTER		**list)
{
	while (*list)
	{
	    if (!delete_from_pointers(*list[0],list))
	    {
	    	screen("ERROR in delete_list(), "
	    	       "delete_from_pointers() failed\n");
	    	clean_up(ERROR);
	    }
	}
}		/*end delete_list*/

EXPORT	void print_int_vector(
	const char	*mesg,
	const int	*v,
	int		dim,
	const char	*end)
{
	int		i;

	if (mesg != NULL) (void) printf("%s",mesg);
	(void) printf("(");
	for (i = 0; i < dim; ++i)
		(void) printf("%d%s",v[i],(i==(dim-1)) ? ")" : ", ");
	(void) printf("%s",end);
}		/*end print_int_vector*/


/*
*		Debugging routines for direct plotting of the interface
*/


#define NOBINARY
#include <plotdecs.h>

#undef PLOTFILE
#define PLOTFILE plotfile

EXPORT	void plot_interface(
	INTERFACE	*intfc,
	const char	*fname,
	int		*step,
	int		*ctr,
	const char	*title)
{
	RECT_GRID	*gr = &topological_grid(intfc);
	FILE		*plotfile = NULL;
	char		lfname[120];
	char		num[10];
	double		h[MAXD];
	int 		i, dim = intfc->dim;
	BOND		*b;
	CURVE		**c;

	if (intfc->dim != 2) /* Only 2d supported at present */
	    return;

	(void) strcpy(lfname,fname);
	if (step != NULL)
	{
	    (void) sprintf(num,".%d",*step);
	    (void) strcat(lfname,num);
	}
	if (ctr != NULL)
	{
	    (void) sprintf(num,".%d",*ctr);
	    (void) strcat(lfname,num);
	}

	if ((plotfile = fopen(lfname,"w")) == NULL)
	{
	    screen("ERROR in plot_interface(), can't open %s\n",fname);
	    clean_up(ERROR);
	}
	if (debugging("nobuf"))
	    setbuf(plotfile,NULL);

	for (i = 0; i < dim; ++i)
	    h[i] = 0.1*(gr->U[i] - gr->L[i]);/*TOLERANCE*/

	fopenpl(plotfile);
	fwindow(plotfile,gr->L[0] - 0.3*h[0],gr->L[1] - 0.3*h[1],
	        gr->U[0] + 0.3*h[0],gr->U[1] + 0.3*h[1]);
	fviewport(plotfile,0.0,0.0,1.0,1.0);

	fset_color_from_table(plotfile,1);
	if (title != NULL && title[0] != '\0')
	{
	    fmove(plotfile,gr->L[0],gr->U[1]+0.05*h[1]);
	    flabel(plotfile,title);
	}
	for (c = intfc->curves; c && *c;  ++c)
	{
	    fset_hyper_surf_color(plotfile,Hyper_surf(*c));
	    fmove(plotfile,Coords((*c)->first->start)[0],
			   Coords((*c)->first->start)[1]);
	    for (b = (*c)->first;  b != NULL;  b = b->next)
	    	fcont(plotfile,Coords(b->end)[0],Coords(b->end)[1]);
	}
	ferase(plotfile);
	fclosepl(plotfile);
	(void) fclose(plotfile);
}		/*end plot_interface*/

EXPORT  void invert_tri(
        TRI *tri)
{
        FT_POINT *ptmp;
        TRI_NEIGHBOR nb_tmp;
        ptmp = Point_of_tri(tri)[1];
        Point_of_tri(tri)[1] = Point_of_tri(tri)[2];
        Point_of_tri(tri)[2] = ptmp;
        nb_tmp = tri->neighbor[0];
        tri->neighbor[0] = tri->neighbor[2];
        tri->neighbor[2] = nb_tmp;
        set_normal_of_tri(tri);
}       /* end invert_tri */

EXPORT  void i_invert_surface(
        SURFACE *surf)
{
        CURVE **c;
        TRI *t;
	COMPONENT comp_tmp;
        for (c = surf->pos_curves; c && *c; ++c)
            invert_curve(*c);
        for (c = surf->neg_curves; c && *c; ++c)
            invert_curve(*c);

        for (t = first_tri(surf); !at_end_of_tri_list(t,surf); t = t->next)
            invert_tri(t);
        comp_tmp = negative_component(surf);
        negative_component(surf) = positive_component(surf);
        positive_component(surf) = comp_tmp;
}       /* end i_invert_surface */
