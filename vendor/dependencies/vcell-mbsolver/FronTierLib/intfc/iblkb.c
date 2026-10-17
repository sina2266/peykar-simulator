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
*				iblkb.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Containing function of rebuilding interface within a mesh block.
*
*/


#include <intfc/int.h>

#define DEBUG_STRING "blk_intfc"

LOCAL	boolean is_case_b1(BLK_CRX*);
LOCAL	boolean is_case_b2(BLK_CRX*);
LOCAL	boolean is_case_b3(BLK_CRX*);
LOCAL	boolean is_case_b4(BLK_CRX*);
LOCAL	void x_rot(BLK_CRX*);
LOCAL	void y_rot(BLK_CRX*);
LOCAL	void z_rot(BLK_CRX*);
LOCAL	void blk_case_b1(BLK_CRX*,BLK_TRI*);
LOCAL	void blk_case_b2(BLK_CRX*,BLK_TRI*);
LOCAL	void blk_case_b3(BLK_CRX*,BLK_TRI*);
LOCAL	void blk_case_b4(BLK_CRX*,BLK_TRI*);
LOCAL	void rot_24(BLK_CRX*,int);
LOCAL   int is_curve(BLK_CRX*,CURVE*);
LOCAL   void stitch_inside_bdry_blk(BLK_TRI *);

EXPORT	int construct_bdry_blk(
	BLK_CRX *blk_crx,
	BLK_TRI *blk_mem)
{
	int i;

	if (blk_crx->nv[0] > 4)
	{
	    screen("ERROR: in construct_bdry_blk(), no such case!\n");
	    clean_up(ERROR);
	}

	for (i = 0; i < 24; i++)
	{
	    if (blk_crx->nv[0] == 1)
	    {
		if (is_case_b1(blk_crx))
		{
		    blk_case_b1(blk_crx,blk_mem);
	    	    break;
		}
	    }
	    else if (blk_crx->nv[0] == 2)
	    {
		if (is_case_b2(blk_crx))
		{
		    blk_case_b2(blk_crx,blk_mem);
	    	    break;
		}
	    }
	    else if (blk_crx->nv[0] == 4)
	    {
		if (is_case_b3(blk_crx))
		{
		    blk_case_b3(blk_crx,blk_mem);
	    	    break;
		}
		if (is_case_b4(blk_crx))
		{
		    blk_case_b4(blk_crx,blk_mem);
	    	    break;
		}
	    }
	    rot_24(blk_crx,i);
	}
	stitch_inside_bdry_blk(blk_mem);
	
	return FUNCTION_SUCCEEDED;
}	/* end reconstruct_blk_intfc */

