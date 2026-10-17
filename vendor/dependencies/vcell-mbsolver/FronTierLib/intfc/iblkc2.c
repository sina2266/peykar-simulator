/*
*				iblkc2.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Containing function of rebuilding interface within a mesh block.
*
*/


#include <intfc/int.h>

#define DEBUG_STRING "blk_intfc"

LOCAL	BBI_POINT *crx_in_idir(const BLK_CRX*,int,int);
LOCAL	BBI_POINT *crx_in_jdir(const BLK_CRX*,int,int);
LOCAL	BBI_POINT *crx_in_kdir(const BLK_CRX*,int,int);
LOCAL	BBI_POINT *crxing_in_between(const int*,const int*,const BLK_CRX*);
LOCAL   int compare_comp(COMPONENT***,COMPONENT****,int);
LOCAL	void copy_blk_crx(const BLK_CRX*,BLK_CRX*);
LOCAL	void remove_null_pair_of_surface(BLK_TRI*,BLK_TRI*,int,int,int);
LOCAL	void set_prime_components(COMPONENT****);
LOCAL   void rot24(BLK_CRX*, int);
LOCAL   void x_rotation(BLK_CRX*);
LOCAL   void y_rotation(BLK_CRX*);
LOCAL   void z_rotation(BLK_CRX*);
LOCAL   void blk_case01_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case02_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case03_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case04_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case05_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case06_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case07_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case08_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case09_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case10_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case11_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case12_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case13_comp2(BLK_CRX*, BLK_TRI*);
LOCAL   void blk_case14_comp2(BLK_CRX*, BLK_TRI*);


/* AREA Calculation functions */
LOCAL	double area_plane_case01(BLK_CRX*);
LOCAL	double area_edge_case02(BLK_CRX*);
LOCAL	double area_corner_case03(BLK_CRX*);
LOCAL	double area_glider_case04(BLK_CRX*);
LOCAL	double area_hexagon_case05(BLK_CRX*);
LOCAL	double area_corner1_case06(BLK_CRX*);
LOCAL	double area_corner2_case06(BLK_CRX*);
LOCAL	double area_twister_case07(BLK_CRX*);
LOCAL	double area_corner_case08(BLK_CRX*);
LOCAL	double area_edge_case08(BLK_CRX*); 
LOCAL	double area_corner1_case09(BLK_CRX*);
LOCAL	double area_corner2_case09(BLK_CRX*);
LOCAL   double area_twister_case10(BLK_CRX*);
LOCAL   double area_edge1_case11(BLK_CRX*);	
LOCAL   double area_edge2_case11(BLK_CRX*);
LOCAL	double area_corner1_case12(BLK_CRX*);
LOCAL	double area_corner2_case12(BLK_CRX*);
LOCAL	double area_corner3_case12(BLK_CRX*);
LOCAL	double area_corner_case13(BLK_CRX*);
LOCAL   double area_glider_case13(BLK_CRX*);
LOCAL	double area_corner1_case14(BLK_CRX*);
LOCAL	double area_corner2_case14(BLK_CRX*);
LOCAL	double area_corner3_case14(BLK_CRX*);
LOCAL	double area_corner4_case14(BLK_CRX*);

/* Volume Calculation functions */
LOCAL	double volume_plane_case01(BLK_CRX*,double*,double*,double*,double*);
LOCAL	double volume_edge_case02(BLK_CRX*,double*,double*);
LOCAL	double volume_corner_case03(BLK_CRX*,double*);
LOCAL	double volume_glider_case04(BLK_CRX*,double*,double*,double*);
LOCAL	double volume_hexagon_case05(BLK_CRX*,double*,double*,double*,double*,
					double*); 
LOCAL	double volume_corner1_case06(BLK_CRX*,double*);
LOCAL	double volume_corner2_case06(BLK_CRX*,double*);
LOCAL	double volume_twister_case07(BLK_CRX*,double*,double*,double*,double*);
LOCAL	double volume_corner_case08(BLK_CRX*,double*);
LOCAL	double volume_edge_case08(BLK_CRX*,double*,double*);
/*	      volume_corner1_case09() = volume_corner_case08() */
LOCAL	double volume_corner2_case09(BLK_CRX*,double*);
LOCAL   double volume_twister_case10(BLK_CRX*,double*,double*,double*,double*);
LOCAL	double volume_edge1_case11(BLK_CRX*,double*,double*);
LOCAL	double volume_edge2_case11(BLK_CRX*,double*,double*);
/*	      volume_corner1_case12() = volume_corner1_case06() */
/*	      volume_corner2_case12() = volume_corner2_case09() */
/*	      volume_corner3_case12() = volume_corner2_case06() */
/*	      volume_corner_case13() = volume_corner2_case09() */
LOCAL   double volume_glider_case13(BLK_CRX*,double*,double*,double*);
LOCAL	double volume_corner1_case14(BLK_CRX*,double*);
/*            volume_corner2_case14() = volume_corner_case03() */
/*            volume_corner3_case14() = volume_corner_case08() */
LOCAL	double volume_corner4_case14(BLK_CRX*,double*);

/* Volume calculation functions for other interface perimeters p_i  */
LOCAL   double volume_edge1_p2_case11(BLK_CRX*,double*,double*);
/*	volume_edge2_p2_case11() = volume_edge_case02() */
/*	volume_corner_p2_case12() = volume_corner_case03() */
/*	volume_hexagon_p2_case12() = volume_hexagon_case05() */
/*	volume_corner_p2_case13() = volume_corner_case03() */
LOCAL   double volume_glider_p2_case13(BLK_CRX*,double*,double*,double*);
LOCAL	double volume_corner1_p2_case14(BLK_CRX*,double*);
/*	volume_corner2_p2_case14() = volume_corner1_case06() */
/*	volume_corner3_p2_case14() = volume_corner2_case06()  */
/*	volume_corner4_p2_case14() = volume_corner2_case09() */
/*	volume_corner1_p3_case14() = volume_corner1_p2_case14() */
/*	volume_corner2_p3_case14() = volume_corner_case03()  */
/*	volume_hexagon_p3_case14() = volume_hexagon_case05() */
/*	volume_corner1_p4_case14() = volume_corner1_case14() */
/*	volume_corner2_p4_case14() = volume_corner2_case06()  */
LOCAL	double volume_hexagon_p4_case14(BLK_CRX*,double*,double*,double*,double*,
					double*); 
/*	volume_corner1_p5_case14() = volume_corner4_case14() */
/*	volume_corner2_p5_case14() = volume_corner1_case06()  */
LOCAL	double volume_hexagon_p5_case14(BLK_CRX*,double*,double*,double*,double*,
					double*); 
/*	volume_corner1_p6_case14() = volume_corner2_case09() */
/*	volume_corner2_p6_case14() = volume_corner_case08()  */
LOCAL	double volume_hexagon_p6_case14(BLK_CRX*,double*,double*,double*,double*,
					double*); 


EXPORT	int construct_comp2_blk(
	BLK_CRX *blk_crx,
	BLK_TRI *blk_mem)
{
	int       i,j,k,is;
	COMPONENT ***comp = blk_crx->comp;
	int       num_crx, case_found;
	BBI_POINT *crxs[12];
	static COMPONENT ****prime_comp;
	static BLK_CRX *bc_rot;
	void (*blk_intfc_comp2[14])(BLK_CRX*, BLK_TRI*) =
	{
	    blk_case01_comp2,
	    blk_case02_comp2,
	    blk_case03_comp2,
	    blk_case04_comp2,
	    blk_case05_comp2,
	    blk_case06_comp2,
	    blk_case07_comp2,
	    blk_case08_comp2,
	    blk_case09_comp2,
	    blk_case10_comp2,
	    blk_case11_comp2,
	    blk_case12_comp2,
	    blk_case13_comp2,
	    blk_case14_comp2,
	};

	if (prime_comp == NULL)
	{
	    quad_array(&prime_comp,14,2,2,2,sizeof(COMPONENT));
	    set_prime_components(prime_comp);
	}

	blk_mem->num_surfaces = 1;
	blk_mem->num_curves = 0;
	num_crx = 0;
	for (j = 0; j < 2; ++j)
	{
	    for (k = 0; k < 2; ++k)
	    {
	        if (comp[0][j][k] != comp[1][j][k])
	        {
	            crxs[num_crx] = crx_in_idir(blk_crx,j,k);
	            if (crxs[num_crx] == NULL)
		    {
	                return FUNCTION_FAILED;
		    }
	            ++num_crx;
	        }
	    }
	}

	for (k = 0; k < 2; ++k)
	{
	    for (i = 0; i < 2; ++i)
	    {
	        if (comp[i][0][k] != comp[i][1][k])
	        {
	            crxs[num_crx] = crx_in_jdir(blk_crx,k,i);
	            if (crxs[num_crx] == NULL)
		    {
	                return FUNCTION_FAILED;
		    }
	            ++num_crx;
	        }
	    }
	}

	for (i = 0; i < 2; ++i)
	{
	    for (j = 0; j < 2; ++j)
	    {
	        if (comp[i][j][0] != comp[i][j][1])
	        {
	            crxs[num_crx] = crx_in_kdir(blk_crx,i,j);
	            if (crxs[num_crx] == NULL)
		    {
	                return FUNCTION_FAILED;
		    }
	            ++num_crx;
	        }
	    }
	}
	if (num_crx == 0)
	{
	    /* No interface, but ONFRONT, this happens */
	    blk_mem->num_tris[0] = 0;
	    return FUNCTION_SUCCEEDED;
	}
	
	for (i = 1; i < num_crx; ++i)
	{
	    if (crxs[i]->s != crxs[0]->s)
	    {  
		printf("surface_numbe of crxs[0]->s = %llu\n",
				surface_number(crxs[0]->s));    
		printf("surface_numbe of crxs[i]->s = %llu\n",
				surface_number(crxs[i]->s));
	        screen("ERROR in construct_comp2_blk(), more than "
	               "one surface in a block, code needed\n");
		(void) printf("crx[%d]->hs = %p  crx[0] = %p\n",
		              i,crxs[i]->s,crxs[0]->s);
                (void) printf("i = %d,num_crx = %d\n",i,num_crx);
		clean_up(ERROR);
	    }
	}
	if (blk_crx->nv[0] > 4)
	{
	    screen("ERROR: in construct_comp2_blk(), no such case!\n");
	    clean_up(ERROR);
	}

	is = is_surface(blk_crx,crxs[0]->s);
	blk_mem->first[is] = NULL;
	blk_mem->num_tris[is] = 0;
	blk_mem->surfs[is] = crxs[0]->s;
	if (bc_rot == NULL) bc_rot = alloc_blk_crx(NO);
	copy_blk_crx(blk_crx, bc_rot);

	/* begin 24-rotation */
	
	case_found = NO;
	for (j = 0; j <= 24; j++)
	{
	    for (i = 0; i < 14; i++)
	    {
	        if (compare_comp(bc_rot->comp, prime_comp, i)) 
	        { 
	            blk_intfc_comp2[i](bc_rot, blk_mem);
		    case_found = YES;
	            break;
	        }
	    }
	    if (case_found == YES) break;
	    rot24(bc_rot, j);
	}

	if (case_found == NO)
	{
	    (void)printf("ERROR: in construct_comp2_blk() No case is found\n");
	    for (i = 0; i < 2; i++)
	    for (j = 0; j < 2; j++) 
	    for (k = 0; k < 2; k++)
	    {
	        (void)printf("comp[%d][%d][%d] = %d\n",i,j,k,comp[i][j][k]);
	        clean_up(ERROR);
	    }
	}

	blk_mem->num_null_sides[is] = 3*blk_mem->num_tris[is];

	if (debugging("print_blk_tri"))
	{
	    (void) printf("debugging(print_blk_tri), "
			  "printing blk_mem BEFORE stitching inside.\n" );
	    print_blk_tri(blk_mem);
	}

	stitch_inside_blk(blk_mem);

	if (debugging("print_blk_tri"))
	{
	    (void) printf("debugging(print_blk_tri), "
			  "printing blk_mem AFTER stitching inside.\n" );
	    print_blk_tri(blk_mem);
	}

	return FUNCTION_SUCCEEDED;
}	/* end reconstruct_blk_intfc */



