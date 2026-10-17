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
*				zoom.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains code to rotate, translate, and clip an interface.
*/



#include <intfc/iloc.h>

	/* LOCAL Function Declarations */
LOCAL	boolean	is_identity_matrix(double**,int);
LOCAL	void	calculate_box(double*,double*,double**,double**,double**,int);
LOCAL	void	rotate_interface(INTERFACE*,double*,double**);
LOCAL	void	rotate_point(double*,double*,double**,double*,int);
#if defined(TWOD)
LOCAL	boolean	exterior_curve(CURVE*,RECT_GRID*);
LOCAL	boolean	exterior_point(FT_POINT*,RECT_GRID*);
LOCAL	void	insert_cuts_and_bdry2d(INTERFACE*,double**);
#endif /* defined(TWOD) */

/*
*			i_zoom_interface():
*
*	A given interface is modified by clipping. The two points
*	are the coordinates of opposite corners of a rectangle.
*	Curves are clipped at the boundaries of the rectangle and
*	segments falling outside of the rectangle are deleted.
*	Finally the rectangular grid of the clipped interface is
*	reset to the clipped values.
*/

/*ARGSUSED*/
EXPORT INTERFACE *i_zoom_interface(
	INTERFACE	*given_intfc,
	RECT_GRID	*gr,
	double		*L,
	double		*U,
	double		**Q)
{
	INTERFACE	*cur_intfc;
	INTERFACE	*zoom_intfc;
	RECT_GRID	*t_gr;
	int		dim = given_intfc->dim;
	int		i, j;
	double		**Qi = NULL;
	static double	**pc = NULL;

	debug_print("zoom","Entered zoom_interface()\n");

	cur_intfc = current_interface();
	if ((zoom_intfc = copy_interface(given_intfc)) == NULL)
	{
		Error(ERROR,"Unable to copy interface.");
		clean_up(ERROR);
	}

	if (debugging("zoom"))
	{
		(void) output();
		(void) printf("INTERFACE before zoom:\n\n");
		print_interface(zoom_intfc);
	}

	if (Q != NULL)
	{	
		static	double** M = NULL;
		
		if (M == NULL)
			bi_array(&M,MAXD,MAXD,FLOAT);

		Qi = M;
		for (i = 0; i < dim; i++)
			for (j = 0; j < dim; j++)
				Qi[i][j] = Q[j][i];
	}

	if (pc == NULL)
		bi_array(&pc,MAXNCORNERS,MAXD,FLOAT);

	calculate_box(L,U,pc,Q,Qi,dim);

	/* Shrink topological grid to cutting boundary */
	t_gr = &topological_grid(zoom_intfc);
	rotate_and_zoom_rect_grid(t_gr,L,U,Q);
	switch(dim)
	{
	case 1:
	    /* TODO */
	    return NULL;
#if defined(TWOD)
	case 2:
	    insert_cuts_and_bdry2d(zoom_intfc,pc);
	    clip_interface2d(zoom_intfc);
	    break;
#endif /* defined(TWOD) */
	case 3:
	    /* TODO */
	    return NULL;
	}
	rotate_interface(zoom_intfc,pc[0],Qi);

	if (set_boundary(zoom_intfc,t_gr,component(pc[0],given_intfc),
			 grid_tolerance(gr) != FUNCTION_SUCCEEDED))
	{
	    screen("ERROR in i_zoom_interface(), set_boundary failed\n");
	    clean_up(ERROR);
	}
	set_current_interface(cur_intfc);

	if (debugging("zoom"))
	{
	    (void) printf("INTERFACE after zoom:\n\n");
	    print_interface(zoom_intfc);
	}
	debug_print("zoom","Leaving zoom_interface()\n");
	return zoom_intfc;
}		/*end i_zoom_interface*/

EXPORT	void rotate_and_zoom_rect_grid(
	RECT_GRID *gr,
	double	  *L,
	double	  *U,
	double	  **Q)
{
	double	     **Qi = NULL;
	int	     i, j, dim = gr->dim;
	static double **pc = NULL;

	if (pc == NULL)
	    bi_array(&pc,MAXNCORNERS,MAXD,FLOAT);
	if (Q != NULL)
	{	
	    static double** M = NULL;
		
	    if (M == NULL)
	    	bi_array(&M,MAXD,MAXD,FLOAT);

	    Qi = M;
	    for (i = 0; i < dim; i++)
	    	for (j = 0; j < dim; j++)
	    	    Qi[i][j] = Q[j][i];
	}
	calculate_box(L,U,pc,Q,Qi,dim);
	for (i = 0; i < dim; i++)
	{
	    gr->gmax[i] = irint(_scaled_separation(pc[1<<i],pc[0],gr->h,dim));
	    if (gr->gmax[i] < 1)
		gr->gmax[i] = 1;
	}
	if (Qi != NULL)
	{
	    rotate_point(L,U,Qi,gr->U,dim);
	    rotate_point(L,gr->GU,Qi,gr->GU,dim);
	    rotate_point(L,gr->GL,Qi,gr->GL,dim);
	}
	set_rect_grid(L,gr->U,gr->GL,gr->GU,gr->lbuf,gr->ubuf,gr->gmax,
		      dim,&gr->Remap,gr);
}		/*end rotate_and_zoom_rect_grid*/

