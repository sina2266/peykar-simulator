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
*				fstate2d.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains routines related to states on the front:
*
*		states_at_distance_along_curve()
*		set_states_by_interpolation()
*/


#if defined(TWOD)

#include <front/fdecs.h>

struct	_DUMMY_ARRAY {
	union {
		HYPER_SURF_ELEMENT		**hse;
		HYPER_SURF			**hs;
		FT_POINT				**p;
		Locstate			*st;
		double				*t;
		byte				*bytes;
	} da;
	byte	*store;
	byte	*ststore;
	size_t	size;
	int	len, rad;
};
typedef struct  _DUMMY_ARRAY DUMMY_ARRAY;


	/* LOCAL Function Declarations */
LOCAL	DUMMY_ARRAY	*alloc_dummy_array(DUMMY_ARRAY*,int,size_t);
LOCAL	DUMMY_ARRAY	*alloc_dummy_point_array(DUMMY_ARRAY*,int,INTERFACE*);
LOCAL	DUMMY_ARRAY	*alloc_dummy_state_array(DUMMY_ARRAY*,int,size_t);
LOCAL	void	no_continuation_along_curve(int,ORIENTATION,int,
					    Locstate*,Locstate*,
					    CURVE*,HYPER_SURF**,
					    HYPER_SURF_ELEMENT**,double*,
					    FT_POINT**,Front*);

/*
*		states_at_distance_along_curve():
*
*	Finds the left and right states at a distance ds from
*	the point p along the curve c.  p is assumed to be either
*	the start or end point of bond b.  (If b == NULL, p is
*	assumed to be the node at the opposite end of c, as defined by
*	the orientation.)
*/

#define at_beginning(p,b,orient)					\
	(  (orient == POSITIVE_ORIENTATION && p == b->start)		\
 	|| (orient == NEGATIVE_ORIENTATION && p == b->end))

LOCAL	DUMMY_ARRAY	*alloc_dummy_array(
	DUMMY_ARRAY		*dpa,
	int			rad,
	size_t			size_data)
{
	int	len = 2*rad + 1;

	if (dpa == NULL)
	{
	    scalar(&dpa,sizeof(DUMMY_ARRAY));
	}

	if (len > dpa->len)
	{
	    dpa->len = len;
	    dpa->rad = rad;
	    if (dpa->store != NULL)
	    	free(dpa->store);
	    scalar(&dpa->store,len*size_data);
	    dpa->da.bytes = dpa->store + rad*size_data;
	}
	return dpa;
}		/*end alloc_dummy_array*/

LOCAL	DUMMY_ARRAY	*alloc_dummy_state_array(
	DUMMY_ARRAY		*dpa,
	int			rad,
	size_t			sizest)
{
	Locstate	*st;
	int	i, len = 2*rad + 1;
	int	olen = (dpa != NULL) ? dpa->len : 0;

	dpa = alloc_dummy_array(dpa,rad,sizeof(Locstate));
	st = dpa->da.st;

	if (len > olen)
	{
	    if (dpa->ststore != NULL)
	    	free(dpa->ststore);
	    scalar(&dpa->ststore,len*sizest);
	}
	for (i = 0; i < len; i++)
	    st[i-rad] = dpa->ststore + i*sizest;
	return dpa;
}		/*end alloc_dummy_state_array*/

LOCAL	DUMMY_ARRAY	*alloc_dummy_point_array(
	DUMMY_ARRAY		*dpa,
	int			rad,
	INTERFACE		*intfc)
{
	FT_POINT	**old_pts = (dpa != NULL) ? dpa->da.p : NULL;
	int	i, len = 2*rad + 1;
	int	olen = (dpa != NULL) ? dpa->len : 0;

	dpa = alloc_dummy_array(dpa,rad,sizeof(FT_POINT*));

	for (i = 0; i < olen; i++)
	    dpa->da.p[i] = old_pts[i];
	for (; i < len; i++)
	    dpa->da.p[i] = Static_point(intfc);
	return dpa;
}		/*end alloc_dummy_point_array*/