EXPORT	void stitch_inside_blk(
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
		/*#bjetbond  one curve has two bonds in one block. */
		if(ic == 1)
		    if(bm->ic[0] == bm->ic[1])
		        b = bm->bonds1[bm->ic[ic]];

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
				    printf("ERROR stitch_inside_blk, curve has negative orientation.\n");
				    clean_up(ERROR);
				}
			    	btri = link_tri_to_bond(NULL,tri1,s,b,c);
				bm->num_null_sides[bm->is[is]] -= 1;
			    }
			    else if (b->start == p1[Next_m3(side1)] &&
                                     b->end == p1[side1])
			    {
				if (orient == POSITIVE_ORIENTATION)
				{
				    printf("ERROR stitch_inside_blk, curve has positive orientation.\n");
				    clean_up(ERROR);
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

EXPORT	void stitch_adj_blk(
	BLK_TRI *bm1,
	BLK_TRI *bm2)
{
	int   n1, n2;
	int   i, j, side1, side2;
	int   ic1,ic2,is1,is2;
	TRI   *tri1, *tri2;
	FT_POINT *p1[3], *p2[3];
	BOND  *b1,*b2;
	CURVE *c1,*c2;
	if (bm1 == NULL || bm2 == NULL)
	{
	    return;
	}
	for (is1 = 0; is1 < bm1->num_surfaces; ++is1)
	{
	    for (is2 = 0; is2 < bm2->num_surfaces; ++is2)
	    {
		if (bm1->surfs[bm1->is[is1]] != bm2->surfs[bm2->is[is2]])
		    continue;
		n1 = bm1->num_tris[bm1->is[is1]];
		n2 = bm2->num_tris[bm2->is[is2]];
	  
		for (i = 0, tri1 = bm1->first[bm1->is[is1]]; i < n1; 
				++i, tri1 = tri1->next)
		{
	    	    p1[0] = Point_of_tri(tri1)[0];
	    	    p1[1] = Point_of_tri(tri1)[1];
	    	    p1[2] = Point_of_tri(tri1)[2];
	    	    for (j = 0, tri2 = bm2->first[bm2->is[is2]]; j < n2; 
		    		++j, tri2 = tri2->next)
	    	    {
	        	p2[0] = Point_of_tri(tri2)[0];
	        	p2[1] = Point_of_tri(tri2)[1];
	        	p2[2] = Point_of_tri(tri2)[2];
			for (side1 = 0; side1 < 3; ++side1)
	        	{
	            	    for (side2 = 0; side2 < 3; ++side2)
	            	    {
	                	if (p1[side1] == p2[(side2+1)%3] &&
	                    	    p1[(side1+1)%3] == p2[side2])
	                	{
	                    	    Tri_on_side(tri1,side1) = tri2;
	                    	    Tri_on_side(tri2,side2) = tri1;
	                    	    bm1->num_null_sides[bm1->is[is1]]--;
	                    	    bm2->num_null_sides[bm2->is[is2]]--;
	                	}
	            	    }
	        	}
	    	    }
		}
	    }
	}
}	/*end stitch_adj_blk*/

EXPORT	void i_reorder_curve_link_list(
	CURVE *c)
{
	BOND *b1,*b2;

	b1 = c->first;
	c->num_points = 1;
	while (b1->next != NULL)
	{
	    b2 = b1->next;
	    if (b1->end == b2->end)
		reverse_bond(b2);
	    if (b2->prev != b1)
	    {
	    	BOND *btmp = b2->next;
		b2->next = b2->prev;
		b2->prev = btmp;
	    }
	    if (b2->prev != b1)
	    {
	    	screen("ERROR in order_curve_link_list(),");
		screen("b2 is not linked to b1!\n");
		clean_up(ERROR);
	    }
	    b1 = b2;
	    ++c->num_points;
	}
	c->last = b1;
	++c->num_points;
}	/* end order_curve_link_list */


EXPORT	void remove_null_pair(
	BLK_TRI *bm1,
	BLK_TRI *bm2,
	int     dir)
{
	int is1, is2;

	if (bm2 == NULL)
	    return;
 
 	if(debugging("chk_bm"))
	{
	    find_blk_tri(bm1);
 	    find_blk_tri(bm2);
	}

	for (is1 = 0; is1 < bm1->num_surfaces; ++is1)
	{
	    for (is2 = 0; is2 < bm2->num_surfaces; ++is2)
	    {
	    	if (bm1->surfs[bm1->is[is1]] != bm2->surfs[bm2->is[is2]])
		    continue;
	    	remove_null_pair_of_surface(bm1,bm2,dir,bm1->is[is1],
					bm2->is[is2]);
	    }
	}	

	if(debugging("chk_bm"))
	{
	    char  s[40];

	    printf("#bm debug rm \n");
	    tecplot_blk_intfc_plot("blk_bm1", bm1);
	    print_blk_tri(bm1);
	    tecplot_blk_intfc_plot("blk_bm2", bm2);
	    print_blk_tri(bm2);
	}

}	/* end remove_null_pair*/

LOCAL	void remove_null_pair_of_surface(
        BLK_TRI *bm1,
        BLK_TRI *bm2,
        int     dir,
	int     is1,
	int     is2)
{
	FT_POINT    *pts[4];
	FT_POINT    *p1[3], *p2[3];
	int      it, jt, i, j, sides[4];
	int      search_step;
	TRI      *tri1, *tri2, *tris[4];
	BLK_INFO *blk_info = bm1->blk_info;

	if (bm1->num_null_sides[is1] < 2 || bm2->num_null_sides[is2] < 2) 
	{ 
	    return;
	}

	search_step = 0;
	for (tri1=bm1->first[is1], it=0; it < bm1->num_tris[is1]; 
			tri1=tri1->next, ++it)
	{
	    p1[0] = Point_of_tri(tri1)[0];
	    p1[1] = Point_of_tri(tri1)[1];
	    p1[2] = Point_of_tri(tri1)[2];
	    for (tri2=bm2->first[is2], jt=0; jt<bm2->num_tris[is2]; 
	    			tri2=tri2->next, ++jt)
	    {
	        p2[0] = Point_of_tri(tri2)[0];
	        p2[1] = Point_of_tri(tri2)[1];
	        p2[2] = Point_of_tri(tri2)[2];
	        for (i = 0; i < 3; ++i)
	        {
		    if (Tri_on_side(tri1,i) != NULL || 
		        Coords(p1[i])[dir] != Coords(p1[(i+1)%3])[dir])
			continue;
	            for (j = 0; j < 3; ++j)
	            {
		    	if (Tri_on_side(tri2,j) != NULL || 
		            Coords(p2[j])[dir] != Coords(p2[(j+1)%3])[dir])
			    continue;
	                if (Coords(p1[i])[dir] == Coords(p2[j])[dir])
	                {
	                    if (search_step == 0)
	                    {
	                        if (p1[i] == p2[(j+1)%3])
	                        {
	                            pts[0] = p2[j];
	                            pts[1] = p1[i];
	                            pts[2] = p1[(i+1)%3];
	                            tris[0] = tri2;
	                            tris[1] = tri1;
	                            sides[0] = j;
	                            sides[1] = i;
	                            search_step = 1;
	                        }
	                        else if (p1[(i+1)%3] == p2[j])
	                        {
	                            pts[0] = p1[i];
	                            pts[1] = p2[j];
	                            pts[2] = p2[(j+1)%3];
	                            tris[0] = tri1;
	                            tris[1] = tri2;
	                            sides[0] = i;
	                            sides[1] = j;
	                            search_step = 1;
	                        }
	                    }
	                    else if (search_step == 1)
	                    {
	                        if (p1[i] == p2[(j+1)%3] &&
	                            p1[(i+1)%3] == pts[0] &&
	                            p2[j] == pts[2]) 
	                        {
	                            pts[3] = p1[i];
	                            tris[2] = tri2;
	                            tris[3] = tri1;
	                            sides[2] = j;
	                            sides[3] = i;
	                            search_step = 2;
	                        }
	                        else if (p1[(i+1)%3] == p2[j] &&
	                                 p2[(j+1)%3] == pts[0] &&
	                                 p1[i] == pts[2]) 
	                        {
	                            pts[3] = p2[j];
	                            tris[2] = tri1;
	                            tris[3] = tri2;
	                            sides[2] = i;
	                            sides[3] = j;
	                            search_step = 2;
	                        }
	                    }
	                }
	            }
	        }
	    }
	}
	if (search_step == 2)
	{
	    SURFACE *s = Surface_of_tri(tris[0]);
	    for (i = 0; i < blk_info->num_surfs; ++i)
	    {
	        if (s == blk_info->surfs[i])
		    break;
	    }
	    if (blk_info->cur_tris[i] == last_tri(s))
	    {
	        tri1 = make_tri(pts[2],pts[1],pts[0],NULL,NULL,NULL,0);
		for (j = 0; j < 3; ++j)
		{
		    pts[j]->hse = Hyper_surf_element(tri1);
		    pts[j]->hs = Hyper_surf(s);
		}
		tri1->prev = bm1->first[is1];
		tri1->next = bm1->first[is1]->next;
		if (bm1->first[is1] == last_tri(s))
		{
		    last_tri(s) = tri1;
	    	    blk_info->cur_tris[i] = tri1;
		}
		else
		    bm1->first[is1]->next->prev = tri1;
		bm1->first[is1]->next = tri1;
	    }
	    else
	    {
	        tri1 = blk_info->cur_tris[i]->next;
		if (tri1 != last_tri(s))
		{
		    tri1->next->prev = blk_info->cur_tris[i];
		    blk_info->cur_tris[i]->next = tri1->next;
		}
		else
		    last_tri(s) = blk_info->cur_tris[i];
	        Point_of_tri(tri1)[2] = pts[0];
	        Point_of_tri(tri1)[1] = pts[1];
	        Point_of_tri(tri1)[0] = pts[2];
	        Boundary_tri(tri1) = 0;
		tri1->prev = bm1->first[is1];
		tri1->next = bm1->first[is1]->next;
                if (bm1->first[is1] == last_tri(s))
                    last_tri(s) = tri1;
                else
                    bm1->first[is1]->next->prev = tri1;
                bm1->first[is1]->next = tri1;
	    }
	    tri1->surf = s;
	    ++(bm1->num_tris[is1]);
	    ++(s->num_tri);

	    if (blk_info->cur_tris[i] == last_tri(s))
	    {
		tri2 = make_tri(pts[0],pts[3],pts[2],NULL,NULL,NULL,0);
		for (j = 0; j < 3; ++j)
		{
		    pts[j]->hse = Hyper_surf_element(tri2);
		    pts[j]->hs = Hyper_surf(s);
		}
		tri2->prev = bm1->first[is1];
		tri2->next = bm1->first[is1]->next;
		if (bm1->first[is1] == last_tri(s))
		{
		    last_tri(s) = tri2;
	    	    blk_info->cur_tris[i] = tri2;
		}
		else
		    bm1->first[is1]->next->prev = tri2;
		bm1->first[is1]->next = tri2;
	    }
	    else
	    {
	        tri2 = blk_info->cur_tris[i]->next;
		if (tri2 != last_tri(s))
		{
		    tri2->next->prev = blk_info->cur_tris[i];
		    blk_info->cur_tris[i]->next = tri2->next;
		}
		else
		    last_tri(s) = blk_info->cur_tris[i];
		Point_of_tri(tri2)[2] = pts[2];
                Point_of_tri(tri2)[1] = pts[3];
                Point_of_tri(tri2)[0] = pts[0];
	        Boundary_tri(tri2) = 0;
		tri2->prev = bm1->first[is1];
		tri2->next = bm1->first[is1]->next;
                if (bm1->first[is1] == last_tri(s))
                    last_tri(s) = tri2;
                else
                    bm1->first[is1]->next->prev = tri2;
                bm1->first[is1]->next = tri2;
	    }
	    tri2->surf = s;
	    ++(bm1->num_tris[is1]);
	    ++(s->num_tri);

	    Tri_on_side12(tri1) = tris[0];
	    Tri_on_side01(tri1) = tris[1];
	    Tri_on_side20(tri1) = tri2;
	    Tri_on_side(tris[0],sides[0]) = tri1;
	    Tri_on_side(tris[1],sides[1]) = tri1;

	    Tri_on_side12(tri2) = tris[2];
	    Tri_on_side01(tri2) = tris[3];
	    Tri_on_side20(tri2) = tri1;
	    Tri_on_side(tris[2],sides[2]) = tri2;
	    Tri_on_side(tris[3],sides[3]) = tri2;
	    bm1->num_null_sides[is1] -= 2;
	    bm2->num_null_sides[is2] -= 2;
	    set_normal_of_tri(tri1);
	    set_normal_of_tri(tri2);
	}
}	/* end remove_null_pair_of_surface */

LOCAL	BBI_POINT *crx_in_idir(
	const BLK_CRX *blk_crx,
	int j,
	int k)
{
	int ip[3],ipn[3];
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	BBI_POINT *bbi;

	ip[0] = ix[0][j][k];
	ip[1] = iy[0][j][k];
	ip[2] = iz[0][j][k];
	ipn[0] = ix[1][j][k];
	ipn[1] = iy[1][j][k];
	ipn[2] = iz[1][j][k];
	bbi = crxing_in_between(ip,ipn,blk_crx);
	return bbi;
}	/* end crx_in_idir */

LOCAL	BBI_POINT *crx_in_jdir(
	const BLK_CRX *blk_crx,
	int k,
	int i)
{
	int ip[3],ipn[3];
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	BBI_POINT *bbi;

	ip[0] = ix[i][0][k];
	ip[1] = iy[i][0][k];
	ip[2] = iz[i][0][k];
	ipn[0] = ix[i][1][k];
	ipn[1] = iy[i][1][k];
	ipn[2] = iz[i][1][k];
	bbi = crxing_in_between(ip,ipn,blk_crx);
        return bbi;
}	/* end crx_in_jdir */

LOCAL	BBI_POINT *crx_in_kdir(
	const BLK_CRX *blk_crx,
	int i,
	int j)
{
	int ip[3],ipn[3];
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	BBI_POINT *bbi;

	ip[0] = ix[i][j][0];
	ip[1] = iy[i][j][0];
	ip[2] = iz[i][j][0];
	ipn[0] = ix[i][j][1];
	ipn[1] = iy[i][j][1];
	ipn[2] = iz[i][j][1];
	bbi = crxing_in_between(ip,ipn,blk_crx);
	return bbi; 
}	/* end crx_in_kdir */

LOCAL	BBI_POINT *crxing_in_between(
	const int *ip,
	const int *ipn,
	const BLK_CRX *blk_crx)
{
	if (ip[0] != ipn[0])
	{
	    if (blk_crx->crx[0][ip[1]][ip[2]]->p == NULL)
	    {
		screen("ERROR in crxing_in_between(), "
		       "no crossing point between:\n"
		       "(%d %d %d) and (%d %d %d)\n",
			ip[0],ip[1],ip[2],ipn[0],ipn[1],ipn[2]);
		clean_up(ERROR);
	    }
	    return blk_crx->crx[0][ip[1]][ip[2]];
	}
	else if (ip[1] != ipn[1])
	{
	    if (blk_crx->crx[1][ip[2]][ip[0]]->p == NULL)
	    {
		screen("ERROR in crxing_in_between(), "
		       "no crossing point between:\n"
		       "(%d %d %d) and (%d %d %d)\n",
			ip[0],ip[1],ip[2],ipn[0],ipn[1],ipn[2]);
		clean_up(ERROR);
	    }
	    return blk_crx->crx[1][ip[2]][ip[0]];
	}
	else if (ip[2] != ipn[2])
	{
	    if (blk_crx->crx[2][ip[0]][ip[1]]->p == NULL)
	    {
		screen("ERROR in crxing_in_between(), "
		       "no crossing point between:\n"
		       "(%d %d %d) and (%d %d %d)\n",
			ip[0],ip[1],ip[2],ipn[0],ipn[1],ipn[2]);
		clean_up(ERROR);
	    }
	    return blk_crx->crx[2][ip[0]][ip[1]];
	}
	screen("ERROR in crxing_in_between(), "
	       "Inconsistent values of ip and ipn arrays\n");
	clean_up(ERROR);
	return NULL;
}	/* end crxing_in_between */

EXPORT	BLK_CRX *alloc_blk_crx(
	boolean alloc_BBI_POINT)
{
	BLK_CRX *blk_crx;

	scalar(&blk_crx,sizeof(BLK_CRX));
	tri_array(&blk_crx->comp,2,2,2,sizeof(COMPONENT));
	tri_array(&blk_crx->ix,2,2,2,sizeof(int));
	tri_array(&blk_crx->iy,2,2,2,sizeof(int));
	tri_array(&blk_crx->iz,2,2,2,sizeof(int));
	tri_array(&blk_crx->crx,3,2,2,sizeof(BBI_POINT*));
	quad_array(&blk_crx->corner_coords,2,2,2,3,FLOAT);
	bi_array(&blk_crx->curve_crx,3,2,sizeof(BBI_POINT*));
	if (alloc_BBI_POINT)
	{
	    int i,j,k,num_crx = 0;
	    uni_array(&blk_crx->crx_store,19,sizeof(BBI_POINT));
	    for (i = 0; i < 3; ++i)
	    for (j = 0; j < 2; ++j)
	    for (k = 0; k < 2; ++k)
	    	blk_crx->crx[i][j][k] = &blk_crx->crx_store[num_crx++];
	    for (i = 0; i < 3; ++i)
	    for (j = 0; j < 2; ++j)
	    	blk_crx->curve_crx[i][j] = &blk_crx->crx_store[num_crx++];
	    blk_crx->node_crx = &blk_crx->crx_store[num_crx++];
	}
	return blk_crx;
}	/*end alloc_blk_crx*/

EXPORT	void  create_triangle(
	BLK_TRI *blk_mem,
	FT_POINT   *p1,
	FT_POINT   *p2,
	FT_POINT   *p3,
	SURFACE *s)
{
	TRI      *tri;
	int      i,is;
	BLK_INFO *blk_info = blk_mem->blk_info;
	
	for (i = 0; i < blk_info->num_surfs; ++i)
	{
	    if (s == blk_info->surfs[i])
		break;
	}
	for (is = 0; is < blk_mem->num_surfaces; ++is)
	{
	    if (s == blk_mem->surfs[blk_mem->is[is]])
		break;
	}
	if (first_tri(s) == NULL)
	{
	    tri = make_tri(p3,p2,p1,NULL,NULL,NULL,0);
	    p1->hse = Hyper_surf_element(tri);
	    p1->hs = Hyper_surf(s);
	    p2->hse = Hyper_surf_element(tri);
	    p2->hs = Hyper_surf(s);
	    p3->hse = Hyper_surf_element(tri);
	    p3->hs = Hyper_surf(s);
	    first_tri(s) = last_tri(s) = tri;
	}
	else if (blk_info->cur_tris[i] == last_tri(s))
	{
	    tri = make_tri(p3,p2,p1,NULL,NULL,NULL,0);
	    p1->hse = Hyper_surf_element(tri);
	    p1->hs = Hyper_surf(s);
	    p2->hse = Hyper_surf_element(tri);
	    p2->hs = Hyper_surf(s);
	    p3->hse = Hyper_surf_element(tri);
	    p3->hs = Hyper_surf(s);
	    tri->prev = last_tri(s);
	    tri->next = last_tri(s)->next;
	    last_tri(s)->next = tri;
	    last_tri(s) = tri;
	}
	else
	{
	    tri = (blk_info->cur_tris[i] == NULL) ?
		first_tri(s) : blk_info->cur_tris[i]->next;
	    Point_of_tri(tri)[2] = p1;
	    Point_of_tri(tri)[1] = p2;
	    Point_of_tri(tri)[0] = p3;
	    Neighbor_on_side01(tri) = NULL;
	    Neighbor_on_side12(tri) = NULL;
	    Neighbor_on_side20(tri) = NULL;
	    Boundary_tri(tri) = 0;
	}
	tri->surf = s;
	set_normal_of_tri(tri);
	blk_info->cur_tris[i] = tri;
	++(blk_mem->num_tris[blk_mem->is[is]]);
	++(s->num_tri);
	if (blk_mem->first[blk_mem->is[is]] == NULL)
	    blk_mem->first[blk_mem->is[is]] = tri;
}	/*end create_triangle*/

EXPORT	ORIENTATION curve_surface_orientation(
	SURFACE *s,
	CURVE *c)
{
	if (pointer_is_in_array(s,c->pos_surfaces))
	    return POSITIVE_ORIENTATION;
	else if (pointer_is_in_array(s,c->neg_surfaces))
	    return NEGATIVE_ORIENTATION;
	else
	    return ORIENTATION_NOT_SET;
}	/* end curve_surface_orientation */

LOCAL   int compare_comp(
	COMPONENT ***comp,
	COMPONENT ****p_comp,
	int ii)
{
	int i,j,k;
        
	for (i = 0; i < 2; i++)
	for (j = 0; j < 2; j++)
        for (k = 0; k < 2; k++) 
	if (comp[i][j][k] != p_comp[ii][i][j][k])
	{
            return NO; 
	}
	return YES; 
}       /* end compare_comp */

LOCAL	void set_prime_components(
	COMPONENT ****pcomp)
{
	/* Case 1: */

	pcomp[0][0][0][0] = 0;
	pcomp[0][0][0][1] = 0;
	pcomp[0][0][1][0] = 0;
	pcomp[0][0][1][1] = 0;
	pcomp[0][1][0][0] = 1;
	pcomp[0][1][0][1] = 1;
	pcomp[0][1][1][0] = 1;
	pcomp[0][1][1][1] = 1;

	/* Case 2: */

	pcomp[1][0][0][0] = 0;
	pcomp[1][0][0][1] = 0;
	pcomp[1][0][1][0] = 0;
	pcomp[1][0][1][1] = 0;
	pcomp[1][1][0][0] = 0;
	pcomp[1][1][0][1] = 0;
	pcomp[1][1][1][0] = 1;
	pcomp[1][1][1][1] = 1;

	/* Case 3: */

	pcomp[2][0][0][0] = 0;
	pcomp[2][0][0][1] = 0;
	pcomp[2][0][1][0] = 0;
	pcomp[2][0][1][1] = 0;
	pcomp[2][1][0][0] = 0;
	pcomp[2][1][0][1] = 0;
	pcomp[2][1][1][0] = 0;
	pcomp[2][1][1][1] = 1;

	/* Case 4: */

	pcomp[3][0][0][0] = 0;
	pcomp[3][0][0][1] = 0;
	pcomp[3][0][1][0] = 0;
	pcomp[3][0][1][1] = 0;
	pcomp[3][1][0][0] = 0;
	pcomp[3][1][0][1] = 1;
	pcomp[3][1][1][0] = 1;
	pcomp[3][1][1][1] = 1;

	/* Case 5: */

	pcomp[4][0][0][0] = 0;
	pcomp[4][0][0][1] = 0;
	pcomp[4][0][1][0] = 0;
	pcomp[4][0][1][1] = 1;
	pcomp[4][1][0][0] = 0;
	pcomp[4][1][0][1] = 1;
	pcomp[4][1][1][0] = 1;
	pcomp[4][1][1][1] = 1;

	/* Case 6: */

	pcomp[5][0][0][0] = 0;
	pcomp[5][0][0][1] = 0;
	pcomp[5][0][1][0] = 0;
	pcomp[5][0][1][1] = 0;
	pcomp[5][1][0][0] = 0;
	pcomp[5][1][0][1] = 1;
	pcomp[5][1][1][0] = 1;
	pcomp[5][1][1][1] = 0;

	/* Case 7: */

	pcomp[6][0][0][0] = 0;
	pcomp[6][0][0][1] = 0;
	pcomp[6][0][1][0] = 0;
	pcomp[6][0][1][1] = 1;
	pcomp[6][1][0][0] = 1;
	pcomp[6][1][0][1] = 1;
	pcomp[6][1][1][0] = 0;
	pcomp[6][1][1][1] = 1;

	/* Case 8: */

	pcomp[7][0][0][0] = 0;
	pcomp[7][0][0][1] = 0;
	pcomp[7][0][1][0] = 0;
	pcomp[7][0][1][1] = 1;
	pcomp[7][1][0][0] = 1;
	pcomp[7][1][0][1] = 0;
	pcomp[7][1][1][0] = 0;
	pcomp[7][1][1][1] = 1;

	/* Case 9: */

	pcomp[8][0][0][0] = 0;
	pcomp[8][0][0][1] = 0;
	pcomp[8][0][1][0] = 0;
	pcomp[8][0][1][1] = 1;
	pcomp[8][1][0][0] = 1;
	pcomp[8][1][0][1] = 0;
	pcomp[8][1][1][0] = 0;
	pcomp[8][1][1][1] = 0;

	/* Case 10: */

	pcomp[9][0][0][0] = 0;
	pcomp[9][0][0][1] = 0;
	pcomp[9][0][1][0] = 0;
	pcomp[9][0][1][1] = 1;
	pcomp[9][1][0][0] = 1;
	pcomp[9][1][0][1] = 0;
	pcomp[9][1][1][0] = 1;
	pcomp[9][1][1][1] = 1;

	/* Case 11: */

	pcomp[10][0][0][0] = 0;
	pcomp[10][0][0][1] = 0;
	pcomp[10][0][1][0] = 1;
	pcomp[10][0][1][1] = 1;
	pcomp[10][1][0][0] = 1;
	pcomp[10][1][0][1] = 1;
	pcomp[10][1][1][0] = 0;
	pcomp[10][1][1][1] = 0;

	/* Case 12: */

	pcomp[11][0][0][0] = 0;
	pcomp[11][0][0][1] = 0;
	pcomp[11][0][1][0] = 0;
	pcomp[11][0][1][1] = 1;
	pcomp[11][1][0][0] = 0;
	pcomp[11][1][0][1] = 1;
	pcomp[11][1][1][0] = 1;
	pcomp[11][1][1][1] = 0;

	/* Case 13: */

	pcomp[12][0][0][0] = 0;
	pcomp[12][0][0][1] = 0;
	pcomp[12][0][1][0] = 0;
	pcomp[12][0][1][1] = 1;
	pcomp[12][1][0][0] = 1;
	pcomp[12][1][0][1] = 1;
	pcomp[12][1][1][0] = 1;
	pcomp[12][1][1][1] = 0;

	/* Case 14: */

	pcomp[13][0][0][0] = 0;
	pcomp[13][0][0][1] = 1;
	pcomp[13][0][1][0] = 1;
	pcomp[13][0][1][1] = 0;
	pcomp[13][1][0][0] = 1;
	pcomp[13][1][0][1] = 0;
	pcomp[13][1][1][0] = 0;
	pcomp[13][1][1][1] = 1;
}	/* end set_prime_components */

EXPORT	void copy_blk_crx(
	const BLK_CRX *blk_crx1,
	BLK_CRX       *blk_crx2)
{
	int i,j,k,l;

	blk_crx2->blk_info = blk_crx1->blk_info;
	blk_crx2->cell_volume = blk_crx1->cell_volume;
	blk_crx2->comp_vfrac = blk_crx1->comp_vfrac;
	
	for (i = 0; i < 3; ++i)
	for (j = 0; j < 2; ++j)
	for (k = 0; k < 2; ++k)
	     blk_crx2->crx = blk_crx1->crx;
	for (i = 0; i < 2; i++)
	for (j = 0; j < 2; j++)
        for (k = 0; k < 2; k++)
	{
	    if (blk_crx1->comp[i][j][k] == blk_crx1->comps[0])
	        blk_crx2->comp[i][j][k] = 1;
	    else
	        blk_crx2->comp[i][j][k] = 0;
	    blk_crx2->ix[i][j][k] = blk_crx1->ix[i][j][k];
	    blk_crx2->iy[i][j][k] = blk_crx1->iy[i][j][k];
	    blk_crx2->iz[i][j][k] = blk_crx1->iz[i][j][k];
	    for (l = 0; l < 3; ++l)
		blk_crx2->corner_coords[i][j][k][l] = 
			blk_crx1->corner_coords[i][j][k][l];
	}
	for (i = 0; i < 8; i++) 
	{
	    blk_crx2->comps[i] = blk_crx1->comps[i];
	    blk_crx2->nv[i] = blk_crx1->nv[i];
	}
}      /*end copy_blk_crx*/

EXPORT  void rot24(
        BLK_CRX *blk_crx,
	int n)
{       
        if (n < 16)
        {
            x_rotation(blk_crx);
            if ((n+1)%4 == 0)
                y_rotation(blk_crx);
        }
        else if (n == 16)
        {   
            z_rotation(blk_crx); 
            x_rotation(blk_crx); 
        }   
        else if (n == 20)
        {   
	    z_rotation(blk_crx);
	    z_rotation(blk_crx);
	    x_rotation(blk_crx);
	}
	else if (n < 24)
	{
	    x_rotation(blk_crx);
	}
}       /* end rot24 */

LOCAL   void x_rotation(BLK_CRX *blk_crx)
{
        int i, temp;
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT comp_tmp;
	
	for (i = 0; i < 2; i++)
	{ 
	    comp_tmp = comp[i][0][0];
	    comp[i][0][0] = comp[i][1][0];
	    comp[i][1][0] = comp[i][1][1];
	    comp[i][1][1] = comp[i][0][1];
	    comp[i][0][1] = comp_tmp;
		
	    temp = ix[i][0][0];
	    ix[i][0][0] = ix[i][1][0];
	    ix[i][1][0] = ix[i][1][1];
	    ix[i][1][1] = ix[i][0][1]; 
	    ix[i][0][1] = temp;
		
	    temp = iy[i][0][0];
	    iy[i][0][0] = iy[i][1][0];
	    iy[i][1][0] = iy[i][1][1];
	    iy[i][1][1] = iy[i][0][1];    
	    iy[i][0][1] = temp;
		
	    temp = iz[i][0][0];
	    iz[i][0][0] = iz[i][1][0];
	    iz[i][1][0] = iz[i][1][1];
	    iz[i][1][1] = iz[i][0][1];    
	    iz[i][0][1] = temp;
	}
}       /* end x_rotation */

LOCAL   void y_rotation(BLK_CRX *blk_crx)
{
        int j, temp;
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT comp_tmp;

	for (j = 0; j < 2; j++)
	{
            comp_tmp = comp[0][j][0];
	    comp[0][j][0] = comp[0][j][1];
	    comp[0][j][1] = comp[1][j][1];
	    comp[1][j][1] = comp[1][j][0];
	    comp[1][j][0] = comp_tmp;

	    temp = ix[0][j][0];
	    ix[0][j][0] = ix[0][j][1];
	    ix[0][j][1] = ix[1][j][1];
	    ix[1][j][1] = ix[1][j][0];
	    ix[1][j][0] = temp;

	    temp = iy[0][j][0];
	    iy[0][j][0] = iy[0][j][1];
	    iy[0][j][1] = iy[1][j][1];
	    iy[1][j][1] = iy[1][j][0];
	    iy[1][j][0] = temp;

	    temp = iz[0][j][0];
	    iz[0][j][0] = iz[0][j][1];
	    iz[0][j][1] = iz[1][j][1];
	    iz[1][j][1] = iz[1][j][0];
	    iz[1][j][0] = temp;
	}
}       /* z_rotation */

LOCAL   void z_rotation(BLK_CRX *blk_crx)
{
        int k, temp;
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	COMPONENT ***comp = blk_crx->comp;
	COMPONENT comp_tmp;

	for (k = 0; k < 2; k++)
	{
	    comp_tmp = comp[0][0][k];
	    comp[0][0][k] = comp[1][0][k];
	    comp[1][0][k] = comp[1][1][k];
	    comp[1][1][k] = comp[0][1][k];
	    comp[0][1][k] = comp_tmp;

	    temp = ix[0][0][k];
	    ix[0][0][k] = ix[1][0][k];
	    ix[1][0][k] = ix[1][1][k];
	    ix[1][1][k] = ix[0][1][k];
	    ix[0][1][k] = temp;

	    temp = iy[0][0][k];
	    iy[0][0][k] = iy[1][0][k];
	    iy[1][0][k] = iy[1][1][k];
	    iy[1][1][k] = iy[0][1][k];
	    iy[0][1][k] = temp;
		
	    temp = iz[0][0][k];
	    iz[0][0][k] = iz[1][0][k];
	    iz[1][0][k] = iz[1][1][k];
	    iz[1][1][k] = iz[0][1][k];
	    iz[0][1][k] = temp;
	}
}       /* end z_rotation */

LOCAL void blk_case01_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4;
	SURFACE *s;
        
	s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one plan_float */

        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_idir(blk_crx,1,1)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_idir(blk_crx,0,1)->p; 

	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p2,p4,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p2,p3,p4,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    double cell_volume = blk_crx->cell_volume;
	    double *crx1,*crx2,*crx3,*crx4;
	    double c1[3], c5[3],c6[3],c7[3],c8[3];
	    int i;

	    crx1 = Coords(p1);
	    crx2 = Coords(p2);
	    crx3 = Coords(p3);
	    crx4 = Coords(p4);

	    for (i = 0; i < 3; ++i)
	    {
	    	c1[i] = crn[0][0][0][i];
	    	c5[i] = crn[1][0][0][i];
            	c6[i] = crn[1][0][1][i];
            	c7[i] = crn[1][1][0][i];
            	c8[i] = crn[1][1][1][i];
	    }
	    blk_mem->area = area_plane_case01(blk_crx);
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = volume_plane_case01(blk_crx,
					c7,c8,c5,c6)/cell_volume;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - volume_plane_case01(blk_crx,
					c7,c8,c5,c6)/cell_volume;
	    }
	    else
	    	blk_mem->volume_fraction = 0.0;
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case01_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("corner 1 = %f %f %f\n",c1[0],c1[1],c1[2]);
	    	(void) printf("corner 8 = %f %f %f\n",c8[0],c8[1],c8[2]);
	    	(void) printf("crx 1 = %f %f %f\n",crx1[0],crx1[1],crx1[2]);
	    	(void) printf("crx 2 = %f %f %f\n",crx2[0],crx2[1],crx2[2]);
	    	(void) printf("crx 3 = %f %f %f\n",crx3[0],crx3[1],crx3[2]);
	    	(void) printf("crx 4 = %f %f %f\n",crx4[0],crx4[1],crx4[2]);
		(void) printf("\n\n");
	    }
	}
}       /* end blk_case01_comp2 */