LOCAL void calculate_box(
	double		*L,
	double		*U,
	double		**rot_box,
	double		**Q,
	double		**Qi,
	int		dim)
{
	double		corner[2][MAXD];
	double		new_U[MAXD];
	int		i, j, imax = (1<<dim);

	/*  rotate upper corner backwards  */
	if (Qi != NULL)
		rotate_point(L, U, Qi, new_U, dim);
	else
		for (j = 0; j < dim; j++) new_U[j] = U[j];

	for (j = 0; j < dim; j++)
	{
		corner[0][j] = rot_box[0][j] = L[j];
		corner[1][j] = new_U[j];
		rot_box[imax-1][j] = U[j];
	}

	/*  find corners to be rotated  */
	for (i = 1; i < imax-1; i++)
		for (j = 0; j < dim; j++)
			rot_box[i][j] = corner[(i>>j)%2][j];

	/*  rotate corners   */
	if (Q != NULL)
	{
		for (i = 1; i < imax-1; i++)
			rotate_point(L, rot_box[i],
				     Q, rot_box[i], dim);
	}
}		/*end calculate_box*/


/*
*			rotate_interface():
*
* 	TODO THREED
*/

LOCAL void rotate_interface(
	INTERFACE	*intfc,
	double		*origin,
	double		**Q)
{
	BOND		*b;
	CURVE	 	**c;
	NODE  		**n;
	FT_POINT		*p;
	int		dim = intfc->dim;
	
	if (is_identity_matrix(Q,dim) == YES)
	    return;

	for (n = intfc->nodes; *n; n++)
	{
		p = (*n)->posn;
		rotate_point(origin,Coords(p),Q,Coords(p),dim);
	}

	for (c = intfc->curves; *c; c++)
	{
		for (b = (*c)->first; b != (*c)->last; b = b->next)
		{
			p = b->end;
			rotate_point(origin,Coords(p),Q,Coords(p),dim);
		}
	}

	if (dim == 3)
	{
		/* TODO */
		screen("ERROR in rotate_interface(), 3D code needed\n");
		clean_up(ERROR);
	}
}		/*end rotate_interface*/

LOCAL void rotate_point(
	double		*origin,
	double		*old_pt,
	double		**Q,
	double		*new_pt,
	int		dim)
{
	int		i, j;
	double		diff_vec[MAXD];

	for (i = 0; i < dim; i++)
		diff_vec[i] = old_pt[i] - origin[i];

	for (i = 0; i < dim; i++)
	{
		new_pt[i] = origin[i];
		for (j = 0; j < dim; j++)
			new_pt[i] += Q[i][j] * diff_vec[j];
	}
}		/*end rotate_point*/


LOCAL	boolean is_identity_matrix(
	double		**Q,
	int		dim)
{
	int	    i, j;
	const double IDENTITY_TOL = 1.0e-10; /*TOLERANCE*/

	if (Q == NULL)
	    return YES;
	for (i = 0; i < dim; i++)
	    if (fabs(1.0 - Q[i][i]) > IDENTITY_TOL)
		return NO;

	for (i = 0; i < dim; i++)
	{
	    for (j = 0; j < dim; j++)
	    {
	    	if (i == j)
		    continue;
	    	if (fabs(Q[i][j]) > IDENTITY_TOL)
		    return NO;
	    }
	}
	return YES;
}		/*end is_identity_matrix*/

#if defined(TWOD)

/*
 *			exterior_point():
 *
 *	Determines if a given point p is exterior to a given domain.
 *
 */

LOCAL	boolean exterior_point(
	FT_POINT		*p,
	RECT_GRID	*grid)
{
	int   i,dim = grid->dim;
	double p_i;

	for (i = 0; i < dim; i++)
	{
	    p_i = Coords(p)[i];
	    if (p_i < grid->L[i] || p_i > grid->U[i])
		return YES;
	}
	return NO;
}		/*end exterior_point*/

/*
*			exterior_curve():
*
*	Determines if a given curve is exterior to a given domain.
*	The logic used in this code is not robust enough and is
*	a place to spot for bugs.
*
*/

LOCAL 	boolean exterior_curve(
	CURVE		*c,
	RECT_GRID	*grid)
{
	if ((exterior_point(c->start->posn,grid) == YES) ||
	    (exterior_point(c->end->posn,grid) == YES))
		return YES;

	if ((c->num_points > 2) && is_bdry(c->start) && is_bdry(c->end) &&
		(exterior_point(c->first->end,grid) == YES))
	    return YES;

	return NO;
}		/*end exterior_curve*/


/*
*			clip_interface2d():
*
* 		clips all the exterior objects.
*/