/*previous stitch_insde_bdry_blk, used for bdry case. */
LOCAL	void stitch_inside_bdry_blk(
	BLK_TRI *bm)
{
	const int *num_tris = bm->num_tris;
	int       is, ic, i, j, side1, side2;
	TRI       *tri1, *tri2;
	FT_POINT     **p1, **p2;
	BOND      *b;
	SURFACE   *s;
	CURVE     *c;
	FT_POINT 	  *ptmp;
	ORIENTATION orient;
	BOND_TRI  *btri;

	if (bm->num_curves != 0)
	{
	    for (ic = 0; ic < bm->num_curves; ++ic)
	    {
		c = bm->curves[bm->ic[ic]];
		b = bm->bonds[bm->ic[ic]];
		for (is = 0; is < bm->num_surfaces; ++is)
		{
		    s = bm->surfs[bm->is[is]];
		    orient = curve_surface_orientation(s,c);
	    	    for (i = 0, tri1 = bm->first[bm->is[is]];
				    i < num_tris[bm->is[is]];
	    			++i, tri1 = tri1->next)
	    	    {
	        	p1 = Point_of_tri(tri1);
	            	for (side1 = 0; side1 < 3; ++side1)
	            	{
			    if (Neighbor_on_side(tri1,side1) != NULL)
			    	continue;
			    if (b->start == p1[side1] &&
			    	b->end == p1[Next_m3(side1)])
			    {
				if (orient == NEGATIVE_ORIENTATION)
				{
				    ptmp = b->start;
		    		    b->start = b->end;
		    		    b->end = ptmp;
				}
			    	btri = link_tri_to_bond(NULL,tri1,s,b,c);
				bm->num_null_sides[bm->is[is]] -= 1;
			    }
			    else if (b->start == p1[Next_m3(side1)] &&
                                b->end == p1[side1])
			    {
				if (orient == POSITIVE_ORIENTATION)
				{
		    		    ptmp = b->start;
		    		    b->start = b->end;
		    		    b->end = ptmp;
				}
			    	btri = link_tri_to_bond(NULL,tri1,s,b,c);
				bm->num_null_sides[bm->is[is]] -= 1;
			    }
			}
		    }
		}
	    }
	}
	for (is = 0; is < bm->num_surfaces; ++is)
	{
	    for (i = 0, tri1 = bm->first[bm->is[is]]; i < num_tris[bm->is[is]]; 
	    		++i, tri1 = tri1->next)
	    {
	        p1 = Point_of_tri(tri1);
	        for (j = i+1, tri2 = tri1->next; j < num_tris[bm->is[is]]; 
				++j, tri2 = tri2->next)
	        {
	            p2 = Point_of_tri(tri2);
	            for (side1 = 0; side1 < 3; ++side1)
	            {
		        if (Neighbor_on_side(tri1,side1) != NULL)
		    	    continue;
	                for (side2 = 0; side2 < 3; ++side2)
	                {
		    	    if (Neighbor_on_side(tri2,side2) != NULL)
		    	        continue;
	                    if (p1[side1] == p2[Next_m3(side2)] &&
	                        p1[(side1+1)%3] == p2[side2])
	                    {
	                        Tri_on_side(tri1,side1) = tri2;
	                        Tri_on_side(tri2,side2) = tri1;
	                        bm->num_null_sides[bm->is[is]] -= 2;
	                    }
	                }
	            }
	        }
	    }
	}
}	/*end stitch_tris_inside_block*/


LOCAL	boolean is_case_b1(BLK_CRX *blk_crx)
{
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT *comps = blk_crx->comps;

	if (comp[0][0][0] != comps[1]) return NO;
	if (comp[0][0][1] != comps[1]) return NO;
	if (comp[0][1][0] != comps[1]) return NO;
	if (comp[0][1][1] != comps[1]) return NO;
	if (comp[1][0][0] != comps[1]) return NO;
	if (comp[1][0][1] != comps[0]) return NO;
	if (comp[1][1][0] != comps[1]) return NO;
	if (comp[1][1][1] != comps[1]) return NO;
	return YES;
}	/* end is_case_b1 */

LOCAL	boolean is_case_b2(BLK_CRX *blk_crx)
{
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT *comps = blk_crx->comps;

	if (comp[0][0][0] != comps[1]) return NO;
	if (comp[0][0][1] != comps[1]) return NO;
	if (comp[0][1][0] != comps[1]) return NO;
	if (comp[0][1][1] != comps[1]) return NO;
	if (comp[1][0][0] != comps[0]) return NO;
	if (comp[1][0][1] != comps[0]) return NO;
	if (comp[1][1][0] != comps[1]) return NO;
	if (comp[1][1][1] != comps[1]) return NO;
	return YES;
}	/* end is_case_b2 */

LOCAL	boolean is_case_b3(BLK_CRX *blk_crx)
{
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT *comps = blk_crx->comps;

	if (comp[0][0][0] != comps[1]) return NO;
	if (comp[0][1][0] != comps[1]) return NO;
	if (comp[1][0][0] != comps[1]) return NO;
	if (comp[1][1][0] != comps[1]) return NO;
	if (comp[0][0][1] != comps[0]) return NO;
	if (comp[0][1][1] != comps[0]) return NO;
	if (comp[1][0][1] != comps[0]) return NO;
	if (comp[1][1][1] != comps[0]) return NO;
	if (blk_crx->curve_crx[0][0] != NULL &&
	    blk_crx->curve_crx[0][1] != NULL &&
	    blk_crx->curve_crx[1][0] != NULL &&
	    blk_crx->curve_crx[1][1] != NULL)
	    return YES;
	else
	    return NO;
}	/* end is_case_b3 */