LOCAL void blk_case02_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,1)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one float_tri */

	p1 = crx_in_idir(blk_crx,1,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,1)->p; 
        p3 = crx_in_idir(blk_crx,1,0)->p;
	p4 = crx_in_jdir(blk_crx,0,1)->p;
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p3,p2,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p3,p4,p2,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double *crx1,*crx2,*crx3,*crx4;
	    double c1[3],c7[3],c8[3];
	    double norm = blk_crx->cell_volume;

	    crx1 = Coords(p1);
	    crx2 = Coords(p2);
	    crx3 = Coords(p3);
	    crx4 = Coords(p4);

	    blk_mem->area = area_edge_case02(blk_crx);
	    for (i = 0; i < 3; ++i)
	    {
	    	c1[i] = crn[0][0][0][i];
	    	c7[i] = crn[1][1][0][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = volume_edge_case02(blk_crx,
					c8,c7)/norm;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - volume_edge_case02(blk_crx,
					c8,c7)/norm;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case02_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("corner 1 = %f %f %f\n",c1[0],c1[1],c1[2]);
	    	(void) printf("corner 8 = %f %f %f\n",c8[0],c8[1],c8[2]);
	    	(void) printf("crx 1 = %f %f %f\n",crx1[0],crx1[1],crx1[2]);
	    	(void) printf("crx 2 = %f %f %f\n",crx2[0],crx2[1],crx2[2]);
	    	(void) printf("crx 3 = %f %f %f\n",crx3[0],crx3[1],crx3[2]);
	    	(void) printf("crx 4 = %f %f %f\n",crx4[0],crx4[1],crx4[2]);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case02_comp2 */

LOCAL   void blk_case03_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,1)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one corner_tri */

        p1 = crx_in_idir(blk_crx,1,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,1)->p; 
        p3 = crx_in_kdir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    double cell_volume = blk_crx->cell_volume;
	    double *crx1,*crx2,*crx3;
	    double c1[3],c8[3];
	    int i;

	    crx1 = Coords(p1);
	    crx2 = Coords(p2);
	    crx3 = Coords(p3);
	    
	    for (i = 0; i < 3; ++i)
            {	
		c1[i] = crn[0][0][0][i];
		c8[i] = crn[1][1][1][i];
	    }
	    blk_mem->area = area_corner_case03(blk_crx);
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = volume_corner_case03(blk_crx,c8)
					/cell_volume;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	        blk_mem->volume_fraction = 1.0 - volume_corner_case03(blk_crx,
					c8)/cell_volume;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case03_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("corner 1 = %f %f %f\n",c1[0],c1[1],c1[2]);
	    	(void) printf("corner 8 = %f %f %f\n",c8[0],c8[1],c8[2]);
	    	(void) printf("crx 1 = %f %f %f\n",crx1[0],crx1[1],crx1[2]);
	    	(void) printf("crx 2 = %f %f %f\n",crx2[0],crx2[1],crx2[2]);
	    	(void) printf("crx 3 = %f %f %f\n",crx3[0],crx3[1],crx3[2]);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case03_comp2 */

LOCAL void blk_case04_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4, *p5;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one ceiling_corner */
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_idir(blk_crx,1,1)->p; 
        p3 = crx_in_jdir(blk_crx,0,1)->p; 
	p4 = crx_in_kdir(blk_crx,1,0)->p; 
	p5 = crx_in_idir(blk_crx,0,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p3,p2,p4,s);
	    create_triangle(blk_mem,p2,p5,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p3,p4,p2,s);
	    create_triangle(blk_mem,p2,p4,p5,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    double cell_volume = blk_crx->cell_volume;
	    double *crx1,*crx2,*crx3,*crx4,*crx5;
	    double c1[3],c6[3],c7[3],c8[3];
	    int i;

	    crx1 = Coords(p1);
	    crx2 = Coords(p2);
	    crx3 = Coords(p3);
	    crx4 = Coords(p4);
	    crx5 = Coords(p5);

	    for (i = 0; i < 3; ++i)
	    {
		c1[i] = crn[0][0][0][i];
		c6[i] = crn[1][0][1][i];
		c7[i] = crn[1][1][0][i];
            	c8[i] = crn[1][1][1][i];
	    }
	    blk_mem->area = area_glider_case04(blk_crx);
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = volume_glider_case04(blk_crx,c6,c8,
					c7)/cell_volume;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - volume_glider_case04(blk_crx,
					c6,c8,c7)/cell_volume;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case04_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("corner 1 = %f %f %f\n",c1[0],c1[1],c1[2]);
	    	(void) printf("corner 8 = %f %f %f\n",c8[0],c8[1],c8[2]);
	    	(void) printf("crx 1 = %f %f %f\n",crx1[0],crx1[1],crx1[2]);
	    	(void) printf("crx 2 = %f %f %f\n",crx2[0],crx2[1],crx2[2]);
	    	(void) printf("crx 3 = %f %f %f\n",crx3[0],crx3[1],crx3[2]);
	    	(void) printf("crx 4 = %f %f %f\n",crx4[0],crx4[1],crx4[2]);
	    	(void) printf("crx 5 = %f %f %f\n",crx5[0],crx5[1],crx5[2]);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case04_comp2 */

LOCAL void blk_case05_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one corner_star */

        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	p4 = crx_in_kdir(blk_crx,0,1)->p;
	p5 = crx_in_idir(blk_crx,0,1)->p;
	p6 = crx_in_jdir(blk_crx,1,0)->p;
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p4,p2,s);
	    create_triangle(blk_mem,p4,p5,p2,s);
	    create_triangle(blk_mem,p4,p6,p5,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p2,p4,s);
	    create_triangle(blk_mem,p4,p2,p5,s);
	    create_triangle(blk_mem,p4,p5,p6,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double *crx1,*crx2,*crx3,*crx4,*crx5,*crx6;
	    double c1[3],c4[3],c6[3],c7[3],c8[3];
	    double norm = blk_crx->cell_volume;
	    double *h = blk_crx->h;

	    crx1 = Coords(p1);
	    crx2 = Coords(p2);
	    crx3 = Coords(p3);
	    crx4 = Coords(p4);
	    crx5 = Coords(p5);
	    crx6 = Coords(p6);
	    
	    blk_mem->area = area_hexagon_case05(blk_crx);
	    for (i = 0; i < 3; ++i)
	    {
		c1[i] = crn[0][0][0][i];
		c4[i] = crn[0][1][1][i];
		c6[i] = crn[1][0][1][i];
	    	c7[i] = crn[1][1][0][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = volume_hexagon_case05(blk_crx,
					c6,c7,c4,c8,h)/norm;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - volume_hexagon_case05(blk_crx,
					c6,c7,c4,c8,h)/norm;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case05_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("corner 1 = %f %f %f\n",c1[0],c1[1],c1[2]);
	    	(void) printf("corner 8 = %f %f %f\n",c8[0],c8[1],c8[2]);
	    	(void) printf("crx 1 = %f %f %f\n",crx1[0],crx1[1],crx1[2]);
	    	(void) printf("crx 2 = %f %f %f\n",crx2[0],crx2[1],crx2[2]);
	    	(void) printf("crx 3 = %f %f %f\n",crx3[0],crx3[1],crx3[2]);
	    	(void) printf("crx 4 = %f %f %f\n",crx4[0],crx4[1],crx4[2]);
	    	(void) printf("crx 5 = %f %f %f\n",crx5[0],crx5[1],crx5[2]);
	    	(void) printf("crx 6 = %f %f %f\n",crx6[0],crx6[1],crx6[2]);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case05_comp2 */

LOCAL void blk_case06_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/*  the first corner_tri  */

        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,1)->p; 
        p3 = crx_in_jdir(blk_crx,0,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/*  the second corner_tri  */
        
	p1 = crx_in_jdir(blk_crx,1,1)->p; 
        p2 = crx_in_idir(blk_crx,0,1)->p; 
        p3 = crx_in_kdir(blk_crx,1,0)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c6[3],c7[3];
	    double norm = blk_crx->cell_volume;
	    double vol_1,vol_2;

	    blk_mem->area = area_corner1_case06(blk_crx) 
	    			+ area_corner2_case06(blk_crx);
	    for (i = 0; i < 3; ++i)
	    {
		c6[i] = crn[1][0][1][i];
	    	c7[i] = crn[1][1][0][i];
	    }
	    vol_1 = volume_corner1_case06(blk_crx,c7);
	    vol_2 = volume_corner2_case06(blk_crx,c6);
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction =  (vol_1 + vol_2)/norm;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - (vol_1 + vol_2)/norm;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case06_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case06_comp2 */


LOCAL void blk_case07_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	SURFACE *s;

        s = crx_in_kdir(blk_crx,0,1)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one ij_twist */

        p1 = crx_in_kdir(blk_crx,0,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,1,1)->p;
	p4 = crx_in_idir(blk_crx,0,1)->p; 
	p5 = crx_in_jdir(blk_crx,0,1)->p; 
	p6 = crx_in_idir(blk_crx,0,0)->p;
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p3,p2,p4,s);
	    create_triangle(blk_mem,p3,p4,p5,s);
	    create_triangle(blk_mem,p5,p4,p6,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p3,p4,p2,s);
	    create_triangle(blk_mem,p3,p5,p4,s);
	    create_triangle(blk_mem,p5,p6,p4,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c4[3],c5[3],c6[3],c8[3];
	    double norm = blk_crx->cell_volume;

	    blk_mem->area = area_twister_case07(blk_crx);
	    for (i = 0; i < 3; ++i)
	    {
	    	c4[i] = crn[0][1][1][i];
		c5[i] = crn[1][0][0][i];
		c6[i] = crn[1][0][1][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = volume_twister_case07(blk_crx,
					c5,c6,c8,c4)/norm;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - volume_twister_case07(blk_crx,
					c5,c6,c8,c4)/norm;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case07_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case07_comp2 */

LOCAL void blk_case08_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4;
	SURFACE *s;

        s = crx_in_jdir(blk_crx,0,1)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one cornor_tri */

        p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p;
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* one float_tri */

        p1 = crx_in_kdir(blk_crx,0,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,0)->p; 
        p3 = crx_in_kdir(blk_crx,1,1)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p3,p2,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p3,p4,p2,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c4[3],c5[3],c8[3];
	    double norm = blk_crx->cell_volume;
	    double vol_corner,vol_edge;

	    blk_mem->area = area_corner_case08(blk_crx);
	    blk_mem->area += area_edge_case08(blk_crx);
	    for (i = 0; i < 3; ++i)
	    {
	    	c4[i] = crn[0][1][1][i];
		c5[i] = crn[1][0][0][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    vol_corner = volume_corner_case08(blk_crx,c5);
	    vol_edge = volume_edge_case08(blk_crx,c4,c8);
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = (vol_corner + vol_edge)/norm;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - (vol_corner + vol_edge)/norm;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case08_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case08_comp2 */

LOCAL void blk_case09_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3;
	SURFACE *s;

        s = crx_in_jdir(blk_crx,0,1)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* the first corner_tri */

        p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p;
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* the second corner_tri */

        p1 = crx_in_kdir(blk_crx,0,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c4[3],c5[3];
	    double norm = blk_crx->cell_volume;
	    double area_1,area_2,vol_1,vol_2;

	    area_1 = area_corner1_case09(blk_crx);
	    area_2 = area_corner2_case09(blk_crx);
	    blk_mem->area = area_1 + area_2;
	    for (i = 0; i < 3; ++i)
	    {
	    	c4[i] = crn[0][1][1][i];
	    	c5[i] = crn[1][0][0][i];
	    }
	    vol_1 = volume_corner_case08(blk_crx,c5);
	    vol_2 = volume_corner2_case09(blk_crx,c4);
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = (vol_1 + vol_2)/norm;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - (vol_1 + vol_2)/norm;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case09_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case09_comp2 */

LOCAL void blk_case10_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one ji_twist */

        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	p5 = crx_in_kdir(blk_crx,0,1)->p; 
	p6 = crx_in_jdir(blk_crx,1,0)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p4,p2,s);
	    create_triangle(blk_mem,p1,p5,p4,s);
	    create_triangle(blk_mem,p5,p6,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p2,p4,s);
	    create_triangle(blk_mem,p1,p4,p5,s);
	    create_triangle(blk_mem,p5,p4,p6,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c4[3],c5[3],c7[3],c8[3];
	    double norm = blk_crx->cell_volume;

	    blk_mem->area = area_twister_case10(blk_crx);
	    for (i = 0; i < 3; ++i)
	    {
		c4[i] = crn[0][1][1][i];
		c5[i] = crn[1][0][0][i];
	    	c7[i] = crn[1][1][0][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = volume_twister_case10(blk_crx,
					c4,c8,c7,c5)/norm;
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - volume_twister_case10(blk_crx,
					c4,c8,c7,c5)/norm;
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case10_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case10_comp2 */


LOCAL void blk_case11_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s); 
	/* the first float_tri */

        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_jdir(blk_crx,0,0)->p; 
        p3 = crx_in_jdir(blk_crx,1,0)->p; 
	p4 = crx_in_idir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p4,p1,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p4,p3,p1,s);
	}

	/* the second float_tri */

        p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_idir(blk_crx,0,1)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p4,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p2,p4,s);
	}
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c1[3],c2[3],c3[3],c4[3],c5[3],c6[3],c7[3],c8[3];
	    double norm = blk_crx->cell_volume;
	    double vol_11, vol_12, vol_21, vol_22, area_1, area_2;

	    area_1 = area_edge1_case11(blk_crx);
	    area_2 = area_edge2_case11(blk_crx);
	    blk_mem->area = area_1 + area_2;
	    for (i = 0; i < 3; ++i)
	    {
	    	c1[i] = crn[0][0][0][i];
		c2[i] = crn[0][0][1][i];
	    	c3[i] = crn[0][1][0][i];
		c4[i] = crn[0][1][1][i];
		c5[i] = crn[1][0][0][i];
		c6[i] = crn[1][0][1][i];
		c7[i] = crn[1][1][0][i];
		c8[i] = crn[1][1][1][i];
	    }
	    /* volumes of two edges from first interface perimeter */
	    vol_11 = volume_edge1_case11(blk_crx,c3,c4);
	    vol_12 = volume_edge2_case11(blk_crx,c6,c5);
	    /* volumes of two edges from second interface perimeter  */
	    vol_21 = volume_edge1_p2_case11(blk_crx,c1,c2);
	    vol_22 = volume_edge_case02(blk_crx,c7,c8);
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = (vol_11 + vol_12 + norm - vol_21 
						- vol_22)/(2*norm);
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - (vol_11 + vol_12 + norm 
						- vol_21 - vol_22)/(2*norm);
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case11_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case11_comp2 */

LOCAL void blk_case12_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem) 
{
        FT_POINT *p1, *p2, *p3;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* the first corner_tri */

        p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p; 
        p3 = crx_in_jdir(blk_crx,0,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* the second corner_tri */

        p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* the third corner_tri */
	
        p1 = crx_in_jdir(blk_crx,1,1)->p; 
        p2 = crx_in_idir(blk_crx,0,1)->p; 
        p3 = crx_in_kdir(blk_crx,1,0)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c4[3],c6[3],c7[3],c8[3];
	    double norm = blk_crx->cell_volume;
	    double vol_1,vol_2,vol_3,vol_hex,vol_cor,area_1,area_2,area_3;
	    double *h = blk_crx->h;

	    area_1 = area_corner1_case12(blk_crx);
	    area_2 = area_corner2_case12(blk_crx);
	    area_3 = area_corner3_case12(blk_crx);
	    blk_mem->area = area_1 + area_2 + area_3;
	    for (i = 0; i < 3; ++i)
	    {
		c4[i] = crn[0][1][1][i];
		c6[i] = crn[1][0][1][i];
	    	c7[i] = crn[1][1][0][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    /* volumes of 3 corners from first interface perimeter  */
	    vol_1 = volume_corner1_case06(blk_crx,c7);
	    vol_2 = volume_corner2_case09(blk_crx,c4);
	    vol_3 = volume_corner2_case06(blk_crx,c6);
	    /* volumes of hexagon and corner from second interface perimeter */
	    vol_hex = volume_hexagon_case05(blk_crx,c6,c7,c4,c8,h); 
	    vol_cor = volume_corner_case03(blk_crx,c8);

	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = (vol_1 + vol_2 + vol_3 + vol_hex 
						- vol_cor)/(2*norm);
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - (vol_1 + vol_2 + vol_3 
						+ vol_hex - vol_cor)/(2*norm);
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case12_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case12_comp2 */

LOCAL void blk_case13_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3, *p4, *p5;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* one ceiling_corner */

        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,1)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	p5 = crx_in_idir(blk_crx,0,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p2,p4,p3,s);
	    create_triangle(blk_mem,p4,p5,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p2,p3,p4,s);
	    create_triangle(blk_mem,p4,p3,p5,s);
	}

	/* one corner_tri */

        p1 = crx_in_kdir(blk_crx,0,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c1[3],c2[3],c3[3],c4[3],c5[3],c6[3],c7[3],c8[3];
	    double norm = blk_crx->cell_volume;
	    double vol_glider1, vol_corner1, vol_glider2, vol_corner2; 
	    double ar_glider, ar_corner;

	    ar_glider = area_glider_case13(blk_crx);
	    ar_corner = area_corner_case13(blk_crx);
	    blk_mem->area = ar_corner + ar_glider;
	    for (i = 0; i < 3; ++i)
	    {
		c1[i] = crn[0][0][0][i];
		c2[i] = crn[0][0][1][i];
		c3[i] = crn[0][1][0][i];
		c4[i] = crn[0][1][1][i];
		c5[i] = crn[1][0][0][i];
		c6[i] = crn[1][0][1][i];
	    	c7[i] = crn[1][1][0][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    /* volumes of corner and glider from first interface perimeter  */
	    vol_glider1 = volume_glider_case13(blk_crx,c7,c5,c6);
	    vol_corner1 = volume_corner2_case09(blk_crx,c4);
	    /* volumes of corner and glider from second interface perimeter  */
	    vol_glider2 = volume_glider_p2_case13(blk_crx,c2,c1,c3);
	    vol_corner2 = volume_corner_case03(blk_crx,c8);

	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = (vol_glider1 + vol_corner1 + norm
				- vol_glider2 - vol_corner2)/(2*norm);
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - (vol_glider1 + vol_corner1 
				+ norm - vol_glider2 - vol_corner2)/(2*norm);
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case13_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case13_comp2 */

LOCAL void blk_case14_comp2(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT *p1, *p2, *p3;
	SURFACE *s;

        s = crx_in_idir(blk_crx,1,0)->s;
        blk_mem->is[0]= is_surface(blk_crx,s);
	/* the first corner_tri */

        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_jdir(blk_crx,0,0)->p; 
        p3 = crx_in_kdir(blk_crx,0,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* the second corner_tri */

        p1 = crx_in_kdir(blk_crx,1,1)->p; 
        p2 = crx_in_idir(blk_crx,1,1)->p; 
        p3 = crx_in_jdir(blk_crx,1,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* the third corner_tri */

        p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* the fourth corner_tri */

        p1 = crx_in_jdir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,0,0)->p; 
        p3 = crx_in_idir(blk_crx,0,1)->p; 
	if (blk_crx->comps[0] != positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
	if (!blk_crx->blk_info->do_volume_frac) 
	    return;
	else
	{
	    double ****crn = blk_crx->corner_coords;
	    int i;
	    double c1[3],c2[3],c3[3],c4[3],c5[3],c6[3],c7[3],c8[3];
	    double norm = blk_crx->cell_volume;
	    double vol_11, vol_12, vol_13, vol_14, vol_21, vol_22, vol_23, 
	    	  vol_24, vol_31, vol_32, vol_hex_3, vol_41, vol_42, vol_hex_4,
		  vol_51, vol_52, vol_hex_5, vol_61, vol_62, vol_hex_6;
	    double area_1, area_2, area_3, area_4;
	    double *h = blk_crx->h;

	    area_1 = area_corner1_case14(blk_crx);
	    area_2 = area_corner2_case14(blk_crx);
	    area_3 = area_corner3_case14(blk_crx);
	    area_4 = area_corner4_case14(blk_crx); 
	    blk_mem->area = area_1 + area_2 + area_3 + area_4;
	    for (i = 0; i < 3; ++i)
	    {
	    	c1[i] = crn[0][0][0][i];
	    	c2[i] = crn[0][0][1][i];
		c3[i] = crn[0][1][0][i];
		c4[i] = crn[0][1][1][i];
		c5[i] = crn[1][0][0][i];
		c6[i] = crn[1][0][1][i];
	    	c7[i] = crn[1][1][0][i];
	    	c8[i] = crn[1][1][1][i];
	    }
	    /* volumes of 4 corners from first interface perimeter  */
	    vol_11 = volume_corner1_case14(blk_crx,c3);
	    vol_12 = volume_corner_case03(blk_crx,c8);
	    vol_13 = volume_corner_case08(blk_crx,c5);
	    vol_14 = volume_corner4_case14(blk_crx,c2);
	    /* volumes of 4 corners from second interface perimeter  */
	    vol_21 = volume_corner1_p2_case14(blk_crx,c1);
	    vol_22 = volume_corner1_case06(blk_crx,c7);
	    vol_23 = volume_corner2_case06(blk_crx,c6);
	    vol_24 = volume_corner2_case09(blk_crx,c4);
	    /* volumes of 2 corners and hexagon from third interface perimeter  */
	    vol_31 = volume_corner1_p2_case14(blk_crx,c1);
	    vol_32 = volume_corner_case03(blk_crx,c8);
	    vol_hex_3 = volume_hexagon_case05(blk_crx,c6,c7,c4,c8,h);
	    /* volumes of 2 corners and hexagon from fourth interface perimeter  */
	    vol_41 = volume_corner1_case14(blk_crx,c3);
	    vol_42 = volume_corner2_case06(blk_crx,c6);
	    vol_hex_4 = volume_hexagon_p4_case14(blk_crx,c2,c5,c8,c6,h);
	    /* volumes of 2 corners and hexagon from fifth interface perimeter  */
	    vol_51 = volume_corner4_case14(blk_crx,c2);
	    vol_52 = volume_corner1_case06(blk_crx,c7);
	    vol_hex_5 = volume_hexagon_p5_case14(blk_crx,c3,c8,c5,c7,h);
	    /* volumes of 2 corners and hexagon from sixth interface perimeter  */
	    vol_61 = volume_corner_case08(blk_crx,c5);
	    vol_62 = volume_corner2_case09(blk_crx,c4);
	    vol_hex_6 = volume_hexagon_p6_case14(blk_crx,c8,c3,c2,c4,h);
	    
	    
	    if (blk_crx->comp_vfrac == negative_component(s))
	    {
	    	blk_mem->volume_fraction = (vol_11 + vol_12 + vol_13 + vol_14
				+ 2*norm - vol_21 - vol_22 - vol_23 - vol_24 
				- vol_31 + vol_32 - vol_hex_3 + vol_41 - vol_42
				+ vol_hex_4 + vol_51 - vol_52 + vol_hex_5 
				+ vol_61 - vol_62 + vol_hex_6)/(6*norm);
	    }
	    else if (blk_crx->comp_vfrac == positive_component(s))
	    {
	    	blk_mem->volume_fraction = 1.0 - (vol_11 + vol_12 + vol_13 
	    			+ vol_14 + 2*norm - vol_21 - vol_22 - vol_23 
				- vol_24 - vol_31 + vol_32 - vol_hex_3 + vol_41
				- vol_42 + vol_hex_4 + vol_51 - vol_52 
				+ vol_hex_5 + vol_61 - vol_62 
				+ vol_hex_6)/(6*norm);
	    }
	    else
	    {
	    	blk_mem->volume_fraction = 0.0;
	    }
	    if (debugging("vol_frac"))
	    {
		int j,k;
	    	(void) printf("In blk_case14_comp2() doing volume fraction\n");
		(void) printf("blk_crx->comps = %d %d\n",
				blk_crx->comps[0],blk_crx->comps[1]);
		(void) printf("blk_crx->nv = %d %d\n",
				blk_crx->nv[0],blk_crx->nv[1]);
		(void) printf("blk_crx->comp_vfrac = %d\n",blk_crx->comp_vfrac);
		(void) printf("neg_component(s) = %d\n",negative_component(s));
		(void) printf("pos_component(s) = %d\n",positive_component(s));
	    	(void) printf("area = %f  volume_fraction = %f\n",blk_mem->area,
				blk_mem->volume_fraction);
	    	(void) printf("\n\n");
	    }
	}
}       /* end blk_case14_comp2 */


LOCAL double area_corner_case03(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
        p1 = crx_in_idir(blk_crx,1,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,1)->p; 
        p3 = crx_in_kdir(blk_crx,1,1)->p; 
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner_case03 */

LOCAL double area_edge_case02(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3];
	double area;
	
	p1 = crx_in_idir(blk_crx,1,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,1)->p; 
        p3 = crx_in_idir(blk_crx,1,0)->p;
	p4 = crx_in_jdir(blk_crx,0,1)->p;

	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
            D2[i] = crx4[i] - crx1[i];
            D3[i] = crx3[i] - crx1[i];
            D4[i] = crx1[i] - crx2[i];
	    D5[i] = crx3[i] - crx2[i];
            D6[i] = crx4[i] - crx2[i];
	}
	
	Cross3d(D1,D2,A1);
        Cross3d(D2,D3,A2);
        Cross3d(D4,D5,A3);
        Cross3d(D5,D6,A4);
	
	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4) )/4.0;
	return area;
}	/* end area_edge_case02 */

LOCAL double area_edge1_case11(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3];
	double area;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_jdir(blk_crx,0,0)->p; 
        p3 = crx_in_jdir(blk_crx,1,0)->p; 
	p4 = crx_in_idir(blk_crx,1,1)->p;

	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
            D2[i] = crx4[i] - crx1[i];
            D3[i] = crx3[i] - crx1[i];
            D4[i] = crx4[i] - crx3[i];
            D5[i] = crx4[i] - crx2[i];
	}
	
	Cross3d(D1,D3,A1);
        Cross3d(D3,D2,A2);
        Cross3d(D2,D5,A3);
        Cross3d(D5,D4,A4);
	
	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4) )/4.0;
	return area;
}	/* end area_edge1_case11 */

LOCAL double area_edge2_case11(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3];
	double area;
	
        p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_idir(blk_crx,0,1)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 

	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
            D2[i] = crx4[i] - crx1[i];
            D3[i] = crx3[i] - crx1[i];
            D4[i] = crx1[i] - crx3[i];
	    D5[i] = crx4[i] - crx3[i];
            D6[i] = crx2[i] - crx3[i];
	}
	
	Cross3d(D2,D1,A1);
        Cross3d(D1,D3,A2);
        Cross3d(D4,D5,A3);
        Cross3d(D5,D6,A4);
	
	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4) )/4.0;
	return area;
}	/* end area_edge2_case11 */

LOCAL double area_plane_case01(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3]; 
	double A1[3], A2[3], A3[3], A4[3];
	double area;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_idir(blk_crx,1,1)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_idir(blk_crx,0,1)->p;
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4);
	
	for (i = 0; i < 3; i++)
	{       
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx3[i] - crx2[i];
	    D3[i] = crx4[i] - crx2[i];
	    D4[i] = crx2[i] - crx1[i];
	    D5[i] = crx4[i] - crx1[i];
	    D6[i] = crx3[i] - crx1[i];
	}       
        
	Cross3d(D1,D2,A1);
        Cross3d(D2,D3,A2);
        Cross3d(D4,D5,A3);
        Cross3d(D5,D6,A4);
	
	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4) )/4.0;
	return area;
}	/* end area_plane_case01 */