EXPORT void states_at_distance_along_curve(
	FT_POINT		*p,
	BOND		*b,
	CURVE		*c,
	ORIENTATION	orient, /* direction along curve for state evaluation */
	double		ds,	/* distance along curve for state evaluation  */
	int		npts,	/* number of points on curve to load          */
	Locstate	*left,
	Locstate	*right,
	HYPER_SURF			**curr_hs,
	HYPER_SURF_ELEMENT		**curr_hse,
	double		*t,
	FT_POINT		**posn,
	Front		*fr)
{
	CURVE		*cc;
	BOND		*cb, *fb;
	int		isgn, indx, i, j, dim = fr->rect_grid->dim;
	ORIENTATION	c_or;
	double		lds;
	static	DUMMY_ARRAY	*lsd = NULL, *rsd = NULL, *cd = NULL,
	                        *bd = NULL, *td = NULL, *pd = NULL;

	if (left == NULL)
	    left = (lsd = alloc_dummy_state_array(lsd,npts,fr->sizest))->da.st;
	if (right == NULL)
	    right = (rsd = alloc_dummy_state_array(rsd,npts,fr->sizest))->da.st;
	if (curr_hs == NULL)
	    curr_hs = (cd = alloc_dummy_array(cd,npts,
					      sizeof(HYPER_SURF*)))->da.hs;
	if (curr_hse == NULL)
	    curr_hse = (bd = alloc_dummy_array(bd,npts,
					  sizeof(HYPER_SURF_ELEMENT*)))->da.hse;
	if (t == NULL)
	    t = (td = alloc_dummy_array(td,npts,sizeof(double)))->da.t;
	if (posn == NULL)
	    posn = (pd = alloc_dummy_point_array(pd,npts,fr->interf))->da.p;

			/* initialize loop */
		/* p should be at "beginning" of cb */

	if (ds < 0.0) 
	{
	    ds = -ds;
	    orient = Opposite_orient(orient);
	}
	isgn = (orient == NEGATIVE_ORIENTATION) ? -1 : 1;

	cb = (b==NULL||at_beginning(p,b,orient)) ? b : Following_bond(b,orient);
	cc = c;
	c_or = orient;

	if (cb == NULL) 	/* at termination of curve */
	{
	    if (!is_closed_node(Node_of(cc,c_or)))
	    {
	    	no_continuation_along_curve(0,c_or,npts,left,right,cc,curr_hs,
					    curr_hse,t,posn,fr);
		return;
	    }
	    cb = Bond_at_node(cc,c_or);
	}

	for (i = 0, lds = ds; i < npts; i++, lds += ds)
	{
	    if (cb == NULL)
		continue;
	    indx = i*isgn;

	    /* loop to find cb, in the middle of which is the displaced point */

	    while (bond_length(cb) < lds) 
	    {
	    	lds -= bond_length(cb);
	    	fb = Following_bond(cb,c_or);
	    	if ((fb == NULL) && (!is_closed_node(Node_of(cc,c_or))))
		{
		    cb = NULL;
		    break;
		}
		cb = (fb == NULL) ? Bond_at_node(cc,c_or) : fb;
	    }
	    if (cb != NULL)
	    {

	    	/* interpolate */

	    	t[indx] = lds / bond_length(cb);
	    	if (c_or == NEGATIVE_ORIENTATION)
	    	    t[indx] = 1.0 - t[indx];
		for (j = 0; j < dim; j++)
		{
		    Coords(posn[indx])[j] = Coords(cb->start)[j] + 
					    t[indx]*(Coords(cb->end)[j] -
					    Coords(cb->start)[j]);
		}
		left_state_along_bond(t[indx],cb,cc,left[indx]);
		right_state_along_bond(t[indx],cb,cc,right[indx]);
		t[indx] *= bond_length(cb);
		curr_hs[indx] = Hyper_surf(cc);
		curr_hse[indx] = Hyper_surf_element(cb);
	    }
	    else
	    {
	    	/* at termination of curve */
	    	no_continuation_along_curve(indx,c_or,npts,left,right,cc,
					    curr_hs,curr_hse,t,posn,fr);
	    }
	}
}		/*end states_at_distance_along_curve*/