EXPORT void clip_interface2d(
	INTERFACE	*intfc)
{
	CURVE		**c;
	NODE		**n;
	RECT_GRID	*grid = &topological_grid(intfc);
	int		dim = intfc->dim,i;
	double 		eps = HUGE_VAL;

	for (i = 0; i < dim; i++)
	    eps = min(eps,grid->h[i]);
	eps *= 0.01;/*TOLERANCE*/

	for (i = 0; i < dim; i++)
	{
	    grid->L[i] -= eps;
	    grid->U[i] += eps;
	}

			/* delete curve: */
	for(;;)
	{
	    for(c = intfc->curves;*c;c++)
	    {
	    	if (exterior_curve(*c,grid) == YES)
		    break;
	    }
	    if (*c)
	    {
	    	(void) delete_curve(*c);
	    }
	    else
		break;
	}

			/* delete node: */
	for(;;)
	{
	    for(n = intfc->nodes;*n;n++)
	    {
	    	if ((*n)->in_curves != NULL || (*n)->out_curves != NULL)
	    		continue;
	    	if (exterior_point((*n)->posn,grid) == YES)
	    	    break;
	    }
	    if (*n)
	    {
	    	(void) delete_node(*n);
	    }
	    else break;
	}
}		/*end clip_interface2d*/


LOCAL 	void insert_cuts_and_bdry2d(
	INTERFACE	*intfc,	/* an orginal intfc	*/
	double		**pc)	/* given corners of the subdomain */
{
	COMPONENT	comp;
	CROSS		*cross;
	CURVE		**cc, *c[4];
	CURVE		**curves1, **curves2;
	FT_POINT		*p;
	INTERFACE	*sav_intfc;
	NODE		*n, **nn, *bn[4];
	int		i;

	sav_intfc = current_interface();
	set_current_interface(intfc);
	comp = (intfc->modified) ? long_component(pc[0],intfc) :
			component(pc[0],intfc);

redo_curve_list:
	for (cc = intfc->curves; cc && *cc; cc++)
	{
	    if (is_bdry(*cc))
	    {
	    	(void) delete_curve(*cc);
	    	goto redo_curve_list;
	    }
	}
	for (nn = intfc->nodes; nn && *nn; nn++)
	{
	    if (is_bdry(*nn))
	    {
	    	int num_in, num_out;
	    	if (num_curves_at_node(*nn,&num_in,&num_out) == 0)
	    	    (void) delete_node(*nn);
	    	else
	    	    set_not_bdry(*nn);
	    }
	}
	bn[0] = make_node(Point(pc[0]));
	bn[1] = make_node(Point(pc[2]));
	bn[2] = make_node(Point(pc[3]));
	bn[3] = make_node(Point(pc[1]));
	for (i = 0; i < 4; i++)
	{
	    c[i] = make_curve(NO_COMP,NO_COMP,bn[i],bn[(i+1)%4]);
	    set_is_bdry(c[0]);
	}

	if (intersections(intfc,&cross,YES) == FUNCTION_FAILED)
	{
	    screen("ERROR in insert_cuts_and_bdry2d(), "
	           "intersections() failed\n");
	    clean_up(ERROR);
	}

	if (cross == NULL)
	{
	    for (i = 0; i < 4; i++)
	    {
	    	positive_component(c[i]) = comp;
	    	negative_component(c[i]) = exterior_component(intfc);
	    }
	    return;
	}
	for (; cross != NULL; cross = cross->next)
	{
	    p = cross->p;
	    if (insert_point_in_bond(p,cross->b1,cross->c1)!=FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in insert_cuts_and_bdry2d(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	    rcl_after_insert_point(cross,p,cross->b1);
	    curves1 = split_curve(p,cross->b1,cross->c1,
	    		          positive_component(cross->c1),
	    		          negative_component(cross->c1),
	    		          positive_component(cross->c1),
	    		          negative_component(cross->c1));
	    rcl_after_split(cross,p,cross->b1,cross->c1,curves1);
	    if (insert_point_in_bond(p,cross->b2,cross->c2)!=FUNCTION_SUCCEEDED)
	    {
	        screen("ERROR in insert_cuts_and_bdry2d(), "
		       "insert_point_in_bond() failed\n");
	        clean_up(ERROR);
	    }
	    rcl_after_insert_point(cross,p,cross->b2);
	    curves2 = split_curve(p,cross->b2,cross->c2,
	    		          positive_component(cross->c2),
	    		          negative_component(cross->c2),
	    		          positive_component(cross->c2),
	    		          negative_component(cross->c2));
	    rcl_after_split(cross,p,cross->b2,cross->c2,curves1);
	    n = curves2[0]->end;
	    change_node_of_curve(curves2[0],
				NEGATIVE_ORIENTATION,curves1[0]->end);
	    change_node_of_curve(curves2[1],
				POSITIVE_ORIENTATION,curves1[0]->end);
	    (void) delete_node(n);
	}

	set_current_interface(sav_intfc);
	return;
}		/*end insert_cuts_and_bdry2d*/
#endif /* defined(TWOD) */