LOCAL double area_glider_case04(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5;
	double *crx1, *crx2, *crx3, *crx4, *crx5;
	double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3];
	double A1[3], A2[3], A3[3], A4[3], A5[3], A6[3], A7[3], A8[3], 
	      A9[3], A10[3], A11[3], A12[3], A13[3], A14[3], A15[3];
	double area;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_idir(blk_crx,1,1)->p; 
        p3 = crx_in_jdir(blk_crx,0,1)->p; 
	p4 = crx_in_kdir(blk_crx,1,0)->p; 
	p5 = crx_in_idir(blk_crx,0,1)->p; 
	
	crx1=Coords(p1);
	crx2=Coords(p2);
	crx3=Coords(p3);
	crx4=Coords(p4);
	crx5=Coords(p5);
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i]; 	E11[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i]; 	E12[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i]; 	E13[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i]; 	E14[i]=-E4[i];
	    E5[i]=crx2[i]-crx3[i]; 	E15[i]=-E5[i];
	    E6[i]=crx2[i]-crx4[i]; 	E16[i]=-E6[i];
	    E7[i]=crx2[i]-crx5[i]; 	E17[i]=-E7[i];
	    E8[i]=crx3[i]-crx4[i]; 	E18[i]=-E8[i];
	    E9[i]=crx3[i]-crx5[i]; 	E19[i]=-E9[i];
	    E10[i]=crx4[i]-crx5[i]; 	E20[i]=-E10[i];	
	}
			
	Cross3d(E11,E14,A1);
	Cross3d(E14,E13,A2);
	Cross3d(E13,E12,A3);
	
	Cross3d(E1,E15,A4);
	Cross3d(E15,E16,A5);
	Cross3d(E16,E17,A6);
	
	Cross3d(E2,E5,A7);
	Cross3d(E5,E19,A8);
	Cross3d(E19,E18,A9);
	
	Cross3d(E7,E4,A10);
	Cross3d(E4,E9,A11);
	Cross3d(E9,E10,A12);
	
	Cross3d(E20,E6,A13);
	Cross3d(E6,E3,A14);
	Cross3d(E3,E8,A15);

	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4)+Mag3d(A5)
			+Mag3d(A6)+Mag3d(A7)+Mag3d(A8)+Mag3d(A9)
			+Mag3d(A10)+Mag3d(A11)+Mag3d(A12)+Mag3d(A13)
			+Mag3d(A14)+Mag3d(A15) )/10.0;
	return area;
}	/* end area_glider_case04 */

LOCAL double area_glider_case13(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5;
	double *crx1, *crx2, *crx3, *crx4, *crx5;
	double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3];
	double A1[3], A2[3], A3[3], A4[3], A5[3], A6[3], A7[3], A8[3], 
	      A9[3], A10[3], A11[3], A12[3], A13[3], A14[3], A15[3];
	double area;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,1)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	p5 = crx_in_idir(blk_crx,0,1)->p; 
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	
	for (i=0; i < 3; i++)
	{
		E1[i]=crx1[i]-crx2[i]; 		E11[i]=-E1[i];
		E2[i]=crx1[i]-crx3[i]; 		E12[i]=-E2[i];
		E3[i]=crx1[i]-crx4[i]; 		E13[i]=-E3[i];
		E4[i]=crx1[i]-crx5[i]; 		E14[i]=-E4[i];
		E5[i]=crx2[i]-crx3[i]; 		E15[i]=-E5[i];
		E6[i]=crx2[i]-crx4[i]; 		E16[i]=-E6[i];
		E7[i]=crx2[i]-crx5[i]; 		E17[i]=-E7[i];
		E8[i]=crx3[i]-crx4[i]; 		E18[i]=-E8[i];
		E9[i]=crx3[i]-crx5[i]; 		E19[i]=-E9[i];
		E10[i]=crx4[i]-crx5[i]; 	E20[i]=-E10[i];	
	}
			
	Cross3d(E11,E13,A1);
	Cross3d(E13,E14,A2);
	Cross3d(E14,E12,A3);
	
	Cross3d(E1,E15,A4);
	Cross3d(E15,E17,A5);
	Cross3d(E17,E16,A6);
	
	Cross3d(E2,E5,A7);
	Cross3d(E5,E18,A8);
	Cross3d(E18,E19,A9);
	
	Cross3d(E6,E3,A10);
	Cross3d(E3,E8,A11);
	Cross3d(E8,E20,A12);
	
	Cross3d(E10,E7,A13);
	Cross3d(E7,E4,A14);
	Cross3d(E4,E9,A15);

	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4)+Mag3d(A5)
			+Mag3d(A6)+Mag3d(A7)+Mag3d(A8)+Mag3d(A9)
			+Mag3d(A10)+Mag3d(A11)+Mag3d(A12)+Mag3d(A13)
			+Mag3d(A14)+Mag3d(A15) )/10.0;
	return area;
}	/* end area_glider_case11 */


LOCAL double area_hexagon_case05(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
        double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
	double A1[3], A2[3], A3[3], A4[3], A5[3], A6[3], A7[3], A8[3],
	      A9[3], A10[3], A11[3], A12[3], A13[3], A14[3], A15[3], 
	      A16[3], A17[3], A18[3], A19[3], A20[3], A21[3], A22[3], 
	      A23[3], A24[3], A25[3], A26[3], A27[3], A28[3], A29[3], 
	      A30[3], A31[3], A32[3], A33[3], A34[3], A35[3], A36[3], 
	      A37[3], A38[3], A39[3], A40[3], A41[3], A42[3], A43[3], 
	      A44[3], A45[3], A46[3], A47[3], A48[3], A49[3], A50[3], 
	      A51[3], A52[3], A53[3], A54[3], A55[3], A56[3];
	double area;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	p4 = crx_in_kdir(blk_crx,0,1)->p;
	p5 = crx_in_idir(blk_crx,0,1)->p;
	p6 = crx_in_jdir(blk_crx,1,0)->p;
	
	crx1=Coords(p1);
	crx2=Coords(p2);
	crx3=Coords(p3);
	crx4=Coords(p4);
	crx5=Coords(p5);
	crx6=Coords(p6);	
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];	E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	}
	
	/* group A */        
	Cross3d(E18,E20,A1);	Cross3d(E20,E19,A2);	
	Cross3d(E19,E16,A3);	Cross3d(E16,E17,A4);
        
	Cross3d(E2,E25,A5);	Cross3d(E25,E27,A6);	
	Cross3d(E27,E26,A7);	Cross3d(E26,E6,A8);
        
	Cross3d(E23,E24,A9);	Cross3d(E24,E22,A10);	
	Cross3d(E22,E1,A11);	Cross3d(E1,E21,A12);
        
	Cross3d(E8,E11,A13);	Cross3d(E11,E4,A14);	
	Cross3d(E4,E13,A15);	Cross3d(E13,E30,A16);
        
	Cross3d(E15,E9,A17);	Cross3d(E9,E12,A18);	
	Cross3d(E12,E5,A19);	Cross3d(E5,E14,A20);
        
	Cross3d(E29,E28,A21);	Cross3d(E28,E7,A22);	
	Cross3d(E7,E10,A23);	Cross3d(E10,E3,A24);
 	 
	/* group B */	
	Cross3d(E30,E13,A25);	Cross3d(E13,E4,A26);	
	Cross3d(E19,E16,A27);	Cross3d(E16,E17,A28);
        
	Cross3d(E29,E28,A29);	Cross3d(E28,E7,A30);	
	Cross3d(E22,E1,A31);	Cross3d(E1,E21,A32);
        
	Cross3d(E15,E9,A33);	Cross3d(E9,E12,A34);	
	Cross3d(E27,E25,A35);	Cross3d(E25,E2,A36);
   	
	Cross3d(E14,E5,A37);	Cross3d(E5,E12,A38);	
	Cross3d(E27,E26,A39);	Cross3d(E26,E6,A40);
        
	Cross3d(E18,E20,A41);	Cross3d(E20,E19,A42);	
	Cross3d(E4,E11,A43);	Cross3d(E11,E8,A44);
        
	Cross3d(E3,E10,A45);	Cross3d(E10,E7,A46);	
	Cross3d(E22,E24,A47);	Cross3d(E24,E23,A48);
        
	/* group C */	
	Cross3d(E30,E13,A49);	Cross3d(E13,E11,A50);	
	Cross3d(E11,E8,A51);	Cross3d(E3,E10,A52);
        
	Cross3d(E15,E9,A53);	Cross3d(E9,E5,A54);	
	Cross3d(E5,E14,A55);	Cross3d(E6,E2,A56);

	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4)+Mag3d(A5)
			     +Mag3d(A6)+Mag3d(A7)+Mag3d(A8)+Mag3d(A9)
			     +Mag3d(A10)+Mag3d(A11)+Mag3d(A12)+Mag3d(A13)
			     +Mag3d(A14)+Mag3d(A15)+Mag3d(A16)+Mag3d(A17)
			     +Mag3d(A18)+Mag3d(A19)+Mag3d(A20)+Mag3d(A21)
			     +Mag3d(A22)+Mag3d(A23)+Mag3d(A24)+Mag3d(A25)
			     +Mag3d(A26)+Mag3d(A27)+Mag3d(A28)+Mag3d(A29)
			     +Mag3d(A30)+Mag3d(A31)+Mag3d(A32)+Mag3d(A33)
			     +Mag3d(A34)+Mag3d(A35)+Mag3d(A36)+Mag3d(A37)
			     +Mag3d(A38)+Mag3d(A39)+Mag3d(A40)+Mag3d(A41)
			     +Mag3d(A42)+Mag3d(A43)+Mag3d(A44)+Mag3d(A45)
			     +Mag3d(A46)+Mag3d(A47)+Mag3d(A48)+Mag3d(A49)
			     +Mag3d(A50)+Mag3d(A51)+Mag3d(A52)+Mag3d(A53)
			     +Mag3d(A54)+Mag3d(A55)+Mag3d(A56) )/28.0;
	return area;
}	/* end area_hexagon_case05 */

LOCAL double area_corner1_case06(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,1)->p; 
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner1_case06 */

LOCAL double area_corner2_case06(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_jdir(blk_crx,1,1)->p; 
        p2 = crx_in_idir(blk_crx,0,1)->p; 
        p3 = crx_in_kdir(blk_crx,1,0)->p; 
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner2_case06 */

LOCAL double area_twister_case07(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
	double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
	double A1[3], A2[3], A3[3], A4[3], A5[3], A6[3], A7[3], A8[3], 
	      A9[3], A10[3], A11[3], A12[3], A13[3], A14[3], A15[3], 
	      A16[3], A17[3], A18[3], A19[3], A20[3], A21[3], A22[3], 
	      A23[3], A24[3], A25[3], A26[3], A27[3], A28[3], A29[3], 
	      A30[3], A31[3], A32[3], A33[3], A34[3], A35[3], A36[3], 
	      A37[3], A38[3], A39[3], A40[3], A41[3], A42[3], A43[3], 
	      A44[3], A45[3], A46[3], A47[3], A48[3], A49[3], A50[3], 
	      A51[3], A52[3], A53[3], A54[3], A55[3], A56[3];
	double area;
	
        p1 = crx_in_kdir(blk_crx,0,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,1,1)->p;
	p4 = crx_in_idir(blk_crx,0,1)->p; 
	p5 = crx_in_jdir(blk_crx,0,1)->p; 
	p6 = crx_in_idir(blk_crx,0,0)->p;
	
	crx1=Coords(p1);
	crx2=Coords(p2);
	crx3=Coords(p3);
	crx4=Coords(p4);
	crx5=Coords(p5);
	crx6=Coords(p6);	

	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
   	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];      E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	}
	
	/* group A */	
	Cross3d(E17,E19,A1);    Cross3d(E19,E20,A2);    
	Cross3d(E20,E18,A3);    Cross3d(E18,E16,A4);
        
	Cross3d(E1,E21,A5);     Cross3d(E21,E23,A6);    
	Cross3d(E23,E24,A7);    Cross3d(E24,E22,A8);
        
	Cross3d(E29,E28,A9);    Cross3d(E28,E10,A10);   
	Cross3d(E10,E3,A11);    Cross3d(E3,E7,A12);
        
	Cross3d(E14,E9,A13);    Cross3d(E9,E5,A14);     
	Cross3d(E5,E12,A15);    Cross3d(E12,E15,A16);
        
	Cross3d(E30,E13,A17);   Cross3d(E13,E8,A18);    
	Cross3d(E8,E4,A19);     Cross3d(E4,E11,A20);
        
	Cross3d(E26,E27,A21);   Cross3d(E27,E25,A22);   
	Cross3d(E25,E6,A23);    Cross3d(E6,E2,A24);

	/* group B */       
	Cross3d(E15,E12,A25);   Cross3d(E12,E5,A26);    
	Cross3d(E20,E18,A27);   Cross3d(E18,E16,A28);
        
	Cross3d(E26,E27,A29);   Cross3d(E27,E25,A30);   
	Cross3d(E10,E3,A31);    Cross3d(E3,E7,A32);
        
	Cross3d(E30,E13,A33);   Cross3d(E13,E8,A34);    
	Cross3d(E23,E21,A35);   Cross3d(E21,E1,A36);
        
	Cross3d(E11,E4,A37);    Cross3d(E4,E8,A38);     
	Cross3d(E23,E24,A39);   Cross3d(E24,E22,A40);
        
	Cross3d(E17,E19,A41);   Cross3d(E19,E20,A42);   
	Cross3d(E5,E9,A43);     Cross3d(E9,E14,A44);
        
	Cross3d(E2,E6,A45);     Cross3d(E6,E25,A46);    
	Cross3d(E10,E28,A47);   Cross3d(E28,E29,A48);
       
	/* group C */	
	Cross3d(E15,E12,A49);   Cross3d(E12,E9,A50);    
	Cross3d(E9,E14,A51);    Cross3d(E2,E6,A52);
        
	Cross3d(E30,E13,A53);   Cross3d(E13,E4,A54);    
	Cross3d(E4,E11,A55);    Cross3d(E22,E1,A56);
	
	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4)+Mag3d(A5)
			     +Mag3d(A6)+Mag3d(A7)+Mag3d(A8)+Mag3d(A9)
			     +Mag3d(A10)+Mag3d(A11)+Mag3d(A12)+Mag3d(A13)
			     +Mag3d(A14)+Mag3d(A15)+Mag3d(A16)+Mag3d(A17)
			     +Mag3d(A18)+Mag3d(A19)+Mag3d(A20)+Mag3d(A21)
			     +Mag3d(A22)+Mag3d(A23)+Mag3d(A24)+Mag3d(A25)
			     +Mag3d(A26)+Mag3d(A27)+Mag3d(A28)+Mag3d(A29)
			     +Mag3d(A30)+Mag3d(A31)+Mag3d(A32)+Mag3d(A33)
			     +Mag3d(A34)+Mag3d(A35)+Mag3d(A36)+Mag3d(A37)
			     +Mag3d(A38)+Mag3d(A39)+Mag3d(A40)+Mag3d(A41)
			     +Mag3d(A42)+Mag3d(A43)+Mag3d(A44)+Mag3d(A45)
			     +Mag3d(A46)+Mag3d(A47)+Mag3d(A48)+Mag3d(A49)
			     +Mag3d(A50)+Mag3d(A51)+Mag3d(A52)+Mag3d(A53)
			     +Mag3d(A54)+Mag3d(A55)+Mag3d(A56) )/28.0;
	return area;
}	/* end area_twister_case07 */