LOCAL	boolean is_case_b4(BLK_CRX *blk_crx)
{
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT *comps = blk_crx->comps;

	if (comp[0][0][0] != comps[1]) return NO;
	if (comp[0][1][0] != comps[1]) return NO;
	if (comp[1][0][0] != comps[1]) return NO;
	if (comp[1][1][0] != comps[1]) return NO;
	if (comp[0][0][1] != comps[0]) return NO;
	if (comp[0][1][1] != comps[0]) return NO;
	if (comp[1][0][1] != comps[0]) return NO;
	if (comp[1][1][1] != comps[0]) return NO;
	if (blk_crx->curve_crx[0][0] != NULL &&
	    blk_crx->curve_crx[0][1] != NULL &&
	    blk_crx->curve_crx[1][0] == NULL &&
	    blk_crx->curve_crx[1][1] == NULL)
	    return YES;
	else
	    return NO;
}	/* end is_case_b4 */

LOCAL 	void blk_case_b1(
        BLK_CRX *bc,
        BLK_TRI *bm)
{
        FT_POINT   *p1, *p2, *p3;
	SURFACE *s;
	CURVE   *c;
	int     ic, is;
 
	bm->num_curves = 3;
	bm->num_surfaces = 3;

	/* the first curve */
	c = bc->curve_crx[0][1]->c;
	ic = bm->ic[0] = is_curve(bc,c);
	bm->curves[ic] = c;
	p1 = bc->curve_crx[0][1]->p;
        p2 = bc->node_crx->p;
	if (p1 == c->start->posn)
	    bm->bonds[ic] = Bond(p1,p2);
	else
	    bm->bonds[ic] = Bond(p2,p1);

	/* the second curve */
	c = bc->curve_crx[1][0]->c;
	ic = bm->ic[1] = is_curve(bc,c);
	bm->curves[ic] = c;
	p1 = bc->curve_crx[1][0]->p;
        p2 = bc->node_crx->p;
	if (p1 == c->start->posn)
	    bm->bonds[ic] = Bond(p1,p2);
	else
	    bm->bonds[ic] = Bond(p2,p1);

	/* the third curve */
	c = bc->curve_crx[2][1]->c;
	ic = bm->ic[2] = is_curve(bc,c);
	bm->curves[ic] = c;
	p1 = bc->curve_crx[2][1]->p;
        p2 = bc->node_crx->p;
	if (p1 == c->start->posn)
	    bm->bonds[ic] = Bond(p1,p2);
	else
	    bm->bonds[ic] = Bond(p2,p1);

	/* the first surface */

	s = bc->crx[0][0][1]->s;
	is = bm->is[0]= is_surface(bc,s);
	bm->surfs[is] = s;
	bm->num_null_sides[is] = 6;
        p1 = bc->crx[0][0][1]->p;
        p2 = bc->curve_crx[2][1]->p;
        p3 = bc->curve_crx[1][0]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);

        p1 = bc->node_crx->p;
        p2 = bc->curve_crx[1][0]->p;
        p3 = bc->curve_crx[2][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);

	/* the second surface */
	s = bc->crx[1][1][1]->s;
	is = bm->is[1]= is_surface(bc,s);
	bm->surfs[is] = s;
	bm->num_null_sides[is] = 6;
        p1 = bc->crx[1][1][1]->p;
        p2 = bc->curve_crx[0][1]->p;
        p3 = bc->curve_crx[2][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);
 
        p1 = bc->node_crx->p;
        p2 = bc->curve_crx[2][1]->p;
        p3 = bc->curve_crx[0][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);

	/* the third surface */

	s = bc->crx[2][1][0]->s;
	is = bm->is[2]= is_surface(bc,s);
	bm->surfs[is] = s;
	bm->num_null_sides[is] = 6;
	p1 = bc->crx[2][1][0]->p;
        p2 = bc->curve_crx[1][0]->p;
        p3 = bc->curve_crx[0][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);

        p1 = bc->node_crx->p;
        p2 = bc->curve_crx[0][1]->p;
        p3 = bc->curve_crx[1][0]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);
}       /* end blk_case_b1 */