LOCAL	void	no_continuation_along_curve(
	int		indx,
	ORIENTATION	orient,
	int		npts,
	Locstate	*left,
	Locstate	*right,
	CURVE		*cc,
	HYPER_SURF			**curr_hs,
	HYPER_SURF_ELEMENT		**curr_hse,
	double		*t,
	FT_POINT		**posn,
	Front		*fr)
{
	FT_POINT		*curr_posn;
	int		i, j, dim = fr->rect_grid->dim;
	int		isgn = (orient == NEGATIVE_ORIENTATION) ? -1 : 1;
	size_t		sizest = fr->sizest;

			/* use states at node */

	curr_hs[indx] = Hyper_surf(cc);
	if (orient == POSITIVE_ORIENTATION) 
	{
		curr_hse[indx] = Hyper_surf_element(cc->last);
		t[indx] = bond_length(cc->last);
		curr_posn = cc->last->end;
	}
	else 
	{
		curr_hse[indx] = Hyper_surf_element(cc->first);
		t[indx] = 0.0;
		curr_posn = cc->first->start;
	}
	for (i = 0; i < dim; i++)
		Coords(posn[indx])[i] = Coords(curr_posn)[i];
	ft_assign(left[indx],Left_state_at_node(cc,
		Opposite_orient(orient)),sizest);
	ft_assign(right[indx],Right_state_at_node(cc,
		Opposite_orient(orient)),sizest);
	for (i = indx+isgn; i*isgn < npts; i += isgn)
	{
		curr_hse[i] = curr_hse[indx];
		curr_hs[i] = curr_hs[indx];
		t[i] = t[indx];
		for (j = 0; j < dim; j++)
			Coords(posn[i])[j] = Coords(posn[indx])[j];
		ft_assign(left[i],left[indx],sizest);
		ft_assign(right[i],right[indx],sizest);
	}
}		/*end no_continuation_along_curve*/

/*
*		set_states_by_interpolation():
*
*	Computes the states on a curve from the start of bond b1
*	to the end of bond b2 by linear interpolation between
*	the states start and end.
*/

EXPORT void set_states_by_interpolation(
	CURVE		*c,
	BOND		*bs,
	BOND		*be,
	SIDE		side,
	Locstate	start,
	Locstate	end,
	size_t		sizest)
{
	BOND		*b;
	double		clen,t;
	INTERFACE	*intfc = c->interface;

	if (c == NULL)
	    return;
	if (bs == NULL)
	    bs = c->first;
	if (be == NULL)
	    be = c->last;

	clen = t = 0.0;
	for (b = bs; b; b = b->next)
	{
	    clen += bond_length(b);
	    if (b == be)
		break;
	}
	if (side == NEGATIVE_SIDE)
	{
	    ft_assign(left_state_at_point_on_curve(bs->start,bs,c),start,sizest);
	    ft_assign(left_state_at_point_on_curve(be->end,be,c),end,sizest);
	    for (b = bs; b != NULL && b != be; b = b->next)
	    {
	    	t += bond_length(b) / clen;
	    	bi_interpolate_intfc_states(intfc,1.0-t,t,
	    		                    Coords(bs->start),start,
	    		                    Coords(bs->end),end,
					    left_state(b->end));
		}	
	}
	if (side == POSITIVE_SIDE)
	{
	    ft_assign(right_state_at_point_on_curve(bs->start,bs,c),start,sizest);
	    ft_assign(right_state_at_point_on_curve(be->end,be,c),end,sizest);
	    for (b = bs; b != NULL && b != be; b = b->next)
	    {
	    	t += bond_length(b) / clen;
	    	bi_interpolate_intfc_states(intfc,1.0-t,t,Coords(bs->start),
					    start,Coords(bs->end),end,
					    right_state(b->end));
	    }	
	}
}		/*end set_states_by_interpolation*/

#endif /* defined(TWOD) */