LOCAL double area_corner_case08(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_jdir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	p3 = crx_in_idir(blk_crx,0,0)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner_case08 */

LOCAL double area_edge_case08(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3];
	double area;
	
	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,1,1)->p;
        p4 = crx_in_jdir(blk_crx,1,1)->p;
				
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
            D2[i] = crx4[i] - crx1[i];
            D3[i] = crx3[i] - crx1[i];
            D4[i] = crx1[i] - crx3[i];
	    D5[i] = crx4[i] - crx3[i];
            D6[i] = crx2[i] - crx3[i];
	}
	
	Cross3d(D2,D1,A1);
        Cross3d(D1,D3,A2);
        Cross3d(D4,D5,A3);
        Cross3d(D5,D6,A4);
	
	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4) )/4.0;
	return area;
}	/* end area_edge_case08 */

LOCAL double area_corner1_case09(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_jdir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner1_case09 */

LOCAL double area_corner2_case09(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
			
        crx1 = Coords(p1);
	crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner2_case09 */

LOCAL double area_twister_case10(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
	double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
	double A1[3], A2[3], A3[3], A4[3], A5[3], A6[3], A7[3], A8[3], 
	      A9[3], A10[3], A11[3], A12[3], A13[3], A14[3], A15[3], 
	      A16[3], A17[3], A18[3], A19[3], A20[3], A21[3], A22[3], 
	      A23[3], A24[3], A25[3], A26[3], A27[3], A28[3], A29[3], 
	      A30[3], A31[3], A32[3], A33[3], A34[3], A35[3], A36[3], 
	      A37[3], A38[3], A39[3], A40[3], A41[3], A42[3], A43[3], 
	      A44[3], A45[3], A46[3], A47[3], A48[3], A49[3], A50[3], 
	      A51[3], A52[3], A53[3], A54[3], A55[3], A56[3];
	double area;
	
	p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	p5 = crx_in_kdir(blk_crx,0,1)->p; 
	p6 = crx_in_jdir(blk_crx,1,0)->p; 
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	crx6 = Coords(p6);	

	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];      E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
   	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	}
	
	/* group A */ 
	Cross3d(E17,E16,A1);    Cross3d(E16,E18,A2);    
	Cross3d(E18,E20,A3);    Cross3d(E20,E19,A4);
        
	Cross3d(E4,E11,A5);     Cross3d(E11,E8,A6);     
	Cross3d(E8,E13,A7);     Cross3d(E13,E30,A8);
        
	Cross3d(E14,E9,A9);     Cross3d(E9,E12,A10);    
	Cross3d(E12,E5,A11);    Cross3d(E5,E15,A12);
        
	Cross3d(E29,E28,A13);   Cross3d(E28,E3,A14);    
	Cross3d(E3,E10,A15);    Cross3d(E10,E7,A16);
        
	Cross3d(E22,E24,A17);   Cross3d(E24,E23,A18);   
	Cross3d(E23,E1,A19);    Cross3d(E1,E21,A20);
        
	Cross3d(E6,E25,A21);    Cross3d(E25,E27,A22);   
	Cross3d(E27,E26,A23);   Cross3d(E26,E2,A24);

	/* group B */
	Cross3d(E7,E10,A25);    Cross3d(E10,E3,A26);    
	Cross3d(E18,E20,A27);   Cross3d(E20,E19,A28);
        
	Cross3d(E6,E25,A29);    Cross3d(E25,E27,A30);   
	Cross3d(E12,E5,A31);    Cross3d(E5,E15,A32);
        
	Cross3d(E22,E24,A33);   Cross3d(E24,E23,A34);   
	Cross3d(E8,E11,A35);    Cross3d(E11,E4,A36);
        
	Cross3d(E21,E1,A37);    Cross3d(E1,E23,A38);    
	Cross3d(E8,E13,A39);    Cross3d(E13,E30,A40);
        
	Cross3d(E17,E16,A41);   Cross3d(E16,E18,A42);   
	Cross3d(E3,E28,A43);    Cross3d(E28,E29,A44);
        
	Cross3d(E2,E26,A45);    Cross3d(E26,E27,A46);   
	Cross3d(E12,E9,A47);    Cross3d(E9,E14,A48);

	/* group C */        
	Cross3d(E7,E10,A49);    Cross3d(E10,E28,A50);   
	Cross3d(E28,E29,A51);   Cross3d(E2,E26,A52);
        
	Cross3d(E22,E24,A53);   Cross3d(E24,E1,A54);    
	Cross3d(E1,E21,A55);    Cross3d(E30,E4,A56);

	area = ( Mag3d(A1)+Mag3d(A2)+Mag3d(A3)+Mag3d(A4)+Mag3d(A5)
			     +Mag3d(A6)+Mag3d(A7)+Mag3d(A8)+Mag3d(A9)
			     +Mag3d(A10)+Mag3d(A11)+Mag3d(A12)+Mag3d(A13)
			     +Mag3d(A14)+Mag3d(A15)+Mag3d(A16)+Mag3d(A17)
			     +Mag3d(A18)+Mag3d(A19)+Mag3d(A20)+Mag3d(A21)
			     +Mag3d(A22)+Mag3d(A23)+Mag3d(A24)+Mag3d(A25)
			     +Mag3d(A26)+Mag3d(A27)+Mag3d(A28)+Mag3d(A29)
			     +Mag3d(A30)+Mag3d(A31)+Mag3d(A32)+Mag3d(A33)
			     +Mag3d(A34)+Mag3d(A35)+Mag3d(A36)+Mag3d(A37)
			     +Mag3d(A38)+Mag3d(A39)+Mag3d(A40)+Mag3d(A41)
			     +Mag3d(A42)+Mag3d(A43)+Mag3d(A44)+Mag3d(A45)
			     +Mag3d(A46)+Mag3d(A47)+Mag3d(A48)+Mag3d(A49)
			     +Mag3d(A50)+Mag3d(A51)+Mag3d(A52)+Mag3d(A53)
			     +Mag3d(A54)+Mag3d(A55)+Mag3d(A56) )/28.0;
	return area;
}	/* end area_twister_case10 */

LOCAL double area_corner1_case12(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;

	p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner1_case12 */

LOCAL double area_corner2_case12(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner2_case12 */

LOCAL double area_corner3_case12(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_jdir(blk_crx,1,1)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
        p3 = crx_in_kdir(blk_crx,1,0)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner3_case12 */

LOCAL double area_corner_case13(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner_case013 */

LOCAL double area_corner1_case14(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner1_case14 */

LOCAL double area_corner2_case14(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_kdir(blk_crx,1,1)->p;
        p2 = crx_in_idir(blk_crx,1,1)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner2_case014 */

LOCAL double area_corner3_case14(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_jdir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p;

	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner3_case14 */

LOCAL double area_corner4_case14(
	BLK_CRX *blk_crx)
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3;
	double A[3], D1[3], D2[3];
	double area;
	
	p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,0,0)->p;
        p3 = crx_in_idir(blk_crx,0,1)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx2[i] - crx1[i];
	    D2[i] = crx3[i] - crx1[i];
	}

	Cross3d(D1,D2,A);
	
	area = Mag3d(A)/2.0;
	return area;
}	/* end area_corner4_case14 */


LOCAL double volume_corner_case03(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;
	
        p1 = crx_in_idir(blk_crx,1,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,1)->p; 
        p3 = crx_in_kdir(blk_crx,1,1)->p; 

	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c8 */
	}
	
	volume = fabs(Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner_case03 */

LOCAL double volume_edge_case02(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3],
	      D9[3], D10[3];
	double V1, V2, V3, V4, V5, V6;
	double volume;
	
	p1 = crx_in_idir(blk_crx,1,1)->p;  
        p2 = crx_in_jdir(blk_crx,1,1)->p; 
        p3 = crx_in_idir(blk_crx,1,0)->p;
	p4 = crx_in_jdir(blk_crx,0,1)->p;

	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4); 
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
            D2[i] = crx1[i] - crx3[i];
 	    D3[i] = crx2[i] - crx3[i];
	    D4[i] = crx2[i] - crx4[i];
	    D5[i] = crx3[i] - crx4[i];
	    D6[i] = crx2[i] - crn2[i];	/* crx2 - c7  */
	    D7[i] = crx3[i] - crn1[i];  /* crx3 - c8  */
	    D8[i] = crx4[i] - crn1[i];  /* crx4 - c8  */
	    D9[i] = crx4[i] - crn2[i];	/* crx4 - c7  */
	    D10[i] = crn2[i] - crn1[i]; /* c7 -c8    */
	}
	/* triangulation given by diagonal crx1-crx4  */
	V1 = fabs(Det3d(D1,D4,D9));  	/* tetrahedron crx1-crx2-crx4-c7 */
	V2 = fabs(Det3d(D2,D5,D9));	/* tetrahedron crx1-crx3-crx4-c7  */
	V3 = fabs(Det3d(D1,D6,D10));	/* tetrahedron crx1-crx2-c7-c8    */
	
	/* triangulation given by diagonal crx2-crx3  */
	V4 = fabs(Det3d(D1,D3,D7)); 	/* tetrahedron crx1-crx2-crx3-c8 */
	V5 = fabs(Det3d(D3,D5,D8));	/* tetrahedron crx2-crx3-crx4-c8  */
	V6 = fabs(Det3d(D5,D9,D10));	/* tetrahedron crx3-crx4-c7-c8    */

	volume = ( V1+V2+V3+V4+V5+V6 )/12.0;
	return volume;
}	/* end volume_edge_case02 */

LOCAL double volume_edge1_case11(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3],
	      D9[3], D10[3];
	double V1, V2, V3, V4, V5, V6;
	double volume;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_jdir(blk_crx,0,0)->p; 
        p3 = crx_in_jdir(blk_crx,1,0)->p; 
	p4 = crx_in_idir(blk_crx,1,1)->p;

	/* same as volume_edge_case02() but with crx3 <-> crx4, c8 -> c3, */
	/* and c7 -> c4 */
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p4);
	crx4 = Coords(p3); 
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
            D2[i] = crx1[i] - crx3[i];
 	    D3[i] = crx2[i] - crx3[i];
	    D4[i] = crx2[i] - crx4[i];
	    D5[i] = crx3[i] - crx4[i];
	    D6[i] = crx2[i] - crn2[i];	/* crx2 - c4  */
	    D7[i] = crx3[i] - crn1[i];  /* crx4 - c3  */
	    D8[i] = crx4[i] - crn1[i];  /* crx3 - c3  */
	    D9[i] = crx4[i] - crn2[i];	/* crx3 - c4  */
	    D10[i] = crn2[i] - crn1[i]; /* c4 - c3  */
	}
	/* triangulation given by diagonal crx1-crx3  */
	V1 = fabs(Det3d(D1,D4,D9));  	/* tetrahedron crx1-crx2-crx3-c4  */
	V2 = fabs(Det3d(D2,D5,D9));	/* tetrahedron crx1-crx3-crx4-c4  */
	V3 = fabs(Det3d(D1,D6,D10));	/* tetrahedron crx1-crx2-c4-c3    */
	
	/* triangulation given by diagonal crx2-crx4  */
	V4 = fabs(Det3d(D1,D3,D7)); 	/* tetrahedron crx1-crx2-crx4-c3  */
	V5 = fabs(Det3d(D3,D5,D8));	/* tetrahedron crx2-crx3-crx4-c3  */
	V6 = fabs(Det3d(D5,D9,D10));	/* tetrahedron crx3-crx4-c4-c3   */
	
	volume = ( V1+V2+V3+V4+V5+V6 )/12.0;
	return volume;
}	/* end volume_edge1_case11 */

LOCAL double volume_edge2_case11(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3],
	      D9[3], D10[3];
	double V1, V2, V3, V4, V5, V6;
	double volume;
	
        p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_idir(blk_crx,0,1)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 

	/* same as volume_edge_case02() but with crx1 -> crx4, crx3 -> crx1, */
	/* crx4 -> crx3, c8 -> c6, and c7 -> c5 */
	
	crx1 = Coords(p4);
        crx2 = Coords(p2);
        crx3 = Coords(p1);
	crx4 = Coords(p3); 
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
            D2[i] = crx1[i] - crx3[i];
 	    D3[i] = crx2[i] - crx3[i];
	    D4[i] = crx2[i] - crx4[i];
	    D5[i] = crx3[i] - crx4[i];
	    D6[i] = crx2[i] - crn2[i];	/* crx2 - c5  */
	    D7[i] = crx3[i] - crn1[i];  /* crx1 - c6  */
	    D8[i] = crx4[i] - crn1[i];  /* crx3 - c6  */
	    D9[i] = crx4[i] - crn2[i];	/* crx3 - c5  */
	    D10[i] = crn2[i] - crn1[i]; /* c5 - c6   */
	}
	/* triangulation given by diagonal crx3-crx4  */
	V1 = fabs(Det3d(D1,D4,D9));  	/* tetrahedron crx2-crx3-crx4-c5  */
	V2 = fabs(Det3d(D2,D5,D9));	/* tetrahedron crx1-crx3-crx4-c5  */
	V3 = fabs(Det3d(D1,D6,D10));	/* tetrahedron crx2-crx4-c5-c6    */
	
	/* triangulation given by diagonal crx1-crx2  */
	V4 = fabs(Det3d(D1,D3,D7)); 	/* tetrahedron crx1-crx2-crx4-c6  */
	V5 = fabs(Det3d(D3,D5,D8));	/* tetrahedron crx1-crx2-crx3-c6  */
	V6 = fabs(Det3d(D5,D9,D10));	/* tetrahedron crx1-crx3-c5-c6    */
	
	volume = ( V1+V2+V3+V4+V5+V6 )/12.0;
	return volume;
}	/* end volume_edge2_case11 */

LOCAL double volume_edge1_p2_case11(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3],
	      D9[3], D10[3];
	double V1, V2, V3, V4, V5, V6;
	double volume;
	
        p1 = crx_in_jdir(blk_crx,0,0)->p; 
        p2 = crx_in_idir(blk_crx,0,0)->p; 
	p3 = crx_in_idir(blk_crx,0,1)->p;
        p4 = crx_in_jdir(blk_crx,1,0)->p; 

	/* same as volume_edge1_case11() but with c3 -> c1 and c4 -> c2 */
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p4);
	crx4 = Coords(p3); 
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
            D2[i] = crx1[i] - crx3[i];
 	    D3[i] = crx2[i] - crx3[i];
	    D4[i] = crx2[i] - crx4[i];
	    D5[i] = crx3[i] - crx4[i];
	    D6[i] = crx2[i] - crn2[i];	/* crx2 - c2  */
	    D7[i] = crx3[i] - crn1[i];  /* crx4 - c1  */
	    D8[i] = crx4[i] - crn1[i];  /* crx3 - c1  */
	    D9[i] = crx4[i] - crn2[i];	/* crx3 - c2  */
	    D10[i] = crn2[i] - crn1[i]; /* c2 - c1  */
	}
	/* triangulation given by diagonal crx1-crx3  */
	V1 = fabs(Det3d(D1,D4,D9));  	/* tetrahedron crx1-crx2-crx3-c2  */
	V2 = fabs(Det3d(D2,D5,D9));	/* tetrahedron crx1-crx3-crx4-c2  */
	V3 = fabs(Det3d(D1,D6,D10));	/* tetrahedron crx1-crx2-c1-c2    */
	
	/* triangulation given by diagonal crx2-crx4  */
	V4 = fabs(Det3d(D1,D3,D7)); 	/* tetrahedron crx1-crx2-crx4-c1  */
	V5 = fabs(Det3d(D3,D5,D8));	/* tetrahedron crx2-crx3-crx4-c1  */
	V6 = fabs(Det3d(D5,D9,D10));	/* tetrahedron crx3-crx4-c1-c2   */
	
	volume = ( V1+V2+V3+V4+V5+V6 )/12.0;
	return volume;
}	/* end volume_edge1_p2_case11 */

LOCAL double volume_plane_case01(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[],
	double crn4[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3],
	      D9[3], D10[3], D11[3], D12[3], D13[3], D14[3], D15[3],
	      D16[3], D17[3], D18[3], D19[3]; 
	double A1[3], A2[3], A3[3], A4[3];
	double V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12;
	double volume;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_idir(blk_crx,1,1)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_idir(blk_crx,0,1)->p;
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4);
	
	for (i = 0; i < 3; i++)
        {       
	    D1[i]=crx1[i]-crx2[i];
	    D2[i]=crx1[i]-crx3[i];
	    D3[i]=crx1[i]-crx4[i];
	    D4[i]=crx2[i]-crx3[i];
	    D5[i]=crx2[i]-crx4[i];
	    D6[i]=crx3[i]-crx4[i];
	    D7[i]=crx1[i]-crn3[i];	/* crx1 - c5   */
	    D8[i]=crx1[i]-crn4[i];	/* crx1 - c6  */
	    D9[i]=crx2[i]-crn3[i];	/* crx2 - c5  */
	    D10[i]=crx3[i]-crn3[i];	/* crx3 - c5  */
	    D11[i]=crx4[i]-crn3[i];	/* crx4 - c5  */
	    D12[i]=crx4[i]-crn4[i];	/* crx4 - c6  */
	    D13[i]=crx4[i]-crn2[i];	/* crx4 - c8   */
	    D14[i]=crn3[i]-crn4[i];	/* c5 - c6   */
	    D15[i]=crn3[i]-crn1[i];	/* c5 - c7   */
	    D16[i]=crn3[i]-crn2[i];	/* c5 - c8   */
	    D17[i]=crn4[i]-crn1[i];	/* c6 - c7   */
	    D18[i]=crn4[i]-crn2[i];	/* c6 - c8   */
	    D19[i]=crn1[i]-crn2[i];	/* c7 - c8   */
	}
	/* triangulation given by diagonal crx1-crx4   */
	V1 = fabs(Det3d(D1,D5,D13));	/* tetrahedron crx1-crx2-crx4-c8  */
	V2 = fabs(Det3d(D3,D12,D18));	/* tetrahedron crx1-crx4-c6-c8     */
	V3 = fabs(Det3d(D8,D17,D19));	/* tetrahedron crx1-c6-c7-c8      */
	V4 = fabs(Det3d(D7,D14,D17));	/* tetrahedron crx1-c5-c6-c7      */
	V5 = fabs(Det3d(D13,D11,D14));	/* tetrahedron crx1-crx4-c5-c6    */
	V6 = fabs(Det3d(D2,D6,D11));	/* tetrahedron crx1-crx3-crx4-c5  */
	
	/* triangulation given by diagonal crx2-crx3  */
	V7 = fabs(Det3d(D1,D4,D10));	/* tetrahedron crx1-crx2-crx3-c5  */
	V8 = fabs(Det3d(D4,D6,D11));	/* tetrahedron crx2-crx3-crx4-c5  */
	V9 = fabs(Det3d(D5,D11,D14));	/* tetrahedron crx2-crx4-c5-c6    */
	V10 = fabs(Det3d(D9,D14,D18));	/* tetrahedron crx2-c5-c6-c8      */
	V11 = fabs(Det3d(D1,D9,D16));	/* tetrahedron crx1-crx2-c5-c8    */
	V12 = fabs(Det3d(D7,D15,D19));	/* tetrahedron crx1-c5-c7-c8      */
	
	volume = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12 )/12.0; 
	return volume;
}	/* end volume_plane_case01 */