LOCAL	void blk_case_b2(
        BLK_CRX *bc,
        BLK_TRI *bm)
{
        FT_POINT   *p1, *p2, *p3;
	SURFACE *s;
	CURVE   *c;
	int     ic, is;

	bm->num_curves = 1;
	bm->num_surfaces = 2;

	/* curve */
	c = bc->curve_crx[2][1]->c;
	ic = bm->ic[0] = is_curve(bc,c);
	bm->curves[ic] = c;
	p1 = bc->curve_crx[2][0]->p;
	p2 = bc->curve_crx[2][1]->p;
	bm->bonds[ic] = Bond(p2,p1);

	/* possible more curves */
	if (bc->curve_crx[0][0] != NULL ||
	    bc->curve_crx[0][1] != NULL)
	{
	    c = (bc->curve_crx[0][0]) ?
	    	bc->curve_crx[0][0]->c : bc->curve_crx[0][1]->c;
	    ic = bm->ic[1] = is_curve(bc,c);
	    bm->curves[ic] = c;
	    p1 = (bc->curve_crx[0][0]) ? bc->curve_crx[0][0]->p :
	    	bc->curve_crx[0][1]->p;
	    if (c->start->posn == bc->curve_crx[2][0]->p ||
	    	c->end->posn == bc->curve_crx[2][0]->p)
	    {
	    	p2 = bc->curve_crx[2][0]->p;
	    }
	    else if (c->start->posn == bc->curve_crx[2][1]->p ||
	        c->end->posn == bc->curve_crx[2][1]->p)
	    {
	    	p2 = bc->curve_crx[2][1]->p;
	    }
	    bm->num_curves++;
	    bm->bonds[ic] = Bond(p2,p1);
	}
	if (bc->curve_crx[1][0] != NULL ||
	    bc->curve_crx[1][1] != NULL)
	{
	    c = (bc->curve_crx[1][0]) ?
	    	bc->curve_crx[1][0]->c : bc->curve_crx[1][1]->c;
	    ic = bm->ic[1] = is_curve(bc,c);
	    bm->curves[ic] = c;
	    p1 = (bc->curve_crx[1][0]) ? bc->curve_crx[1][0]->p :
	    	bc->curve_crx[1][1]->p;
	    if (c->start->posn == bc->curve_crx[2][0]->p ||
	    	c->end->posn == bc->curve_crx[2][0]->p)
	    {
	    	p2 = bc->curve_crx[2][0]->p;
	    }
	    else if (c->start->posn == bc->curve_crx[2][1]->p ||
	        c->end->posn == bc->curve_crx[2][1]->p)
	    {
	    	p2 = bc->curve_crx[2][1]->p;
	    }
	    bm->num_curves++;
	    bm->bonds[ic] = Bond(p2,p1);
	}

	/* the first surface */

	s = bc->crx[0][0][1]->s;
	is = bm->is[0]= is_surface(bc,s);
	bm->surfs[is] = s;
	bm->num_null_sides[is] = 6;
        p1 = bc->crx[0][0][1]->p;
        p2 = bc->crx[0][0][0]->p;
        p3 = bc->curve_crx[2][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p3,p2,s);
	else
	    create_triangle(bm,p1,p2,p3,s);

        p1 = bc->curve_crx[2][0]->p;
        p2 = bc->curve_crx[2][1]->p;
        p3 = bc->crx[0][0][0]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p3,p2,s);
	else
	    create_triangle(bm,p1,p2,p3,s);

	/* the second surface */

	s = bc->crx[1][0][1]->s;
	is= bm->is[1]= is_surface(bc,s);
	bm->surfs[is] = s;
	bm->num_null_sides[is] = 6;
	p1 = bc->curve_crx[2][1]->p;
	p2 = bc->crx[1][0][1]->p;
	p3 = bc->crx[1][1][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p3,p2,s);
	else
	    create_triangle(bm,p1,p2,p3,s);

	p1 = bc->crx[1][0][1]->p;
	p2 = bc->curve_crx[2][1]->p;
	p3 = bc->curve_crx[2][0]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p3,p2,s);
	else
	    create_triangle(bm,p1,p2,p3,s);
}       /* end blk_case_b2 */