LOCAL double volume_glider_case04(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5;
	double *crx1, *crx2, *crx3, *crx4, *crx5;
	double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3]; 
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3];  
	double V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13, V14, 
	      V15, V16, V17, V18, V19, V20, V21, V22, V23, V24, V25, V26, V27;
	double volume;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_idir(blk_crx,1,1)->p; 
        p3 = crx_in_jdir(blk_crx,0,1)->p; 
	p4 = crx_in_kdir(blk_crx,1,0)->p; 
	p5 = crx_in_idir(blk_crx,0,1)->p; 
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	
	    E2[i]=crx1[i]-crx3[i]; 
	    E3[i]=crx1[i]-crx4[i];
	    E4[i]=crx1[i]-crx5[i];
	    E5[i]=crx2[i]-crx3[i]; 
	    E6[i]=crx2[i]-crx4[i]; 
	    E7[i]=crx2[i]-crx5[i]; 
	    E8[i]=crx3[i]-crx4[i]; 
	    E9[i]=crx3[i]-crx5[i]; 
	    E10[i]=crx4[i]-crx5[i]; 	
	}
	
	/* triangulation about crx 1  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx2 - c6   */
	    D2[i]=crx4[i]-crn3[i];	/* crx4 - c7   */
	    D3[i]=crx5[i]-crn1[i];	/* crx5 - c6   */
	    D4[i]=crx5[i]-crn3[i];	/* crx5 - c7   */
	    D5[i]=crn1[i]-crn3[i];	/* c6 - c7    */
	    D6[i]=crn3[i]-crn2[i];	/* c7 - c8    */
	}
	
	V1 = fabs(Det3d(E2,E8,D2));	/* tetrahedron crx1-crx3-crx4-c7  */
	V2 = fabs(Det3d(E3,E10,D4));	/* tetrahedron crx1-crx4-crx5-c7 */
	V3 = fabs(Det3d(E1,E7,D4));	/* tetrahedron crx1-crx2-crx5-c7  */
	V4 = fabs(Det3d(E10,D3,D5));	/* tetrahedron crx4-crx5-c6-c7    */
	V5 = fabs(Det3d(E7,D3,D5));	/* tetrahedron crx2-crx5-c6-c7  */
	V6 = fabs(Det3d(D1,D5,D6));	/* tetrahedron crx2-c6-c7-c8   */
	
	/*triangulation about crx 2  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx3[i]-crn1[i];	/* crx3 - c6  */
	    D2[i]=crx3[i]-crn3[i];	/* crx3 - c7  */
	    D3[i]=crx4[i]-crn1[i];	/* crx4 - c6  */
	    D4[i]=crx5[i]-crn1[i];	/* crx5 - c6   */
	    D5[i]=crn1[i]-crn2[i];	/* c6 - c8    */
	    D6[i]=crn3[i]-crn2[i];	/* c7 - c8   */
	}
	
	V7 = fabs(Det3d(E1,E5,D2));	/* tetrahedron crx1-crx2-crx3-c7  */
	V8 = fabs(Det3d(E5,E8,D3));	/* tetrahedron crx2-crx3-crx4-c6  */
	V9 = fabs(Det3d(E6,E10,D4));	/* tetrahedron crx2-crx4-crx5-c6  */
	V10 = fabs(Det3d(E5,D1,D5));	/* tetrahedron crx2-crx3-c6-c8    */
	V11 = fabs(Det3d(E5,D2,D6));	/* tetrahedron crx2-crx3-c7-c8   */
	
	/*triangulation about crx 3  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx3[i]-crn1[i];	/* crx3 - c6   */
	    D2[i]=crx3[i]-crn3[i];	/* crx3 - c7  */
	    D3[i]=crx5[i]-crn1[i];	/* crx5 - c6 */
	    D4[i]=crn1[i]-crn2[i];	/* c6 - c8    */
	    D5[i]=crn3[i]-crn2[i];	/* c7 - c8    */
	}
	
	V12 = fabs(Det3d(E8,E10,D3));	/* tetrahedron crx3-crx4-crx5-c6  */
	V13 = fabs(Det3d(E5,E9,D3));	/* tetrahedron crx2-crx3-crx5-c6 */
	V14 = fabs(Det3d(E1,E5,D2));	/* tetrahedron crx1-crx2-crx3-c7  */
	V15 = fabs(Det3d(E5,D2,D5));	/* tetrahedron crx2-crx3-c7-c8    */
	V16 = fabs(Det3d(E5,D1,D4));	/* tetrahedron crx2-crx3-c6-c8   */
	
	/*triangulation about crx 4  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx2 - c6   */
	    D2[i]=crx4[i]-crn1[i];	/* crx4 - c6   */
	    D3[i]=crx4[i]-crn3[i];	/* crx4 - c7   */
	    D4[i]=crx5[i]-crn1[i];	/* crx5 - c6   */
	    D5[i]=crn1[i]-crn3[i];	/* c6 - c7    */
	    D6[i]=crn3[i]-crn2[i];	/* c7 - c8    */
	}
	
	V17 = fabs(Det3d(E2,E8,D3));	/* tetrahedron crx1-crx3-crx4-c7  */
	V18 = fabs(Det3d(E1,E6,D3));	/* tetrahedron crx1-crx2-crx4-c7 */
	V19 = fabs(Det3d(E6,E10,D4));	/* tetrahedron crx2-crx4-crx5-c6  */
	V20 = fabs(Det3d(E6,D2,D5));	/* tetrahedron crx2-crx4-c6-c7    */
	V21 = fabs(Det3d(D1,D5,D6));	/* tetrahedron crx2-c6-c7-c8     */

	/*triangulation about crx 5  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx2 - c6   */
	    D2[i]=crx3[i]-crn1[i];	/* crx3 - c6   */
	    D3[i]=crx5[i]-crn1[i];	/* crx5 - c6   */
	    D4[i]=crn1[i]-crn3[i];	/* c6 - c7    */
	    D5[i]=crn3[i]-crn2[i];	/* c7 - c8    */
	}
	
	V22 = fabs(Det3d(E8,E10,D3));	/* tetrahedron crx3-crx4-crx5-c6  */
	V23 = fabs(Det3d(E2,E9,D3));	/* tetrahedron crx1-crx3-crx5-c6 */
	V24 = fabs(Det3d(E1,E7,D3));	/* tetrahedron crx1-crx2-crx5-c6  */
	V25 = fabs(Det3d(E1,D1,D4));	/* tetrahedron crx1-crx2-c6-c7    */
	V26 = fabs(Det3d(E2,D2,D4));	/* tetrahedron crx1-crx3-c6-c7   */
	V27 = fabs(Det3d(D1,D4,D5));	/* tetrahedron crx2-c6-c7-c8    */
	
	volume = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15
		     +V16+V17+V18+V19+V20+V21+V22+V23+V24+V25+V26+V27 )/30.0;
	return volume;
}	/* end volume_glider_case04 */

LOCAL double volume_glider_case13(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5;
	double *crx1, *crx2, *crx3, *crx4, *crx5;
	double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3];  
	double V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13, V14,
              V15, V16, V17, V18, V19, V20, V21, V22, V23, V24, V25, V26, V27;
	double volume;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,1)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	p5 = crx_in_idir(blk_crx,0,1)->p; 

	/* same as volume_glider_case04() but with c8 -> c5, c6 <-> c7,  */
	/* crx1 <-> crx5, crx2 -> crx3, crx4 -> crx2, and crx3 -> crx4 */

	crx1 = Coords(p5);
	crx2 = Coords(p3);
	crx3 = Coords(p4);
	crx4 = Coords(p2);
	crx5 = Coords(p1);
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	
	    E2[i]=crx1[i]-crx3[i]; 
	    E3[i]=crx1[i]-crx4[i]; 
	    E4[i]=crx1[i]-crx5[i]; 
	    E5[i]=crx2[i]-crx3[i];
	    E6[i]=crx2[i]-crx4[i];
	    E7[i]=crx2[i]-crx5[i];
	    E8[i]=crx3[i]-crx4[i]; 
	    E9[i]=crx3[i]-crx5[i]; 
	    E10[i]=crx4[i]-crx5[i]; 		
	}
	
	/* triangulation about crx 5  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx3 - c7   */
	    D2[i]=crx4[i]-crn3[i];	/* crx2 - c6   */
	    D3[i]=crx5[i]-crn1[i];	/* crx1 - c7   */
	    D4[i]=crx5[i]-crn3[i];	/* crx1 - c6   */
	    D5[i]=crn1[i]-crn3[i];	/* c7 - c6    */
	    D6[i]=crn3[i]-crn2[i];	/* c6 - c5    */
	}
	
	V1 = fabs(Det3d(E2,E8,D2));	/* tetrahedron crx2-crx4-crx5-c6  */
	V2 = fabs(Det3d(E3,E10,D4));	/* tetrahedron crx1-crx2-crx5-c6 */
	V3 = fabs(Det3d(E1,E7,D4));	/* tetrahedron crx1-crx3-crx5-c6  */
	V4 = fabs(Det3d(E10,D3,D5));	/* tetrahedron crx1-crx2-c6-c7    */
	V5 = fabs(Det3d(E7,D3,D5));	/* tetrahedron crx1-crx3-c6-c7  */
	V6 = fabs(Det3d(D1,D5,D6));	/* tetrahedron crx3-c5-c6-c7   */
	
	/*triangulation about crx 3  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx3[i]-crn1[i];	/* crx4 - c7  */
	    D2[i]=crx3[i]-crn3[i];	/* crx4 - c6  */
	    D3[i]=crx4[i]-crn1[i];	/* crx2 - c7  */
	    D4[i]=crx5[i]-crn1[i];	/* crx1 - c7   */
	    D5[i]=crn1[i]-crn2[i];	/* c7 - c5    */
	    D6[i]=crn3[i]-crn2[i];	/* c6 - c5   */
	}
	
	V7 = fabs(Det3d(E1,E5,D2));	/* tetrahedron crx3-crx4-crx5-c6  */
	V8 = fabs(Det3d(E5,E8,D3));	/* tetrahedron crx2-crx3-crx4-c7  */
	V9 = fabs(Det3d(E6,E10,D4));	/* tetrahedron crx1-crx2-crx3-c7  */
	V10 = fabs(Det3d(E5,D1,D5));	/* tetrahedron crx3-crx4-c5-c7    */
	V11 = fabs(Det3d(E5,D2,D6));	/* tetrahedron crx3-crx4-c5-c6   */
	
	/*triangulation about crx 4  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx3[i]-crn1[i];	/* crx4 - c7   */
	    D2[i]=crx3[i]-crn3[i];	/* crx4 - c6  */
	    D3[i]=crx5[i]-crn1[i];	/* crx1 - c7 */
	    D4[i]=crn1[i]-crn2[i];	/* c7 - c5    */
	    D5[i]=crn3[i]-crn2[i];	/* c6 - c5    */
	}
	
	V12 = fabs(Det3d(E8,E10,D3));	/* tetrahedron crx1-crx2-crx4-c7  */
	V13 = fabs(Det3d(E5,E9,D3));	/* tetrahedron crx1-crx3-crx4-c7 */
	V14 = fabs(Det3d(E1,E5,D2));	/* tetrahedron crx3-crx4-crx5-c6  */
	V15 = fabs(Det3d(E5,D2,D5));	/* tetrahedron crx3-crx4-c5-c6    */
	V16 = fabs(Det3d(E5,D1,D4));	/* tetrahedron crx3-crx4-c5-c7   */
	
	/*triangulation about crx 2  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx3 - c7   */
	    D2[i]=crx4[i]-crn1[i];	/* crx2 - c7   */
	    D3[i]=crx4[i]-crn3[i];	/* crx2 - c6   */
	    D4[i]=crx5[i]-crn1[i];	/* crx1 - c7   */
	    D5[i]=crn1[i]-crn3[i];	/* c7 - c6    */
	    D6[i]=crn3[i]-crn2[i];	/* c6 - c5    */
	}
	
	V17 = fabs(Det3d(E2,E8,D3));	/* tetrahedron crx2-crx4-crx5-c6  */
	V18 = fabs(Det3d(E1,E6,D3));	/* tetrahedron crx2-crx3-crx5-c6 */
	V19 = fabs(Det3d(E6,E10,D4));	/* tetrahedron crx1-crx2-crx3-c7  */
	V20 = fabs(Det3d(E6,D2,D5));	/* tetrahedron crx2-crx3-c6-c7    */
	V21 = fabs(Det3d(D1,D5,D6));	/* tetrahedron crx3-c5-c6-c7     */

	/*triangulation about crx 1  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx3 - c7   */
	    D2[i]=crx3[i]-crn1[i];	/* crx4 - c7   */
	    D3[i]=crx5[i]-crn1[i];	/* crx1 - c7   */
	    D4[i]=crn1[i]-crn3[i];	/* c7 - c6    */
	    D5[i]=crn3[i]-crn2[i];	/* c6 - c5    */
	}
	
	V22 = fabs(Det3d(E8,E10,D3));	/* tetrahedron crx1-crx2-crx4-c7  */
	V23 = fabs(Det3d(E2,E9,D3));	/* tetrahedron crx1-crx4-crx5-c7 */
	V24 = fabs(Det3d(E1,E7,D3));	/* tetrahedron crx1-crx3-crx5-c7  */
	V25 = fabs(Det3d(E1,D1,D4));	/* tetrahedron crx3-crx5-c6-c7    */
	V26 = fabs(Det3d(E2,D2,D4));	/* tetrahedron crx4-crx5-c6-c7   */
	V27 = fabs(Det3d(D1,D4,D5));	/* tetrahedron crx3-c5-c6-c7    */
	
	volume = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15
		     +V16+V17+V18+V19+V20+V21+V22+V23+V24+V25+V26+V27 )/30.0;
	return volume;
}	/* end volume_glider_case13 */

LOCAL double volume_glider_p2_case13(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5;
	double *crx1, *crx2, *crx3, *crx4, *crx5;
	double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3];  
	double V1, V2, V3, V4, V5, V6, V7, V8, V9, V10, V11, V12, V13, V14,
              V15, V16, V17, V18, V19, V20, V21, V22, V23, V24, V25, V26, V27;
	double volume;
	
        p1 = crx_in_idir(blk_crx,0,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_kdir(blk_crx,0,1)->p; 
	p5 = crx_in_idir(blk_crx,1,0)->p; 

	/* same as volume_glider_case13() but with c7 -> c2, c5 -> c1   */
	/* and c6 -> c3 */

	crx1 = Coords(p5);
	crx2 = Coords(p3);
	crx3 = Coords(p4);
	crx4 = Coords(p2);
	crx5 = Coords(p1);
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	
	    E2[i]=crx1[i]-crx3[i]; 
	    E3[i]=crx1[i]-crx4[i]; 
	    E4[i]=crx1[i]-crx5[i]; 
	    E5[i]=crx2[i]-crx3[i];
	    E6[i]=crx2[i]-crx4[i];
	    E7[i]=crx2[i]-crx5[i];
	    E8[i]=crx3[i]-crx4[i]; 
	    E9[i]=crx3[i]-crx5[i]; 
	    E10[i]=crx4[i]-crx5[i]; 		
	}
	
	/* triangulation about crx 5  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx3 - c2   */
	    D2[i]=crx4[i]-crn3[i];	/* crx2 - c3   */
	    D3[i]=crx5[i]-crn1[i];	/* crx1 - c2   */
	    D4[i]=crx5[i]-crn3[i];	/* crx1 - c3   */
	    D5[i]=crn1[i]-crn3[i];	/* c2 - c3    */
	    D6[i]=crn3[i]-crn2[i];	/* c3 - c1    */
	}
	
	V1 = fabs(Det3d(E2,E8,D2));	/* tetrahedron crx2-crx4-crx5-c3  */
	V2 = fabs(Det3d(E3,E10,D4));	/* tetrahedron crx1-crx2-crx5-c3 */
	V3 = fabs(Det3d(E1,E7,D4));	/* tetrahedron crx1-crx3-crx5-c3  */
	V4 = fabs(Det3d(E10,D3,D5));	/* tetrahedron crx1-crx2-c2-c3    */
	V5 = fabs(Det3d(E7,D3,D5));	/* tetrahedron crx1-crx3-c2-c3  */
	V6 = fabs(Det3d(D1,D5,D6));	/* tetrahedron crx3-c1-c2-c3   */
	
	/*triangulation about crx 3  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx3[i]-crn1[i];	/* crx4 - c2  */
	    D2[i]=crx3[i]-crn3[i];	/* crx4 - c3  */
	    D3[i]=crx4[i]-crn1[i];	/* crx2 - c2  */
	    D4[i]=crx5[i]-crn1[i];	/* crx1 - c2   */
	    D5[i]=crn1[i]-crn2[i];	/* c2 - c1    */
	    D6[i]=crn3[i]-crn2[i];	/* c3 - c1   */
	}
	
	V7 = fabs(Det3d(E1,E5,D2));	/* tetrahedron crx3-crx4-crx5-c3  */
	V8 = fabs(Det3d(E5,E8,D3));	/* tetrahedron crx2-crx3-crx4-c2  */
	V9 = fabs(Det3d(E6,E10,D4));	/* tetrahedron crx1-crx2-crx3-c2  */
	V10 = fabs(Det3d(E5,D1,D5));	/* tetrahedron crx3-crx4-c1-c2    */
	V11 = fabs(Det3d(E5,D2,D6));	/* tetrahedron crx3-crx4-c1-c3   */
	
	/*triangulation about crx 4  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx3[i]-crn1[i];	/* crx4 - c2   */
	    D2[i]=crx3[i]-crn3[i];	/* crx4 - c3  */
	    D3[i]=crx5[i]-crn1[i];	/* crx1 - c2 */
	    D4[i]=crn1[i]-crn2[i];	/* c2 - c1    */
	    D5[i]=crn3[i]-crn2[i];	/* c3 - c1    */
	}
	
	V12 = fabs(Det3d(E8,E10,D3));	/* tetrahedron crx1-crx2-crx4-c2  */
	V13 = fabs(Det3d(E5,E9,D3));	/* tetrahedron crx1-crx3-crx4-c2 */
	V14 = fabs(Det3d(E1,E5,D2));	/* tetrahedron crx3-crx4-crx5-c3  */
	V15 = fabs(Det3d(E5,D2,D5));	/* tetrahedron crx3-crx4-c1-c3    */
	V16 = fabs(Det3d(E5,D1,D4));	/* tetrahedron crx3-crx4-c1-c2   */
	
	/*triangulation about crx 2  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx3 - c2   */
	    D2[i]=crx4[i]-crn1[i];	/* crx2 - c2   */
	    D3[i]=crx4[i]-crn3[i];	/* crx2 - c3   */
	    D4[i]=crx5[i]-crn1[i];	/* crx1 - c2   */
	    D5[i]=crn1[i]-crn3[i];	/* c2 - c3    */
	    D6[i]=crn3[i]-crn2[i];	/* c3 - c1    */
	}
	
	V17 = fabs(Det3d(E2,E8,D3));	/* tetrahedron crx2-crx4-crx5-c3  */
	V18 = fabs(Det3d(E1,E6,D3));	/* tetrahedron crx2-crx3-crx5-c3 */
	V19 = fabs(Det3d(E6,E10,D4));	/* tetrahedron crx1-crx2-crx3-c2  */
	V20 = fabs(Det3d(E6,D2,D5));	/* tetrahedron crx2-crx3-c2-c3    */
	V21 = fabs(Det3d(D1,D5,D6));	/* tetrahedron crx3-c1-c2-c3     */

	/*triangulation about crx 1  */
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-crn1[i];	/* crx3 - c2   */
	    D2[i]=crx3[i]-crn1[i];	/* crx4 - c2   */
	    D3[i]=crx5[i]-crn1[i];	/* crx1 - c2   */
	    D4[i]=crn1[i]-crn3[i];	/* c2 - c3    */
	    D5[i]=crn3[i]-crn2[i];	/* c3 - c1    */
	}
	
	V22 = fabs(Det3d(E8,E10,D3));	/* tetrahedron crx1-crx2-crx4-c2  */
	V23 = fabs(Det3d(E2,E9,D3));	/* tetrahedron crx1-crx4-crx5-c2 */
	V24 = fabs(Det3d(E1,E7,D3));	/* tetrahedron crx1-crx3-crx5-c2  */
	V25 = fabs(Det3d(E1,D1,D4));	/* tetrahedron crx3-crx5-c2-c3    */
	V26 = fabs(Det3d(E2,D2,D4));	/* tetrahedron crx4-crx5-c2-c3   */
	V27 = fabs(Det3d(D1,D4,D5));	/* tetrahedron crx3-c1-c2-c3    */
	
	volume = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15
		     +V16+V17+V18+V19+V20+V21+V22+V23+V24+V25+V26+V27 )/30.0;
	return volume;
}	/* end volume_glider_p2_case13 */