LOCAL 	void blk_case_b3(
        BLK_CRX *bc,
        BLK_TRI *bm)
{
	FT_POINT   *p1, *p2, *p3;
	SURFACE *s;
	CURVE   *c1, *c2;
	int     ic, is;

	bm->num_curves = 2;
	bm->num_surfaces = 1;

	/* the first curve */
	c1 = (bc->curve_crx[0][0]->c != NULL) ?
		bc->curve_crx[0][0]->c : bc->curve_crx[0][1]->c;
	ic = bm->ic[0] = is_curve(bc,c1);
	bm->curves[ic] = c1; 
	p1 = bc->curve_crx[0][0]->p;
	p2 = bc->curve_crx[0][1]->p;
	bm->bonds[ic] = Bond(p1,p2);

	/* the second curve */
	c2 = (bc->curve_crx[1][0]->c != NULL) ?
		bc->curve_crx[1][0]->c : bc->curve_crx[1][1]->c;
	ic = bm->ic[1] = is_curve(bc,c2);
	bm->curves[ic] = c2;
	p1 = bc->curve_crx[1][0]->p;
	p2 = bc->curve_crx[1][1]->p;
	bm->bonds[ic] = Bond(p1,p2);

	/* the surface */
	s = bc->crx[2][0][0]->s;
	is = bm->is[0]= is_surface(bc,s);
	bm->surfs[is] = s;
	p1 = bc->crx[2][0][0]->p;
	p2 = bc->crx[2][0][1]->p;
	p3 = bc->crx[2][1][0]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);

	p1 = bc->crx[2][1][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p3,p2,s);
	else
	    create_triangle(bm,p1,p2,p3,s);
}       /* end blk_case_b3 */

LOCAL 	void blk_case_b4(
        BLK_CRX *bc,
        BLK_TRI *bm)
{
	FT_POINT   *p1, *p2, *p3;
	SURFACE *s;
	CURVE   *c;
	int     ic, is;

	bm->num_curves = 1;
	bm->num_surfaces = 1;

	/* curve */
	c = bc->curve_crx[0][0]->c;
	ic = bm->ic[0] = is_curve(bc,c);
	bm->curves[ic] = c;
	p1 = bc->curve_crx[0][0]->p;
	p2 = bc->curve_crx[0][1]->p;
	bm->bonds[ic] = Bond(p1,p2);

	/* surface */
	s = bc->crx[2][0][0]->s;
	is = bm->is[0]= is_surface(bc,s);
	bm->surfs[is] = s;
	p1 = bc->crx[2][0][0]->p;
	p2 = bc->crx[2][0][1]->p;
	p3 = bc->crx[2][1][0]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p2,p3,s);
	else
	    create_triangle(bm,p1,p3,p2,s);

	p1 = bc->crx[2][1][1]->p;
	if (bc->comps[0] == positive_component(s))
	    create_triangle(bm,p1,p3,p2,s);
	else
	    create_triangle(bm,p1,p2,p3,s);
}       /* end blk_case_b4 */

LOCAL	void rot_24(
	BLK_CRX *blk_crx,
	int n)
{
	if (n < 16)
	{
	    x_rot(blk_crx);
	    if ((n+1)%4 == 0) 
	    	y_rot(blk_crx);
	}
	else if (n == 16)
	{
	    z_rot(blk_crx);
	    x_rot(blk_crx);
	}
	else if (n == 20)
	{
	    z_rot(blk_crx);
	    z_rot(blk_crx);
	    x_rot(blk_crx);
	}
	else if (n < 24)
	{
	    x_rot(blk_crx);
	}
}	/* end rot_24 */

LOCAL	void x_rot(BLK_CRX *blk_crx)
{
	int i;
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT comp_tmp;
	BBI_POINT ****crx = blk_crx->crx;
	BBI_POINT ***curve_crx = blk_crx->curve_crx;
	BBI_POINT *crx_tmp;

	for (i = 0; i < 2; i++)
	{
	    comp_tmp = comp[i][0][0];
	    comp[i][0][0] = comp[i][1][0];
	    comp[i][1][0] = comp[i][1][1];
	    comp[i][1][1] = comp[i][0][1];
	    comp[i][0][1] = comp_tmp;
	    crx_tmp = crx[2][i][0];
	    crx[2][i][0] = crx[1][0][i];
	    crx[1][0][i] = crx[2][i][1];
	    crx[2][i][1] = crx[1][1][i];
	    crx[1][1][i] = crx_tmp;
	}
	crx_tmp = curve_crx[1][0];
	curve_crx[1][0] = curve_crx[2][0];
	curve_crx[2][0] = curve_crx[1][1];
	curve_crx[1][1] = curve_crx[2][1];
	curve_crx[2][1] = crx_tmp;
	crx_tmp = crx[0][0][0];
	crx[0][0][0] = crx[0][1][0];
	crx[0][1][0] = crx[0][1][1];
	crx[0][1][1] = crx[0][0][1];
	crx[0][0][1] = crx_tmp;
}	/* end x_rot */

LOCAL	void y_rot(BLK_CRX *blk_crx)
{
	int j;
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT comp_tmp;
	BBI_POINT ****crx = blk_crx->crx;
	BBI_POINT ***curve_crx = blk_crx->curve_crx;
	BBI_POINT *crx_tmp;

	for (j = 0; j < 2; j++)
	{
	    comp_tmp = comp[0][j][0];
	    comp[0][j][0] = comp[0][j][1];
	    comp[0][j][1] = comp[1][j][1];
	    comp[1][j][1] = comp[1][j][0];
	    comp[1][j][0] = comp_tmp;
	    crx_tmp = crx[2][0][j];
	    crx[2][0][j] = crx[0][j][1];
	    crx[0][j][1] = crx[2][1][j];
	    crx[2][1][j] = crx[0][j][0];
	    crx[0][j][0] = crx_tmp;
	}
	crx_tmp = curve_crx[2][0];
	curve_crx[2][0] = curve_crx[0][0];
	curve_crx[0][0] = curve_crx[2][1];
	curve_crx[2][1] = curve_crx[0][1];
	curve_crx[0][1] = crx_tmp;
	crx_tmp = crx[1][0][0];
	crx[1][0][0] = crx[1][1][0];
	crx[1][1][0] = crx[1][1][1];
	crx[1][1][1] = crx[1][0][1];
	crx[1][0][1] = crx_tmp;
}	/* end y_rot */

LOCAL	void z_rot(BLK_CRX *blk_crx)
{
	int k;
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT comp_tmp;
	BBI_POINT ****crx = blk_crx->crx;
	BBI_POINT ***curve_crx = blk_crx->curve_crx;
	BBI_POINT *crx_tmp;

	for (k = 0; k < 2; k++)
	{
	    comp_tmp = comp[0][0][k];
	    comp[0][0][k] = comp[1][0][k];
	    comp[1][0][k] = comp[1][1][k];
	    comp[1][1][k] = comp[0][1][k];
	    comp[0][1][k] = comp_tmp;
	    crx_tmp = crx[0][0][k];
	    crx[0][0][k] = crx[1][k][1];
	    crx[1][k][1] = crx[0][1][k];
	    crx[0][1][k] = crx[1][k][0];
	    crx[1][k][0] = crx_tmp;
	}
	crx_tmp = curve_crx[0][0];
	curve_crx[0][0] = curve_crx[1][0];
	curve_crx[1][0] = curve_crx[0][1];
	curve_crx[0][1] = curve_crx[1][1];
	curve_crx[1][1] = crx_tmp;
	crx_tmp = crx[2][0][0];
	crx[2][0][0] = crx[2][1][0];
	crx[2][1][0] = crx[2][1][1];
	crx[2][1][1] = crx[2][0][1];
	crx[2][0][1] = crx_tmp;
}	/* end z_rot */

LOCAL   int is_curve(
        BLK_CRX *blk_crx,
        CURVE *c)
{
        int i;
        for (i = 0; i < blk_crx->blk_info->num_curves; ++i)
        {
            if (c == blk_crx->blk_info->curves[i])
            return i;
        }
        screen("ERROR in is_curve, No curve matched\n");
	printf("#c  = %p\n", c);
        clean_up(ERROR);
}	/* end is_curve */