LOCAL double volume_hexagon_case05(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[],
	double crn4[],
	double *h)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
        double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
        double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3], 
	      D9[3], D10[3], D11[3], D12[3], D13[3], D14[3], D15[3], 
	      D16[3];
	double V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12,V13,V14,V15,
	      V16,V17,V18,V19,V20,V21,V22,V23,V24,V25,V26,V27,V28,
	      V29,V30,V31,V32,V33,V34,V35,V36,V37,V38,V39,V40,V41,
	      V42,V43,V44;
	double proj1[3], proj2[3];
	double vol_A, vol_B, vol_C, volume;
	
        p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	p4 = crx_in_kdir(blk_crx,0,1)->p;
	p5 = crx_in_idir(blk_crx,0,1)->p;
	p6 = crx_in_jdir(blk_crx,1,0)->p;
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	crx6 = Coords(p6);	
	
	/* projection point of crx5 in the j dir */
	proj1[0] = crx5[0];
	proj1[1] = crx5[1] + h[1];
	proj1[2] = crx5[2];
	
	/* projection point of crx2 in the j dir */
	proj2[0] = crx2[0];
	proj2[1] = crx2[1] + h[1];
	proj2[2] = crx2[2];
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];	E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	}
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-proj1[i];
	    D2[i]=crx3[i]-proj2[i];
	    D3[i]=crx4[i]-proj2[i];
	    D4[i]=crx4[i]-crn2[i];
	    D5[i]=crx5[i]-proj1[i];
	    D6[i]=crx5[i]-proj2[i];
	    D7[i]=crx6[i]-crn3[i];
	    D8[i]=crx6[i]-proj1[i];
	    D9[i]=proj1[i]-proj2[i];
	    D10[i]=proj1[i]-crn3[i];
	    D11[i]=proj2[i]-crn2[i];
	    D12[i]=proj1[i]-crn4[i];
	    D13[i]=proj2[i]-crn4[i];
	    D14[i]=crn1[i]-crn4[i];
	    D15[i]=proj2[i]-crn4[i];
	    D16[i]=crn2[i]-crn4[i];
	}
				
	/* Symmetry Group A  */
	/* In the 6 triangulations of this group, the 3 diagonals have a common */
	/* vertex: crx1, crx2, ..., crx6  */
	
	V1 = fabs(Det3d(E9,E30,D1));	V2 = fabs(Det3d(E9,D2,D3));		
        V3 = fabs(Det3d(E21,E9,D2));	V4 = fabs(Det3d(E12,D2,D4));		
        V5 = fabs(Det3d(E17,E5,D5));	V6 = fabs(Det3d(E18,E5,D5));		
	V7 = fabs(Det3d(E14,D5,D6));	V8 = fabs(Det3d(E14,E30,D7));
	V9 = fabs(Det3d(E13,D8,D6));	V10 = fabs(Det3d(E22,E8,D8));
	V11 = fabs(Det3d(E8,D8,D9));	V12 = fabs(Det3d(E8,D10,D3));
	V13 = fabs(Det3d(E10,E22,D11));	V14 = fabs(Det3d(E10,E18,D12));		
	V15 = fabs(Det3d(E21,D11,D9));  V16 = fabs(Det3d(E8,D10,D3));	  
	V17 = fabs(Det3d(E17,E1,D13));	V18 = fabs(Det3d(E1,E8,D10));
	V19 = fabs(Det3d(E4,D10,D4));	V20 = fabs(Det3d(E18,E5,D5));
	V21 = fabs(Det3d(E5,E30,D8));	V22 = fabs(Det3d(E14,D5,D6));
        V23 = fabs(Det3d(E8,D10,D3));	V24 = fabs(Det3d(E21,E8,D10));		
        V25 = fabs(Det3d(E12,E30,D10));	V26 = fabs(Det3d(E12,D2,D4));		
        V27 = fabs(Det3d(E10,E18,D12));	V28 = fabs(Det3d(E10,E14,D5));		
        V29 = fabs(Det3d(E14,D5,D6));	V30 = fabs(Det3d(E9,E30,D1));		
	V31 = fabs(Det3d(E9,D2,D3));	V32 = fabs(Det3d(E22,E9,D5));		
	V33 = fabs(Det3d(E14,D5,D6));	V34 = fabs(Det3d(E9,D2,D4));		
	V35 = fabs(Det3d(E18,E1,D11));	V36 = fabs(Det3d(E17,E1,D11));		
        V37 = fabs(Det3d(E21,D13,D4));	V38 = fabs(Det3d(E8,D10,D3));
	V39 = fabs(Det3d(E21,E8,D10));	V40 = fabs(Det3d(E17,E4,D10));
	V41 = fabs(Det3d(E4,D10,D4));	V42 = fabs(Det3d(E18,E4,D8));
	V43 = fabs(Det3d(E14,E30,D8));	V44 = fabs(Det3d(E14,D5,D6));
	
	vol_A = fabs(Det3d(D14,D15,D16))/2.0 + ( V1+V2+V3+V4+V5+V6+V7
			+V8+V9+V10+V11+V12+V13+V14+V15+V16+V17+V18+V19+V20+V21
			+V22+V23+V24+V25+V26+V27+V28+V29+V30+V31+V32+V33+V34
			+V35+V36+V37+V38+V39+V40+V41+V42+V43+V44 )/36.0;
	
	/* Symmetry Group B  */
	/* In the 6 triangulations of this group, the 3 diagonals form a "z" */
	/* inside the hexagon (6 possible rotations) */

	V1 = fabs(Det3d(E14,D5,D6));	V2 = fabs(Det3d(E18,E5,D5));            
        V3 = fabs(Det3d(E8,D10,D3));    V4 = fabs(Det3d(E21,E8,D10));           
        V5 = fabs(Det3d(E12,E30,D10));  V6 = fabs(Det3d(E5,D2,D4));            
        V7 = fabs(Det3d(E17,E5,D2));    V8 = fabs(Det3d(E14,D5,D6));             	
	V9 = fabs(Det3d(E18,E5,D5));    V10 = fabs(Det3d(E8,D10,D3));          
	V11 = fabs(Det3d(E21,E8,D10));  V12 = fabs(Det3d(E5,E30,D8));          
	V13 = fabs(Det3d(E4,D10,D4));   V14 = fabs(Det3d(E17,E4,D10));          
	V15 = fabs(Det3d(E14,E30,D7));  V16 = fabs(Det3d(E13,D8,D6));
	V17 = fabs(Det3d(E18,E4,D8));	V18 = fabs(Det3d(E4,D10,D4));
	V19 = fabs(Det3d(E1,E8,D10));	V20 = fabs(Det3d(E17,E1,D13));
	V21 = fabs(Det3d(E8,D10,D3));	V22 = fabs(Det3d(E9,E30,D1));          
        V23 = fabs(Det3d(E9,D2,D3));    V24 = fabs(Det3d(E21,E9,D2));           
        V25 = fabs(Det3d(E12,D2,D4));   V26 = fabs(Det3d(E10,E18,D12));           	
	V27 = fabs(Det3d(E10,E14,D5));  V28 = fabs(Det3d(E14,D5,D6));  
	V29 = fabs(Det3d(E14,E30,D7));  V30 = fabs(Det3d(E13,D8,D6));   
	V31 = fabs(Det3d(E22,E8,D8));   V32 = fabs(Det3d(E8,D8,D9));           
	V33 = fabs(Det3d(E8,D10,D3));   V34 = fabs(Det3d(E17,E1,D13));          
	V35 = fabs(Det3d(E1,D13,D4));   V36 = fabs(Det3d(E18,E1,D11));		
	V37 = fabs(Det3d(E9,E30,D1));	V38 = fabs(Det3d(E9,D2,D3));
	V39 = fabs(Det3d(E22,E9,D5));	V40 = fabs(Det3d(E14,D5,D6));
	V41 = fabs(Det3d(E9,D2,D4));	V42 = fabs(Det3d(E10,E18,D12));
	V43 = fabs(Det3d(E10,E22,D11));	V44 = fabs(Det3d(E21,D11,D9));
	
	vol_B = fabs(Det3d(D14,D15,D16))/2.0 + ( V1+V2+V3+V4+V5+V6+V7
			+V8+V9+V10+V11+V12+V13+V14+V15+V16+V17+V18+V19+V20+V21
			+V22+V23+V24+V25+V26+V27+V28+V29+V30+V31+V32+V33+V34
			+V35+V36+V37+V38+V39+V40+V41+V42+V43+V44 )/36.0;

	/* Symmetry Group C  */
	/* In the 2 triangulations of this group, the 3 diagonals form a */
	/* triangle inside the hexagon (only 2 possible rotations) */
	
	V1 = fabs(Det3d(E3,E14,D7));	/* tetrahedron crx1-crx4-crx6-c4  */
	V2 = fabs(Det3d(E5,D8,D10));	/* tetrahedron crx1-crx6-proj1-c4   */
	V3 = fabs(Det3d(E1,E9,D8));	/* tetrahedron crx1-crx2-crx6-proj1      */
	V4 = fabs(Det3d(E8,E15,D8));	/* tetrahedron crx2-crx5-crx6-proj1 */
	V5 = fabs(Det3d(E1,D1,D9));     /* tetrahedron crx1-crx2-proj1-proj2 */
	V6 = fabs(Det3d(E1,E6,D2));	/* tetrahedron crx1-crx2-crx3-proj2  */
	V7 = fabs(Det3d(E2,D2,D11)); 	/* tetrahedron crx1-crx3-proj2-c7              	 */
	V8 = fabs(Det3d(E14,D8,D10));	/* tetrahedron crx4-crx6-proj1-c4 */
	V9 = fabs(Det3d(E13,E15,D8));	/* tetrahedron crx4-crx5-crx6-proj1 */
	V10 = fabs(Det3d(E2,E10,D4));	/* tetrahedron crx1-crx3-crx4-c7 */
	V11 = fabs(Det3d(E10,D3,D11));	/* tetrahedron crx3-crx4-proj2-c7  */
	V12 = fabs(Det3d(E6,E11,D6));	/* tetrahedron crx2-crx3-crx5-proj2 */
	V13 = fabs(Det3d(E10,E13,D6));	/* tetrahedron crx3-crx4-crx5-proj2 */
	V14 = fabs(Det3d(E13,D5,D9));	/* tetrahedron crx4-crx5-proj1-proj2 */

	vol_C = fabs(Det3d(D12,D13,D14))/2.0 + ( V1+V2+V3+V4+V5+V6+V7
			+V8+V9+V10+V11+V12+V13+V14 )/12.0;
	
/*	volume = (vol_A + vol_B + vol_C)/3.0; */
	volume = vol_C;
	return volume;
}	/* end volume_hexagon_case05 */

LOCAL double volume_hexagon_p4_case14(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[],
	double crn4[],
	double *h)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
        double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
        double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3], 
	      D9[3], D10[3], D11[3], D12[3], D13[3], D14[3], D15[3], 
	      D16[3];
	double V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12,V13,V14;
	double proj1[3], proj2[3];
	double vol_C, volume;
	
        p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_kdir(blk_crx,0,0)->p;
        p3 = crx_in_idir(blk_crx,0,0)->p;
	p4 = crx_in_kdir(blk_crx,1,1)->p;
	p5 = crx_in_jdir(blk_crx,1,0)->p;
	p6 = crx_in_idir(blk_crx,1,1)->p;
	
	/* same as volume_hexagon_case05() but with c6 -> c2, c7 -> c5,  */
	/* c4 -> c8 and c8 -> c6 */
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	crx6 = Coords(p6);	
	
	/* projection point of crx5 in the i dir */
	proj1[0] = crx5[0] + h[0];
	proj1[1] = crx5[1];
	proj1[2] = crx5[2];
	
	/* projection point of crx2 in the i dir */
	proj2[0] = crx2[0] + h[0];
	proj2[1] = crx2[1];
	proj2[2] = crx2[2];
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];	E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	}
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-proj1[i];
	    D2[i]=crx3[i]-proj2[i];
	    D3[i]=crx4[i]-proj2[i];
	    D4[i]=crx4[i]-crn2[i];
	    D5[i]=crx5[i]-proj1[i];
	    D6[i]=crx5[i]-proj2[i];
	    D7[i]=crx6[i]-crn3[i];
	    D8[i]=crx6[i]-proj1[i];
	    D9[i]=proj1[i]-proj2[i];
	    D10[i]=proj1[i]-crn3[i];
	    D11[i]=proj2[i]-crn2[i];
	    D12[i]=proj1[i]-crn4[i];
	    D13[i]=proj2[i]-crn4[i];
	    D14[i]=crn1[i]-crn4[i];
	    D15[i]=proj2[i]-crn4[i];
	    D16[i]=crn2[i]-crn4[i];
	}
				
	/* Symmetry Group C  */
	/* In the 2 triangulations of this group, the 3 diagonals form a */
	/* triangle inside the hexagon (only 2 possible rotations) */
	
	V1 = fabs(Det3d(E3,E14,D7));	/* tetrahedron crx1-crx4-crx6-c8  */
	V2 = fabs(Det3d(E5,D8,D10));	/* tetrahedron crx1-crx6-proj1-c8   */
	V3 = fabs(Det3d(E1,E9,D8));	/* tetrahedron crx1-crx2-crx6-proj1      */
	V4 = fabs(Det3d(E8,E15,D8));	/* tetrahedron crx2-crx5-crx6-proj1 */
	V5 = fabs(Det3d(E1,D1,D9));     /* tetrahedron crx1-crx2-proj1-proj2 */
	V6 = fabs(Det3d(E1,E6,D2));	/* tetrahedron crx1-crx2-crx3-proj2  */
	V7 = fabs(Det3d(E2,D2,D11)); 	/* tetrahedron crx1-crx3-proj2-c5              	 */
	V8 = fabs(Det3d(E14,D8,D10));	/* tetrahedron crx4-crx6-proj1-c8 */
	V9 = fabs(Det3d(E13,E15,D8));	/* tetrahedron crx4-crx5-crx6-proj1 */
	V10 = fabs(Det3d(E2,E10,D4));	/* tetrahedron crx1-crx3-crx4-c5 */
	V11 = fabs(Det3d(E10,D3,D11));	/* tetrahedron crx3-crx4-proj2-c5  */
	V12 = fabs(Det3d(E6,E11,D6));	/* tetrahedron crx2-crx3-crx5-proj2 */
	V13 = fabs(Det3d(E10,E13,D6));	/* tetrahedron crx3-crx4-crx5-proj2 */
	V14 = fabs(Det3d(E13,D5,D9));	/* tetrahedron crx4-crx5-proj1-proj2 */

	vol_C = fabs(Det3d(D12,D13,D14))/2.0 + ( V1+V2+V3+V4+V5+V6+V7
			+V8+V9+V10+V11+V12+V13+V14 )/12.0;
	
	volume = vol_C;
	return volume;
}	/* end volume_hexagon_p4_case14 */

LOCAL double volume_hexagon_p5_case14(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[],
	double crn4[],
	double *h)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
        double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
        double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3], 
	      D9[3], D10[3], D11[3], D12[3], D13[3], D14[3], D15[3], 
	      D16[3];
	double V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12,V13,V14;
	double proj1[3], proj2[3];
	double vol_C, volume;
	
        p1 = crx_in_jdir(blk_crx,1,1)->p; 
        p2 = crx_in_kdir(blk_crx,0,1)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
	p4 = crx_in_kdir(blk_crx,1,0)->p;
	p5 = crx_in_jdir(blk_crx,0,0)->p;
	p6 = crx_in_idir(blk_crx,0,0)->p;
	
	/* same as volume_hexagon_case05() but with c6 -> c3, c7 -> c8,  */
	/* c4 -> c5 and c8 -> c7 */
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	crx6 = Coords(p6);	
	
	/* projection point of crx5 in the i dir */
	proj1[0] = crx5[0] + h[0];
	proj1[1] = crx5[1];
	proj1[2] = crx5[2];
	
	/* projection point of crx2 in the i dir */
	proj2[0] = crx2[0] + h[0];
	proj2[1] = crx2[1];
	proj2[2] = crx2[2];
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];	E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	}
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-proj1[i];
	    D2[i]=crx3[i]-proj2[i];
	    D3[i]=crx4[i]-proj2[i];
	    D4[i]=crx4[i]-crn2[i];
	    D5[i]=crx5[i]-proj1[i];
	    D6[i]=crx5[i]-proj2[i];
	    D7[i]=crx6[i]-crn3[i];
	    D8[i]=crx6[i]-proj1[i];
	    D9[i]=proj1[i]-proj2[i];
	    D10[i]=proj1[i]-crn3[i];
	    D11[i]=proj2[i]-crn2[i];
	    D12[i]=proj1[i]-crn4[i];
	    D13[i]=proj2[i]-crn4[i];
	    D14[i]=crn1[i]-crn4[i];
	    D15[i]=proj2[i]-crn4[i];
	    D16[i]=crn2[i]-crn4[i];
	}
				
	/* Symmetry Group C  */
	/* In the 2 triangulations of this group, the 3 diagonals form a */
	/* triangle inside the hexagon (only 2 possible rotations) */
	
	V1 = fabs(Det3d(E3,E14,D7));	/* tetrahedron crx1-crx4-crx6-c5  */
	V2 = fabs(Det3d(E5,D8,D10));	/* tetrahedron crx1-crx6-proj1-c5   */
	V3 = fabs(Det3d(E1,E9,D8));	/* tetrahedron crx1-crx2-crx6-proj1      */
	V4 = fabs(Det3d(E8,E15,D8));	/* tetrahedron crx2-crx5-crx6-proj1 */
	V5 = fabs(Det3d(E1,D1,D9));     /* tetrahedron crx1-crx2-proj1-proj2 */
	V6 = fabs(Det3d(E1,E6,D2));	/* tetrahedron crx1-crx2-crx3-proj2  */
	V7 = fabs(Det3d(E2,D2,D11)); 	/* tetrahedron crx1-crx3-proj2-c8              	 */
	V8 = fabs(Det3d(E14,D8,D10));	/* tetrahedron crx4-crx6-proj1-c5 */
	V9 = fabs(Det3d(E13,E15,D8));	/* tetrahedron crx4-crx5-crx6-proj1 */
	V10 = fabs(Det3d(E2,E10,D4));	/* tetrahedron crx1-crx3-crx4-c8 */
	V11 = fabs(Det3d(E10,D3,D11));	/* tetrahedron crx3-crx4-proj2-c8  */
	V12 = fabs(Det3d(E6,E11,D6));	/* tetrahedron crx2-crx3-crx5-proj2 */
	V13 = fabs(Det3d(E10,E13,D6));	/* tetrahedron crx3-crx4-crx5-proj2 */
	V14 = fabs(Det3d(E13,D5,D9));	/* tetrahedron crx4-crx5-proj1-proj2 */

	vol_C = fabs(Det3d(D12,D13,D14))/2.0 + ( V1+V2+V3+V4+V5+V6+V7
			+V8+V9+V10+V11+V12+V13+V14 )/12.0;
	
	volume = vol_C;
	return volume;
}	/* end volume_hexagon_p5_case14 */

LOCAL double volume_hexagon_p6_case14(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[],
	double crn4[],
	double *h)
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
        double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
        double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3], 
	      D9[3], D10[3], D11[3], D12[3], D13[3], D14[3], D15[3], 
	      D16[3];
	double V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12,V13,V14;
	double proj1[3], proj2[3];
	double vol_C, volume;
	
        p1 = crx_in_jdir(blk_crx,0,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,1)->p;
        p3 = crx_in_idir(blk_crx,1,0)->p;
	p4 = crx_in_kdir(blk_crx,0,0)->p;
	p5 = crx_in_jdir(blk_crx,1,1)->p;
	p6 = crx_in_idir(blk_crx,0,1)->p;
	
	/* same as volume_hexagon_case05() but with c6 -> c8, c7 -> c3,  */
	/* c4 -> c2 and c8 -> c4 */
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	crx6 = Coords(p6);	
	
	/* projection point of crx5 in the i dir */
	proj1[0] = crx5[0] - h[0];
	proj1[1] = crx5[1];
	proj1[2] = crx5[2];
	
	/* projection point of crx2 in the i dir */
	proj2[0] = crx2[0] - h[0];
	proj2[1] = crx2[1];
	proj2[2] = crx2[2];
	
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];	E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	}
	
	for (i=0; i < 3; i++)
	{
	    D1[i]=crx2[i]-proj1[i];
	    D2[i]=crx3[i]-proj2[i];
	    D3[i]=crx4[i]-proj2[i];
	    D4[i]=crx4[i]-crn2[i];
	    D5[i]=crx5[i]-proj1[i];
	    D6[i]=crx5[i]-proj2[i];
	    D7[i]=crx6[i]-crn3[i];
	    D8[i]=crx6[i]-proj1[i];
	    D9[i]=proj1[i]-proj2[i];
	    D10[i]=proj1[i]-crn3[i];
	    D11[i]=proj2[i]-crn2[i];
	    D12[i]=proj1[i]-crn4[i];
	    D13[i]=proj2[i]-crn4[i];
	    D14[i]=crn1[i]-crn4[i];
	    D15[i]=proj2[i]-crn4[i];
	    D16[i]=crn2[i]-crn4[i];
	}
				
	/* Symmetry Group C  */
	/* In the 2 triangulations of this group, the 3 diagonals form a */
	/* triangle inside the hexagon (only 2 possible rotations) */
	
	V1 = fabs(Det3d(E3,E14,D7));	/* tetrahedron crx1-crx4-crx6-c2  */
	V2 = fabs(Det3d(E5,D8,D10));	/* tetrahedron crx1-crx6-proj1-c2   */
	V3 = fabs(Det3d(E1,E9,D8));	/* tetrahedron crx1-crx2-crx6-proj1      */
	V4 = fabs(Det3d(E8,E15,D8));	/* tetrahedron crx2-crx5-crx6-proj1 */
	V5 = fabs(Det3d(E1,D1,D9));     /* tetrahedron crx1-crx2-proj1-proj2 */
	V6 = fabs(Det3d(E1,E6,D2));	/* tetrahedron crx1-crx2-crx3-proj2  */
	V7 = fabs(Det3d(E2,D2,D11)); 	/* tetrahedron crx1-crx3-proj2-c3              	 */
	V8 = fabs(Det3d(E14,D8,D10));	/* tetrahedron crx4-crx6-proj1-c2 */
	V9 = fabs(Det3d(E13,E15,D8));	/* tetrahedron crx4-crx5-crx6-proj1 */
	V10 = fabs(Det3d(E2,E10,D4));	/* tetrahedron crx1-crx3-crx4-c3 */
	V11 = fabs(Det3d(E10,D3,D11));	/* tetrahedron crx3-crx4-proj2-c3  */
	V12 = fabs(Det3d(E6,E11,D6));	/* tetrahedron crx2-crx3-crx5-proj2 */
	V13 = fabs(Det3d(E10,E13,D6));	/* tetrahedron crx3-crx4-crx5-proj2 */
	V14 = fabs(Det3d(E13,D5,D9));	/* tetrahedron crx4-crx5-proj1-proj2 */

	vol_C = fabs(Det3d(D12,D13,D14))/2.0 + ( V1+V2+V3+V4+V5+V6+V7
			+V8+V9+V10+V11+V12+V13+V14 )/12.0;
	
	volume = vol_C;
	return volume;
}	/* end volume_hexagon_p6_case14 */

LOCAL double volume_corner1_case06(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;

	p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
			 
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c7  */
	}
	
	volume = fabs(Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner1_case06 */

LOCAL double volume_corner2_case06(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;

	p1 = crx_in_jdir(blk_crx,1,1)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
	p3 = crx_in_kdir(blk_crx,1,0)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c6  */
	}
	
	volume = fabs(Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner2_case06 */

LOCAL double volume_twister_case07(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[],
	double crn4[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
        double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
        double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3], 
	      D9[3], D10[3], D11[3], D12[3], D13[3];
	double V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12,V13,V14,V15,V16,
	      V17,V18,V19,V20,V21,V22,V23,V24,V25,V26,V27,V28,V29,V30,
	      V31,V32,V33,V34,V35,V36,V37,V38,V39,V40,V41,V42,V43,V44,
	      V45;
	double vol_A, vol_B, vol_C, volume;
	
        p1 = crx_in_kdir(blk_crx,0,1)->p; 
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,1,1)->p;
	p4 = crx_in_idir(blk_crx,0,1)->p; 
	p5 = crx_in_jdir(blk_crx,0,1)->p; 
	p6 = crx_in_idir(blk_crx,0,0)->p;
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	crx6 = Coords(p6);	
 
	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];      E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
                        
            D1[i]=crx2[i]-crn3[i];
	    D2[i]=crx3[i]-crn3[i];
	    D3[i]=crx3[i]-crn4[i];
	    D4[i]=crx4[i]-crn3[i];
	    D5[i]=crx5[i]-crn2[i];
	    D6[i]=crx5[i]-crn3[i];
	    D7[i]=crx6[i]-crn1[i];
	    D8[i]=crx6[i]-crn2[i];
	    D9[i]=crx6[i]-crn3[i];
	    D10[i]=crn1[i]-crn2[i];
	    D11[i]=crn1[i]-crn3[i];
	    D12[i]=crn2[i]-crn3[i];
	    D13[i]=crn3[i]-crn4[i];
	}
				
	/* Symmetry Group A */
	/* In the 6 triangulations of this group, the 3 diagonals have a  */
	/* common vertex: crx1, crx2, ..., crx6 */
	
	V1 = fabs(Det3d(E13,E15,D1));	V2 = fabs(Det3d(E13,D2,D3));		
        V3 = fabs(Det3d(E13,E26,D4));	V4 = fabs(Det3d(E25,D4,D3));		
        V5 = fabs(Det3d(E25,E17,D5));	V6 = fabs(Det3d(E18,D5,D6));		
	V7 = fabs(Det3d(E22,E16,D7));	V8 = fabs(Det3d(E15,D1,D8));		
	V9 = fabs(Det3d(E15,E24,D9));	V10 = fabs(Det3d(E26,E21,D9));		
	V11 = fabs(Det3d(E21,D9,D10));	V12 = fabs(Det3d(E21,E16,D5));		
	V13 = fabs(Det3d(E16,D5,D6));	V14 = fabs(Det3d(E14,E24,D9));		
	V15 = fabs(Det3d(E15,D1,D8));	V16 = fabs(Det3d(E15,E20,D11));
	V17 = fabs(Det3d(E26,E17,D11)); V18 = fabs(Det3d(E17,D11,D10));
	V19 = fabs(Det3d(D11,D10,D6));  V20 = fabs(Det3d(E18,D11,D12));
	V21 = fabs(Det3d(E14,E20,D11)); V22 = fabs(Det3d(E22,E16,D7));	
	V23 = fabs(Det3d(E15,D1,D8));	V24 = fabs(Det3d(E15,E27,D4));		
	V25 = fabs(Det3d(E14,E27,D4));	V26 = fabs(Det3d(E25,E21,D9));		
        V27 = fabs(Det3d(E21,D9,D10));  V28 = fabs(Det3d(E21,E16,D5));		
        V29 = fabs(Det3d(E16,D5,D6));	V30 = fabs(Det3d(E13,E15,D1));		
	V31 = fabs(Det3d(E13,D2,D3));	V32 = fabs(Det3d(E13,E23,D9));		
	V33 = fabs(Det3d(E23,E16,D11));	V34 = fabs(Det3d(E26,E17,D11));		
	V35 = fabs(Det3d(E17,D11,D10));	V36 = fabs(Det3d(E16,D11,D12));		
	V37 = fabs(Det3d(D11,D10,D6));	V38 = fabs(Det3d(E15,D1,D8));
	V39 = fabs(Det3d(E15,E27,D4));	V40 = fabs(Det3d(E27,E17,D11));
	V41 = fabs(Det3d(E17,D11,D10)); V42 = fabs(Det3d(E24,E16,D11));
	V43 = fabs(Det3d(E14,E24,D9));  V44 = fabs(Det3d(D11,D10,D6));
	V45 = fabs(Det3d(E16,D11,D12));
	
	vol_A = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15
			    +V16+V17+V18+V19+V20+V21+V22+V23+V24+V25+V26+V27
			    +V28+V29+V30+V31+V32+V33+V34+V35+V36+V37+V38+V39
			    +V40+V41+V42+V43+V44+V45 )/36.0;
	
	/* Symmetry Group B */
	/* In the 6 triangulations of this group, the 3 diagonals form a "z" */
	/* inside the hexagon (6 possible rotations) */

	V1 = fabs(Det3d(E15,D1,D8));	V2 = fabs(Det3d(E15,E27,D4)); 
	V3 = fabs(Det3d(E14,E27,D4));   V4 = fabs(Det3d(E25,D4,D10));  
	V5 = fabs(Det3d(E25,E17,D5));   V6 = fabs(Det3d(E22,E16,D7));  
	V7 = fabs(Det3d(E18,D7,D13));   V8 = fabs(Det3d(E13,E15,D1));
	V9 = fabs(Det3d(E13,D2,D3));    V10 = fabs(Det3d(E13,E26,D4));
	V11 = fabs(Det3d(E25,E21,D9));  V12 = fabs(Det3d(E21,D9,D10));          
	V13 = fabs(Det3d(E21,E16,D5));  V14 = fabs(Det3d(E16,D5,D6));           
	V15 = fabs(Det3d(E15,D1,D8));   V16 = fabs(Det3d(E15,E20,D11));
	V17 = fabs(Det3d(E26,E17,D11)); V18 = fabs(Det3d(E17,D11,D10));
	V19 = fabs(Det3d(E24,E16,D11)); V20 = fabs(Det3d(E14,E24,D9));
	V21 = fabs(Det3d(E16,D11,D10)); V22 = fabs(Det3d(E16,D5,D6));
	V23 = fabs(Det3d(E15,D1,D8));   V24 = fabs(Det3d(E15,E27,D4));
	V25 = fabs(Det3d(E27,E17,D11)); V26 = fabs(Det3d(E14,E20,D11)); 
	V27 = fabs(Det3d(E17,D11,D10)); V28 = fabs(Det3d(D11,D10,D6));          
        V29 = fabs(Det3d(E18,D11,D12)); V30 = fabs(Det3d(E22,E16,D7));
	V31 = fabs(Det3d(E15,D1,D8));   V32 = fabs(Det3d(E15,E24,D9)); 
	V33 = fabs(Det3d(E14,E24,D9));  V34 = fabs(Det3d(E23,E16,D11));
	V35 = fabs(Det3d(E26,E17,D11)); V36 = fabs(Det3d(E17,D11,D10));
	V37 = fabs(Det3d(E16,D11,D10)); V38 = fabs(Det3d(E16,D5,D6));		
	V39 = fabs(Det3d(E13,E15,D1));  V40 = fabs(Det3d(E13,D2,D3));
	V41 = fabs(Det3d(E13,E23,D9));  V42 = fabs(Det3d(E26,E21,D9));
	V43 = fabs(Det3d(E21,D9,D10));  V44 = fabs(Det3d(E21,E16,D5));
	V45 = fabs(Det3d(E16,D5,D6));
	
	vol_B = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15
			    +V16+V17+V18+V19+V20+V21+V22+V23+V24+V25+V26+V27
			    +V28+V29+V30+V31+V32+V33+V34+V35+V36+V37+V38+V39
			    +V40+V41+V42+V43+V44+V45 )/36.0;

	/* Symmetry Group C */
	/* In the 2 triangulations of this group, the 3 diagonals form a        */
	/* triangle inside the hexagon (only 2 possible rotations) */

	V1 = fabs(Det3d(E1,E6,D3));	/* tetrahedron crx1-crx2-crx3-c4       */
	V2 = fabs(Det3d(E6,D2,D13));    /* tetrahedron crx2-crx3-c8-c4 */
	V3 = fabs(Det3d(E6,E12,D9));    /* tetrahedron crx2-crx3-crx6-c8    */
	V4 = fabs(Det3d(E7,E14,D9));    /* tetrahedron crx2-crx4-crx6-c8   */
	V5 = fabs(Det3d(E14,D8,D12));   /* tetrahedron crx4-crx6-c6-c8      */
	V6 = fabs(Det3d(D7,D10,D12));   /* tetrahedron crx6-c5-c6-c8      */
	V7 = fabs(Det3d(E15,D7,D11));   /* tetrahedron crx5-crx6-c5-c8       */
	V8 = fabs(Det3d(E11,E15,D9));   /* tetrahedron crx3-crx5-crx6-c8 */
	
	V9 = fabs(Det3d(E1,D1,D13));	/* tetrahedron crx1-crx2-c8-c4 */
	V10 = fabs(Det3d(E1,E7,D4));    /* tetrahedron crx1-crx2-crx4-c8 */
	V11 = fabs(Det3d(E2,E11,D6));	/* tetrahedron crx1-crx3-crx5-c8 */
	V12 = fabs(Det3d(E3,E13,D6));	/* tetrahedron crx1-crx4-crx5-c8 */
	V13 = fabs(Det3d(E15,D7,D10));	/* tetrahedron crx5-crx6-c5-c6 */
	V14 = fabs(Det3d(E13,E15,D8));	/* tetrahedron crx4-crx5-crx6-c6 */
	V15 = fabs(Det3d(E13,D5,D12));	/* tetrahedron crx4-crx5-c6-c8 */

	vol_C = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15 )/12.0;
	
/*	volume = (vol_A + vol_B + vol_C)/3.0;  */
	volume = vol_C;
	return volume;
}	/* end volume_twister_case07 */

LOCAL double volume_corner_case08(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;

	p1 = crx_in_jdir(blk_crx,0,1)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p;
			 
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c5 */
	}
	
	volume = (Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner_case08 */

LOCAL double volume_edge_case08(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4;
	double *crx1, *crx2, *crx3, *crx4;
	double A1[3], A2[3], A3[3], A4[3];
	double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3],
	      D9[3], D10[3];
	double V1, V2, V3, V4, V5, V6;
	double volume;
	
	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,1,1)->p;
	p4 = crx_in_jdir(blk_crx,1,1)->p;
				
	/* same as volume_edge_case02() but with c8 -> c4 and c7 -> c8  */
	
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	crx4 = Coords(p4); 
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
            D2[i] = crx1[i] - crx3[i];
 	    D3[i] = crx2[i] - crx3[i];
	    D4[i] = crx2[i] - crx4[i];
	    D5[i] = crx3[i] - crx4[i];
	    D6[i] = crx2[i] - crn2[i];	/* crx2 - c8  */
	    D7[i] = crx3[i] - crn1[i];  /* crx3 - c4  */
	    D8[i] = crx4[i] - crn1[i];  /* crx4 - c4 */
	    D9[i] = crx4[i] - crn2[i];	/* crx4 - c8  */
	    D10[i] = crn2[i] - crn1[i]; /* c8 - c4   */
	}
	/* triangulation given by diagonal crx1-crx4  */
	V1 = fabs(Det3d(D1,D4,D9));  	/* tetrahedron crx1-crx2-crx4-c8  */
	V2 = fabs(Det3d(D2,D5,D9));	/* tetrahedron crx1-crx3-crx4-c8  */
	V3 = fabs(Det3d(D1,D6,D10));	/* tetrahedron crx1-crx2-c4-c8    */
	
	/* triangulation given by diagonal crx2-crx3 */
	V4 = fabs(Det3d(D1,D3,D7)); 	/* tetrahedron crx1-crx2-crx3-c4  */
	V5 = fabs(Det3d(D3,D5,D8));	/* tetrahedron crx2-crx3-crx4-c4  */
	V6 = fabs(Det3d(D5,D9,D10));	/* tetrahedron crx3-crx4-c4-c8    */
	
	volume = ( V1+V2+V3+V4+V5+V6 )/12.0;
	return volume;
}	/* end volume_edge_case02 */

LOCAL double volume_corner2_case09(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;

	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
			 
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c4  */
	}
	
	volume = fabs(Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner2_case09 */

LOCAL double volume_twister_case10(
	BLK_CRX *blk_crx,
	double crn1[],
	double crn2[],
	double crn3[],
	double crn4[])
{
	int i;
	FT_POINT *p1, *p2, *p3, *p4, *p5, *p6;
	double *crx1, *crx2, *crx3, *crx4, *crx5, *crx6;
        double E1[3], E2[3], E3[3], E4[3], E5[3], E6[3], E7[3], E8[3], 
	      E9[3], E10[3], E11[3], E12[3], E13[3], E14[3], E15[3], 
	      E16[3], E17[3], E18[3], E19[3], E20[3], E21[3], E22[3], 
	      E23[3], E24[3], E25[3], E26[3], E27[3], E28[3], E29[3], 
	      E30[3];
        double D1[3], D2[3], D3[3], D4[3], D5[3], D6[3], D7[3], D8[3], 
	      D9[3], D10[3], D11[3], D12[3], D13[3];
	double V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12,V13,V14,V15,V16,
	      V17,V18,V19,V20,V21,V22,V23,V24,V25,V26,V27,V28,V29,V30,
	      V31,V32,V33,V34,V35,V36,V37,V38,V39,V40,V41,V42,V43,V44,
	      V45;
	double vol_A, vol_B, vol_C, volume;

	p1 = crx_in_idir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,1,0)->p; 
        p3 = crx_in_idir(blk_crx,0,0)->p; 
	p4 = crx_in_jdir(blk_crx,1,1)->p; 
	p5 = crx_in_kdir(blk_crx,0,1)->p; 
	p6 = crx_in_jdir(blk_crx,1,0)->p; 
	
	crx1 = Coords(p1);
	crx2 = Coords(p2);
	crx3 = Coords(p3);
	crx4 = Coords(p4);
	crx5 = Coords(p5);
	crx6 = Coords(p6);	

	for (i=0; i < 3; i++)
	{
	    E1[i]=crx1[i]-crx2[i];	E16[i]=-E1[i];
	    E2[i]=crx1[i]-crx3[i];      E17[i]=-E2[i];
	    E3[i]=crx1[i]-crx4[i];      E18[i]=-E3[i];
	    E4[i]=crx1[i]-crx5[i];      E19[i]=-E4[i];
	    E5[i]=crx1[i]-crx6[i];      E20[i]=-E5[i];
	    E6[i]=crx2[i]-crx3[i];      E21[i]=-E6[i];
	    E7[i]=crx2[i]-crx4[i];      E22[i]=-E7[i];
	    E8[i]=crx2[i]-crx5[i];      E23[i]=-E8[i];
	    E9[i]=crx2[i]-crx6[i];      E24[i]=-E9[i];
	    E10[i]=crx3[i]-crx4[i];     E25[i]=-E10[i];
	    E11[i]=crx3[i]-crx5[i];     E26[i]=-E11[i];
	    E12[i]=crx3[i]-crx6[i];     E27[i]=-E12[i];
	    E13[i]=crx4[i]-crx5[i];     E28[i]=-E13[i];
	    E14[i]=crx4[i]-crx6[i];     E29[i]=-E14[i];
	    E15[i]=crx5[i]-crx6[i];     E30[i]=-E15[i];
	    
	    D1[i]=crx2[i]-crn1[i];
	    D2[i]=crx3[i]-crn1[i];
	    D3[i]=crx4[i]-crn1[i];
	    D4[i]=crx5[i]-crn2[i];
            D5[i]=crx6[i]-crn2[i];
	    D6[i]=crx6[i]-crn3[i];
	    D7[i]=crn1[i]-crn2[i];
	    D8[i]=crn2[i]-crn3[i];
	    D9[i]=crn3[i]-crn4[i];
	    D10[i]=crn1[i]-crn2[i];
	    D11[i]=crx1[i]-crn2[i];
	    D12[i]=crn2[i]-crn4[i];
	    D13[i]=-D6[i];
	}
				
	/* Symmetry Group A */
	/* In the 6 triangulations of this group, the 3 diagonals have a  */
	/* common vertex: crx1, crx2, ..., crx6 */

	V1 = fabs(Det3d(E13,E15,D1));	V2 = fabs(Det3d(E13,D2,D3));		
        V3 = fabs(Det3d(E13,E26,D4));	V4 = fabs(Det3d(E25,D4,D3));		
        V5 = fabs(Det3d(E25,E17,D5));	V6 = fabs(Det3d(E18,D5,D6));		
	V7 = fabs(Det3d(E22,E16,D7));	V8 = fabs(Det3d(E15,D1,D8));		
	V9 = fabs(Det3d(E15,E24,D9));	V10 = fabs(Det3d(E26,E21,D9));		
	V11 = fabs(Det3d(E21,D9,D10));	V12 = fabs(Det3d(E21,E16,D5));		
	V13 = fabs(Det3d(E16,D5,D6));	V14 = fabs(Det3d(E14,E24,D9));		
	V15 = fabs(Det3d(E15,D1,D8));	V16 = fabs(Det3d(E15,E20,D11));
	V17 = fabs(Det3d(E26,E17,D11)); V18 = fabs(Det3d(E17,D11,D10));
	V19 = fabs(Det3d(D11,D10,D6));  V20 = fabs(Det3d(E18,D11,D12));
	V21 = fabs(Det3d(E14,E20,D11)); V22 = fabs(Det3d(E22,E16,D7));		
	V23 = fabs(Det3d(E15,D1,D8));	V24 = fabs(Det3d(E15,E27,D4));		
        V25 = fabs(Det3d(E14,E27,D4));  V26 = fabs(Det3d(E25,E21,D9));		
        V27 = fabs(Det3d(E21,D9,D10));	V28 = fabs(Det3d(E21,E16,D5));		
        V29 = fabs(Det3d(E16,D5,D6));   V30 = fabs(Det3d(E13,E15,D1));		
	V31 = fabs(Det3d(E13,D2,D3));	V32 = fabs(Det3d(E13,E23,D9));		
	V33 = fabs(Det3d(E23,E16,D11));	V34 = fabs(Det3d(E26,E17,D11));		
	V35 = fabs(Det3d(E17,D11,D10));	V36 = fabs(Det3d(E16,D11,D12));		
        V37 = fabs(Det3d(D11,D10,D6));	V38 = fabs(Det3d(E15,D1,D8));
	V39 = fabs(Det3d(E15,E27,D4));  V40 = fabs(Det3d(E27,E17,D11));
	V41 = fabs(Det3d(E17,D11,D10)); V42 = fabs(Det3d(E24,E16,D11));
	V43 = fabs(Det3d(E14,E24,D9));  V44 = fabs(Det3d(D11,D10,D6));
	V45 = fabs(Det3d(E16,D11,D12));
	
	vol_A = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15
			    +V16+V17+V18+V19+V20+V21+V22+V23+V24+V25+V26+V27
			    +V28+V29+V30+V31+V32+V33+V34+V35+V36+V37+V38+V39
			    +V40+V41+V42+V43+V44+V45 )/36.0;
						
	/* Symmetry Group B */
	/* In the 6 triangulations of this group, the 3 diagonals form a "z" */
	/* inside the hexagon (6 possible rotations) */

	V1 = fabs(Det3d(E15,D1,D8));	V2 = fabs(Det3d(E15,E27,D4));           
        V3 = fabs(Det3d(E14,E27,D4));   V4 = fabs(Det3d(E25,D4,D10));           
        V5 = fabs(Det3d(E25,E17,D5));   V6 = fabs(Det3d(E22,E16,D7));           
        V7 = fabs(Det3d(E18,D7,D13));   V8 = fabs(Det3d(E13,E15,D1));           
	V9 = fabs(Det3d(E13,D2,D3));    V10 = fabs(Det3d(E13,E26,D4));          
	V11 = fabs(Det3d(E25,E21,D9));  V12 = fabs(Det3d(E21,D9,D10));          
	V13 = fabs(Det3d(E21,E16,D5));  V14 = fabs(Det3d(E16,D5,D6));           
	V15 = fabs(Det3d(E15,D1,D8));   V16 = fabs(Det3d(E15,E20,D11));
	V17 = fabs(Det3d(E26,E17,D11)); V18 = fabs(Det3d(E17,D11,D10));
	V19 = fabs(Det3d(E24,E16,D11));	V20 = fabs(Det3d(E14,E24,D9));
	V21 = fabs(Det3d(E16,D11,D10));	V22 = fabs(Det3d(E16,D5,D6));
	V23 = fabs(Det3d(E15,D1,D8));   V24 = fabs(Det3d(E15,E27,D4));          
        V25 = fabs(Det3d(E27,E17,D11)); V26 = fabs(Det3d(E14,E20,D11));         
        V27 = fabs(Det3d(E17,D11,D10)); V28 = fabs(Det3d(D11,D10,D6));          
        V29 = fabs(Det3d(E18,D11,D12)); V30 = fabs(Det3d(E22,E16,D7));          
	V31 = fabs(Det3d(E15,D1,D8));   V32 = fabs(Det3d(E15,E24,D9));          
	V33 = fabs(Det3d(E14,E24,D9));  V34 = fabs(Det3d(E23,E16,D11));         
	V35 = fabs(Det3d(E26,E17,D11)); V36 = fabs(Det3d(E17,D11,D10));         
	V37 = fabs(Det3d(E16,D11,D10)); V38 = fabs(Det3d(E16,D5,D6));		
	V39 = fabs(Det3d(E13,E15,D1));  V40 = fabs(Det3d(E13,D2,D3));
	V41 = fabs(Det3d(E13,E23,D9));  V42 = fabs(Det3d(E26,E21,D9));
	V43 = fabs(Det3d(E21,D9,D10));  V44 = fabs(Det3d(E21,E16,D5));
	V45 = fabs(Det3d(E16,D5,D6));
	
	vol_B = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15
			     +V16+V17+V18+V19+V20+V21+V22+V23+V24+V25+V26+V27
			     +V28+V29+V30+V31+V32+V33+V34+V35+V36+V37+V38+V39
			     +V40+V41+V42+V43+V44+V45 )/36.0;

	/* Symmetry Group C */
	/* In the 2 triangulations of this group, the 3 diagonals form a        */
	/* triangle inside the hexagon (only 2 possible rotations) */

	V1 = fabs(Det3d(E15,D6,D9));	/* tetrahedron crx5-crx6-c7-c5 */
	V2 = fabs(Det3d(E4,E15,D6)); 	/* tetrahedron crx1-crx5-crx6-c7 */
	V3 = fabs(Det3d(E5,D5,D8));	/* tetrahedron crx1-crx6-c8-c7   */
	V4 = fabs(Det3d(E9,D5,D8));	/* tetrahedron crx2-crx6-c8-c7    */
	V5 = fabs(Det3d(E1,E6,D2)); 	/* tetrahedron crx1-crx2-crx3-c4  */
	V6 = fabs(Det3d(E1,D1,D7));  	/* tetrahedron crx1-crx2-c4-c8  */
	V7 = fabs(Det3d(E1,E9,D5));  	/* tetrahedron crx1-crx2-crx6-c8  */
	V8 = fabs(Det3d(E7,E14,D6)); 	/* tetrahedron crx2-crx4-crx6-c7  */
	
	V9 = fabs(Det3d(E2,E11,D4));	/* tetrahedron crx1-crx3-crx5-c8  */
	V10 = fabs(Det3d(E10,E13,D4));	/* tetrahedron crx3-crx4-crx5-c8 */
	V11 = fabs(Det3d(E6,E10,D3));   /* tetrahedron crx2-crx3-crx4-c4 */
	V12 = fabs(Det3d(E10,D3,D7));	/* tetrahedron crx3-crx4-c4-c8 */
	V13 = fabs(Det3d(E13,D4,D8));	/* tetrahedron crx4-crx5-c8-c7 */
	V14 = fabs(Det3d(E15,D6,D9));	/* tetrahedron crx5-crx6-c7-c5 */
	V15 = fabs(Det3d(E13,E15,D6));	/* tetrahedron crx4-crx5-crx6-c7 */

	vol_C = ( V1+V2+V3+V4+V5+V6+V7+V8+V9+V10+V11+V12+V13+V14+V15 )/12.0;
	
/*	volume = (vol_A + vol_B + vol_C)/3.0; */
	volume = vol_C;
	return volume;
}	/* end volume_twister_case10 */

LOCAL double volume_corner1_case14(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;

	p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c3  */
	}
	
	volume = fabs(Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner1_case14 */

LOCAL double volume_corner4_case14(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;

	p1 = crx_in_jdir(blk_crx,1,0)->p; 
        p2 = crx_in_kdir(blk_crx,0,0)->p;
        p3 = crx_in_idir(blk_crx,0,1)->p;
			 
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c2  */
	}
	
	volume = fabs(Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner4_case14 */

LOCAL double volume_corner1_p2_case14(
	BLK_CRX *blk_crx,
	double crn[])
{
	int i;
	FT_POINT *p1, *p2, *p3;
	double *crx1, *crx2, *crx3; 
	double D1[3], D2[3], D3[3];
	double volume;

	p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_kdir(blk_crx,0,0)->p;
			
	crx1 = Coords(p1);
        crx2 = Coords(p2);
        crx3 = Coords(p3);
	
	for (i = 0; i < 3; i++)
	{
	    D1[i] = crx1[i] - crx2[i];
	    D2[i] = crx2[i] - crx3[i];
	    D3[i] = crx3[i] - crn[i];	/* crx3 - c1  */
	}
	
	volume = fabs(Det3d(D1,D2,D3))/6.0;
	return volume;
}	/* end volume_corner1_p2_case14 */

