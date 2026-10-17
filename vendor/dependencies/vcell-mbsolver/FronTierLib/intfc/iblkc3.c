/*
*				iblkc3.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Containing function of rebuilding interface within a mesh block.
*
*/

#if defined(THREED)

#include <intfc/int.h>

#define DEBUG_STRING "blk_intfc"

LOCAL	BBI_POINT *crx_in_idir(const BLK_CRX*,int,int);
LOCAL	BBI_POINT *crx_in_jdir(const BLK_CRX*,int,int);
LOCAL	BBI_POINT *crx_in_kdir(const BLK_CRX*,int,int);
LOCAL	BBI_POINT *crxing_in_between(const int*,const int*,const BLK_CRX*);
LOCAL   BBI_POINT *curve_crx_in_idir(const BLK_CRX*,int);
LOCAL   BBI_POINT *curve_crx_in_jdir(const BLK_CRX*,int);
LOCAL   BBI_POINT *curve_crx_in_kdir(const BLK_CRX*,int);
LOCAL   BBI_POINT *curve_crxing_in_between(const int*,const int*,const int*,const BLK_CRX*);
LOCAL   void copy_blk_crx(const BLK_CRX*,BLK_CRX*);
LOCAL   int compare_comp(COMPONENT***,COMPONENT****,int);
LOCAL	void set_prime_components(COMPONENT****);
LOCAL	int count_side_comp(COMPONENT,COMPONENT,COMPONENT,COMPONENT);
LOCAL	int check_consistence_of_crx(BLK_CRX*,boolean);
LOCAL   void rot24(BLK_CRX*,int);
LOCAL   void x_rotation(BLK_CRX*);
LOCAL   void y_rotation(BLK_CRX*);
LOCAL   void z_rotation(BLK_CRX*);
LOCAL   int is_curve(BLK_CRX*,CURVE*);
LOCAL   void blk_case01_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case02_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case03_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case04_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case05_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case06_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case07_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case08_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case09_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case10_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case11_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case12_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case13_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case14_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case15_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case16_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case17_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case18_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case19_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case20_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case21_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case22_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case23_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case24_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case25_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case26_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case27_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case28_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case29_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case30_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case31_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case32_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case33_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case34_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case35_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case36_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case37_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case38_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case39_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case40_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case41_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case42_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case43_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case44_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case45_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case46_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case47_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case48_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case49_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case50_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case51_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case52_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case53_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case54_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case55_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case56_comp3(BLK_CRX*,BLK_TRI*);
LOCAL   void blk_case57_comp3(BLK_CRX*,BLK_TRI*);
LOCAL	int is_positive_curve(CURVE*,SURFACE*);

LOCAL  const char   *blk_name;
EXPORT  void  set_debug_name(const char *s)
{
	blk_name = s;
}

EXPORT	int construct_comp3_blk(
	BLK_CRX *blk_crx,
	BLK_TRI *blk_mem)
{
	int       i,j,k;
	COMPONENT ***comp = blk_crx->comp;
	int       num_crx, case_found;
	static COMPONENT ****prime_comp;
	static BLK_CRX *bc_rot;
	void (*blk_intfc_comp3[57])(BLK_CRX*,BLK_TRI*) =
	{
	    blk_case01_comp3,
	    blk_case02_comp3,
	    blk_case03_comp3,
	    blk_case04_comp3,
	    blk_case05_comp3,
	    blk_case06_comp3,
	    blk_case07_comp3,
	    blk_case08_comp3,
	    blk_case09_comp3,
	    blk_case10_comp3,
	    blk_case11_comp3,
	    blk_case12_comp3,
	    blk_case13_comp3,
	    blk_case14_comp3,
	    blk_case15_comp3,
	    blk_case16_comp3,
	    blk_case17_comp3,
	    blk_case18_comp3,
	    blk_case19_comp3,
	    blk_case20_comp3,
	    blk_case21_comp3,
	    blk_case22_comp3,
	    blk_case23_comp3,
	    blk_case24_comp3,
	    blk_case25_comp3,
	    blk_case26_comp3,
	    blk_case27_comp3,
	    blk_case28_comp3,
	    blk_case29_comp3,
	    blk_case30_comp3,
	    blk_case31_comp3,
	    blk_case32_comp3,
	    blk_case33_comp3,
	    blk_case34_comp3,
	    blk_case35_comp3,
	    blk_case36_comp3,
	    blk_case37_comp3,
	    blk_case38_comp3,
	    blk_case39_comp3,
	    blk_case40_comp3,
	    blk_case41_comp3,
	    blk_case42_comp3,
	    blk_case43_comp3,
	    blk_case44_comp3,
	    blk_case45_comp3,
	    blk_case46_comp3,
	    blk_case47_comp3,
	    blk_case48_comp3,
	    blk_case49_comp3,
	    blk_case50_comp3,
	    blk_case51_comp3,
	    blk_case52_comp3,
	    blk_case53_comp3,
	    blk_case54_comp3,
	    blk_case55_comp3,
	    blk_case56_comp3,
	    blk_case57_comp3,
	};

	if (prime_comp == NULL)
	{
	    quad_array(&prime_comp,57,2,2,2,sizeof(COMPONENT));
	    set_prime_components(prime_comp);
	}
	
	num_crx = check_consistence_of_crx(blk_crx,YES);
	
	if (num_crx == 0)
	{
	        /* No interface, but ONFRONT, this happens */
	        for (i = 0; i < 7; i++)
		    blk_mem->num_tris[i] = 0;
	        return FUNCTION_SUCCEEDED;
	}
	if ((blk_crx->nv[0] > 2) || (blk_crx->nv[1] > 3))
	{
	    screen("ERROR: in construct_comp3_blk(), no such case!\n");
	    clean_up(ERROR);
	}
	for (i = 0; i < 7; i++)
	{
	    blk_mem->first[i] = NULL;
	    blk_mem->num_tris[i] = 0;
	}

	if (bc_rot == NULL) bc_rot = alloc_blk_crx(NO);
	copy_blk_crx(blk_crx, bc_rot);

	/* begin 24-rotation */
	case_found = NO;
	for (j = 0; j <= 24; j++)
	{
	    for (i = 0; i < 57; i++)
	    {
	        if (compare_comp(bc_rot->comp, prime_comp, i)) 
	        { 
		    if(debugging("print_blk"))
		    {
		        printf("construct_comp3_blk(bc_rot) exame i=%d, j %d\n", i, j); 	    
		        /*print_blk_crx(blk_crx); */
		        print_blk_crx(bc_rot);
		    }
		    blk_intfc_comp3[i](bc_rot, blk_mem);
		    case_found = YES;

		    if(debugging("print_blk") || debugging("case_num"))
		    {
		        printf("construct_comp3_blk found case %d\n", i+1); 	    
		        if(i+1 == 17)
			{
			    printf("#new db tst\n");
			    blk_crx->debug_flag = YES;
			    set_debug_name("bonddb");
			}
		    }
		    if (debugging("BLK_check"))
		    {
	                static int ib, ic;
		        
			ic = blk_mem->ic[0];
			printf("0 case=%d, num_surfaces=%d, num_curves=%d\n", 
			      i+1, blk_mem->num_surfaces, blk_mem->num_curves);
			printf("ic=%d, curve=%p, bond=%p\n", 
			      ic, blk_mem->curves[ic], blk_mem->bonds[ic]);
			
			if ((blk_mem->bonds[0] != NULL) && (i==30))
		        {
		            ib++;
			    printf("case-%d finding bonds[%d]\n",i+1,ib);
			    printf("%f %f %f\n",Coords(blk_mem->bonds[0]->start)[0],
			        Coords(blk_mem->bonds[0]->start)[1],
			        Coords(blk_mem->bonds[0]->start)[2]);
			     printf("%f %f %f\n",Coords(blk_mem->bonds[0]->end)[0],
			        Coords(blk_mem->bonds[0]->end)[1],
			        Coords(blk_mem->bonds[0]->end)[2]);
		        }
		    }
	            break;
	        }
	    }
	    if (case_found == YES) break;
	    rot24(bc_rot, j);
	}
	
	if (case_found == NO)
	{
    	    COMPONENT c_tmp; 
	    if (blk_crx->nv[0] == blk_crx->nv[1]) 
	    { 
	        c_tmp = blk_crx->comps[1]; 
		blk_crx->comps[1] = blk_crx->comps[0]; 
		blk_crx->comps[0] = c_tmp; 
	    } 
	    else if (blk_crx->nv[1] == blk_crx->nv[2]) 
	    { 
	        c_tmp = blk_crx->comps[1]; 
		blk_crx->comps[1] = blk_crx->comps[2]; 
		blk_crx->comps[2] = c_tmp; 
	    }   
	    copy_blk_crx(blk_crx, bc_rot);
	    for (j = 0; j <= 24; j++)
	    {
	        for (i = 0; i < 57; i++)
	        { 
	            if (compare_comp(bc_rot->comp, prime_comp, i)) 
	            { 
			(void)printf("WARNING: Special case is found\n");
	                blk_intfc_comp3[i](bc_rot, blk_mem);
			case_found = YES;
			
			if(debugging("print_blk") || debugging("case_num"))
		            printf("construct_comp3_blk found case %d\n", i+1); 	    

		        if (debugging("BLK_check"))
		        {
			    static int ib, ic;
		            
			    ic = blk_mem->ic[0];
			    printf("1 case=%d, num_surfaces=%d, num_curves=%d\n", 
			          i+1, blk_mem->num_surfaces, blk_mem->num_curves);
			    printf("ic=%d, curve=%p, bond=%p\n", 
			          ic, blk_mem->curves[ic], blk_mem->bonds[ic]);

			    printf("case=%d, bond=%p \n", i+1, blk_mem->bonds[0]);
			    if ((blk_mem->bonds[0] != NULL) && (i == 30))
		            {
		                ib++;
			        printf("case-%d finding bonds[%d]\n",i+1,ib);
			        printf("%f %f %f\n",Coords(blk_mem->bonds[0]->start)[0],
			            Coords(blk_mem->bonds[0]->start)[1],
			            Coords(blk_mem->bonds[0]->start)[2]);
			         printf("%f %f %f\n",Coords(blk_mem->bonds[0]->end)[0],
			            Coords(blk_mem->bonds[0]->end)[1],
			            Coords(blk_mem->bonds[0]->end)[2]);
		            }
		        }
	                break;
	            }
	        }
	        if (case_found == YES) break;
	        rot24(bc_rot, j);
	    }
	    if (case_found == NO)
	    { 
	        (void)printf("ERROR: No case is found\n");
	        for (i = 0; i < 2; i++)
	        for (j = 0; j < 2; j++) 
	        for (k = 0; k < 2; k++)
	        {
	            (void)printf("comp[%d][%d][%d] = %d\n",i,j,k,comp[i][j][k]);
	        }
	        clean_up(ERROR);
	    }
	}

	stitch_inside_blk(blk_mem);
	
	if(blk_crx->debug_flag)
	{
	    char     s[40];
	    
	    printf("#bm debug  %s \n", blk_name);
	    /*print_blk_crx(blk_crx); */
	    tecplot_blk_intfc_plot(blk_name, blk_mem);
	    print_blk_tri(blk_mem);
	}

	return FUNCTION_SUCCEEDED;
}	/* end reconstruct_blk_intfc */


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
	    if (debugging("comp3_blk"))
	    {
		printf("ip[1] = %d  ip[2] = %d\n",ip[1],ip[2]);
		printf("blk_crx->crx[0][ip[1]][ip[2]] = %p\n",
				blk_crx->crx[0][ip[1]][ip[2]]);
	    }
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

LOCAL   BBI_POINT *curve_crx_in_idir(
        const BLK_CRX *blk_crx, 
	int i)
{       
        int ipx[4],ipy[4],ipz[4];
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	BBI_POINT *bbi;

	ipx[0] = ix[i][0][0];
	ipx[1] = ix[i][0][1];
	ipx[2] = ix[i][1][0];
	ipx[3] = ix[i][1][1];

	ipy[0] = iy[i][0][0];
	ipy[1] = iy[i][0][1];
	ipy[2] = iy[i][1][0];
	ipy[3] = iy[i][1][1];
	
	ipz[0] = iz[i][0][0];
	ipz[1] = iz[i][0][1];
	ipz[2] = iz[i][1][0];
	ipz[3] = iz[i][1][1];
	
	bbi = curve_crxing_in_between(ipx,ipy,ipz,blk_crx);
	return bbi;
}       /* end curve_crx_in_idir */

LOCAL   BBI_POINT *curve_crx_in_jdir(
        const BLK_CRX *blk_crx, 
	int j)
{       
        int ipx[4],ipy[4],ipz[4];
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	BBI_POINT *bbi;

	ipx[0] = ix[0][j][0];
	ipx[1] = ix[0][j][1];
	ipx[2] = ix[1][j][0];
	ipx[3] = ix[1][j][1];

	ipy[0] = iy[0][j][0];
	ipy[1] = iy[0][j][1];
	ipy[2] = iy[1][j][0];
	ipy[3] = iy[1][j][1];

	ipz[0] = iz[0][j][0];
	ipz[1] = iz[0][j][1];
	ipz[2] = iz[1][j][0];
	ipz[3] = iz[1][j][1];
	
	bbi = curve_crxing_in_between(ipx,ipy,ipz,blk_crx);
	return bbi;
}       /* end curve_crx_in_jdir */

LOCAL   BBI_POINT *curve_crx_in_kdir(
        const BLK_CRX *blk_crx, 
	int k)
{       
        int ipx[4],ipy[4],ipz[4];
	int ***ix = blk_crx->ix;
	int ***iy = blk_crx->iy;
	int ***iz = blk_crx->iz;
	BBI_POINT *bbi;

	ipx[0] = ix[0][0][k];
	ipx[1] = ix[0][1][k];
	ipx[2] = ix[1][0][k];
	ipx[3] = ix[1][1][k];

	ipy[0] = iy[0][0][k];
	ipy[1] = iy[0][1][k];
	ipy[2] = iy[1][0][k];
	ipy[3] = iy[1][1][k];

	ipz[0] = iz[0][0][k];
	ipz[1] = iz[0][1][k];
	ipz[2] = iz[1][0][k];
	ipz[3] = iz[1][1][k];
	
	bbi = curve_crxing_in_between(ipx,ipy,ipz,blk_crx);
	return bbi;
}       /* end curve_crx_in_kdir */

LOCAL   BBI_POINT *curve_crxing_in_between(
        const int *ipx,
	const int *ipy,
	const int *ipz,
	const BLK_CRX *blk_crx)
{
	if ((ipx[0] == ipx[1]) && (ipx[1] == ipx[2]) && (ipx[2] == ipx[3]))
	{
	    if (blk_crx->curve_crx[0][ipx[0]]->p == NULL)
	    {
	        screen("ERROR in curve_crxing_in_between(), "
		    "no crossing point in curve_crx[0][%d]\n",ipx[0]);
		print_blk_crx(blk_crx);
		clean_up(ERROR);
	    }
	    return blk_crx->curve_crx[0][ipx[0]];
	}
	if ((ipy[0] == ipy[1]) && (ipy[1] == ipy[2]) && (ipy[2] == ipy[3]))
	{
	    if (blk_crx->curve_crx[1][ipy[0]]->p == NULL)
	    {
	        screen("ERROR in curve_crxing_in_between(), "
		    "no crossing point in curve_crx[1][%d]\n",ipy[0]);
		print_blk_crx(blk_crx);
		clean_up(ERROR);
	    }
	    return blk_crx->curve_crx[1][ipy[0]];
	}
	if ((ipz[0] == ipz[1]) && (ipz[1] == ipz[2]) && (ipz[2] == ipz[3]))
	{
	    if (blk_crx->curve_crx[2][ipz[0]]->p == NULL)
	    {
	        screen("ERROR in curve_crxing_in_between(), "
		    "no crossing point in curve_crx[2][%d]\n",ipz[0]);
		print_blk_crx(blk_crx);
		clean_up(ERROR);
	    }
	    return blk_crx->curve_crx[2][ipz[0]];
	}
}       /* end curve_crxing_in_between */

LOCAL  void rot24(
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

LOCAL   void copy_blk_crx(
        const BLK_CRX *blk_crx1,
	BLK_CRX       *blk_crx2)
{
        int i,j,k;
	
        blk_crx2->blk_info = blk_crx1->blk_info;
	
	for (i = 0; i < 3; ++i)
	for (j = 0; j < 2; ++j)
	{
	    for (k = 0; k < 2; ++k)
	        blk_crx2->crx[i][j][k] = blk_crx1->crx[i][j][k];
	    blk_crx2->curve_crx[i][j] = blk_crx1->curve_crx[i][j];
	}
	    
	for (i = 0; i < 2; i++) 
	for (j = 0; j < 2; j++)
	for (k = 0; k < 2; k++)
	{
	    if (blk_crx1->comp[i][j][k] == blk_crx1->comps[0])
	        blk_crx2->comp[i][j][k] = 0;
	    else if (blk_crx1->comp[i][j][k] == blk_crx1->comps[1])
	        blk_crx2->comp[i][j][k] = 1;
	    else
	        blk_crx2->comp[i][j][k] = 2;
	    blk_crx2->ix[i][j][k] = blk_crx1->ix[i][j][k];
	    blk_crx2->iy[i][j][k] = blk_crx1->iy[i][j][k];
	    blk_crx2->iz[i][j][k] = blk_crx1->iz[i][j][k];
	}
	for (i = 0; i < 8; i++)
	    blk_crx2->comps[i] = blk_crx1->comps[i];
}      /*end copy_blk_crx*/

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
	/* Case 1: /  (1, 1, 6) */

	pcomp[0][0][0][0] = 2;
	pcomp[0][0][0][1] = 2;
	pcomp[0][0][1][0] = 2;
	pcomp[0][0][1][1] = 2;
	pcomp[0][1][0][0] = 2;
	pcomp[0][1][0][1] = 0;
	pcomp[0][1][1][0] = 2;
	pcomp[0][1][1][1] = 1;

	/* Case 2  (1, 1, 6) */

	pcomp[1][0][0][0] = 2;
	pcomp[1][0][0][1] = 2;
	pcomp[1][0][1][0] = 2;
	pcomp[1][0][1][1] = 1;
	pcomp[1][1][0][0] = 2;
	pcomp[1][1][0][1] = 0;
	pcomp[1][1][1][0] = 2;
	pcomp[1][1][1][1] = 2;

	/* Case 3  (1, 1, 6) */

	pcomp[2][0][0][0] = 2;
	pcomp[2][0][0][1] = 2;
	pcomp[2][0][1][0] = 1;
	pcomp[2][0][1][1] = 2;
	pcomp[2][1][0][0] = 2;
	pcomp[2][1][0][1] = 0;
	pcomp[2][1][1][0] = 2;
	pcomp[2][1][1][1] = 2;

	/* Case 4  (1, 2, 5) */

	pcomp[3][0][0][0] = 2;
	pcomp[3][0][0][1] = 2;
	pcomp[3][0][1][0] = 2;
	pcomp[3][0][1][1] = 1;
	pcomp[3][1][0][0] = 2;
	pcomp[3][1][0][1] = 0;
	pcomp[3][1][1][0] = 2;
	pcomp[3][1][1][1] = 1;

	/* Case 5  (1, 2, 5) */

	pcomp[4][0][0][0] = 2;
	pcomp[4][0][0][1] = 1;
	pcomp[4][0][1][0] = 2;
	pcomp[4][0][1][1] = 2;
	pcomp[4][1][0][0] = 2;
	pcomp[4][1][0][1] = 0;
	pcomp[4][1][1][0] = 2;
	pcomp[4][1][1][1] = 1;

	/* Case 6  (1, 2, 5) */

	pcomp[5][0][0][0] = 2;
	pcomp[5][0][0][1] = 2;
	pcomp[5][0][1][0] = 2;
	pcomp[5][0][1][1] = 2;
	pcomp[5][1][0][0] = 2;
	pcomp[5][1][0][1] = 0;
	pcomp[5][1][1][0] = 1;
	pcomp[5][1][1][1] = 1;

	/* Case 7  (1, 2, 5) */

	pcomp[6][0][0][0] = 2;
	pcomp[6][0][0][1] = 2;
	pcomp[6][0][1][0] = 1;
	pcomp[6][0][1][1] = 2;
	pcomp[6][1][0][0] = 2;
	pcomp[6][1][0][1] = 0;
	pcomp[6][1][1][0] = 2;
	pcomp[6][1][1][1] = 1;

	/* Case 8  (1, 2, 5) */

	pcomp[7][0][0][0] = 1;
	pcomp[7][0][0][1] = 2;
	pcomp[7][0][1][0] = 2;
	pcomp[7][0][1][1] = 2;
	pcomp[7][1][0][0] = 2;
	pcomp[7][1][0][1] = 0;
	pcomp[7][1][1][0] = 2;
	pcomp[7][1][1][1] = 1;

	/* Case 9  (1, 2, 5) */

	pcomp[8][0][0][0] = 2;
	pcomp[8][0][0][1] = 2;
	pcomp[8][0][1][0] = 2;
	pcomp[8][0][1][1] = 1;
	pcomp[8][1][0][0] = 2;
	pcomp[8][1][0][1] = 0;
	pcomp[8][1][1][0] = 1;
	pcomp[8][1][1][1] = 2;

	/* Case 10  (1, 2, 5) */

	pcomp[9][0][0][0] = 2;
	pcomp[9][0][0][1] = 2;
	pcomp[9][0][1][0] = 1;
	pcomp[9][0][1][1] = 1;
	pcomp[9][1][0][0] = 2;
	pcomp[9][1][0][1] = 0;
	pcomp[9][1][1][0] = 2;
	pcomp[9][1][1][1] = 2;

	/* Case 11  (1, 3, 4) */

	pcomp[10][0][0][0] = 2;
	pcomp[10][0][0][1] = 1;
	pcomp[10][0][1][0] = 2;
	pcomp[10][0][1][1] = 1;
	pcomp[10][1][0][0] = 2;
	pcomp[10][1][0][1] = 0;
	pcomp[10][1][1][0] = 2;
	pcomp[10][1][1][1] = 1;

	/* Case 12  (1, 3, 4) */

	pcomp[11][0][0][0] = 2;
	pcomp[11][0][0][1] = 2;
	pcomp[11][0][1][0] = 2;
	pcomp[11][0][1][1] = 1;
	pcomp[11][1][0][0] = 1;
	pcomp[11][1][0][1] = 0;
	pcomp[11][1][1][0] = 2;
	pcomp[11][1][1][1] = 1;

	/* Case 13  (1, 3, 4) */

	pcomp[12][0][0][0] = 2;
	pcomp[12][0][0][1] = 2;
	pcomp[12][0][1][0] = 2;
	pcomp[12][0][1][1] = 1;
	pcomp[12][1][0][0] = 2;
	pcomp[12][1][0][1] = 0;
	pcomp[12][1][1][0] = 1;
	pcomp[12][1][1][1] = 1;

	/* Case 14  (1, 3, 4) */

	pcomp[13][0][0][0] = 2;
	pcomp[13][0][0][1] = 2;
	pcomp[13][0][1][0] = 1;
	pcomp[13][0][1][1] = 1;
	pcomp[13][1][0][0] = 2;
	pcomp[13][1][0][1] = 0;
	pcomp[13][1][1][0] = 2;
	pcomp[13][1][1][1] = 1;

	/* Case 15  (1, 3, 4) */

	pcomp[14][0][0][0] = 1;
	pcomp[14][0][0][1] = 2;
	pcomp[14][0][1][0] = 2;
	pcomp[14][0][1][1] = 1;
	pcomp[14][1][0][0] = 2;
	pcomp[14][1][0][1] = 0;
	pcomp[14][1][1][0] = 2;
	pcomp[14][1][1][1] = 1;

	/* Case 16  (1, 3, 4) */

	pcomp[15][0][0][0] = 2;
	pcomp[15][0][0][1] = 1;
	pcomp[15][0][1][0] = 2;
	pcomp[15][0][1][1] = 2;
	pcomp[15][1][0][0] = 1;
	pcomp[15][1][0][1] = 0;
	pcomp[15][1][1][0] = 2;
	pcomp[15][1][1][1] = 1;

	/* Case 17  (1, 3, 4) */

	pcomp[16][0][0][0] = 2;
	pcomp[16][0][0][1] = 1;
	pcomp[16][0][1][0] = 2;
	pcomp[16][0][1][1] = 2;
	pcomp[16][1][0][0] = 2;
	pcomp[16][1][0][1] = 0;
	pcomp[16][1][1][0] = 1;
	pcomp[16][1][1][1] = 1;

	/* Case 18  (1, 3, 4) */

	pcomp[17][0][0][0] = 2;
	pcomp[17][0][0][1] = 1;
	pcomp[17][0][1][0] = 1;
	pcomp[17][0][1][1] = 2;
	pcomp[17][1][0][0] = 2;
	pcomp[17][1][0][1] = 0;
	pcomp[17][1][1][0] = 2;
	pcomp[17][1][1][1] = 1;

	/* Case 19  (1, 3, 4) */

	pcomp[18][0][0][0] = 2;
	pcomp[18][0][0][1] = 2;
	pcomp[18][0][1][0] = 1;
	pcomp[18][0][1][1] = 2;
	pcomp[18][1][0][0] = 2;
	pcomp[18][1][0][1] = 0;
	pcomp[18][1][1][0] = 1;
	pcomp[18][1][1][1] = 1;

	/* Case 20  (1, 3, 4) */

	pcomp[19][0][0][0] = 1;
	pcomp[19][0][0][1] = 2;
	pcomp[19][0][1][0] = 2;
	pcomp[19][0][1][1] = 2;
	pcomp[19][1][0][0] = 2;
	pcomp[19][1][0][1] = 0;
	pcomp[19][1][1][0] = 1;
	pcomp[19][1][1][1] = 1;

	/* Case 21  (1, 3, 4) */

	pcomp[20][0][0][0] = 1;
	pcomp[20][0][0][1] = 2;
	pcomp[20][0][1][0] = 1;
	pcomp[20][0][1][1] = 2;
	pcomp[20][1][0][0] = 2;
	pcomp[20][1][0][1] = 0;
	pcomp[20][1][1][0] = 2;
	pcomp[20][1][1][1] = 1;

	/* Case 22  (1, 3, 4) */

	pcomp[21][0][0][0] = 2;
	pcomp[21][0][0][1] = 2;
	pcomp[21][0][1][0] = 1;
	pcomp[21][0][1][1] = 1;
	pcomp[21][1][0][0] = 2;
	pcomp[21][1][0][1] = 0;
	pcomp[21][1][1][0] = 1;
	pcomp[21][1][1][1] = 2;

	/* Case 23  (1, 3, 4) */

	pcomp[22][0][0][0] = 1;
	pcomp[22][0][0][1] = 2;
	pcomp[22][0][1][0] = 2;
	pcomp[22][0][1][1] = 1;
	pcomp[22][1][0][0] = 2;
	pcomp[22][1][0][1] = 0;
	pcomp[22][1][1][0] = 1;
	pcomp[22][1][1][1] = 2;

	/* Case 24  (2, 2, 4) */

	pcomp[23][0][0][0] = 2;
	pcomp[23][0][0][1] = 1;
	pcomp[23][0][1][0] = 2;
	pcomp[23][0][1][1] = 1;
	pcomp[23][1][0][0] = 2;
	pcomp[23][1][0][1] = 0;
	pcomp[23][1][1][0] = 2;
	pcomp[23][1][1][1] = 0;

	/* Case 25  (2, 2, 4) */

	pcomp[24][0][0][0] = 2;
	pcomp[24][0][0][1] = 2;
	pcomp[24][0][1][0] = 2;
	pcomp[24][0][1][1] = 1;
	pcomp[24][1][0][0] = 1;
	pcomp[24][1][0][1] = 0;
	pcomp[24][1][1][0] = 2;
	pcomp[24][1][1][1] = 0;

	/* Case 26  (2, 2, 4) */

	pcomp[25][0][0][0] = 2;
	pcomp[25][0][0][1] = 2;
	pcomp[25][0][1][0] = 2;
	pcomp[25][0][1][1] = 1;
	pcomp[25][1][0][0] = 2;
	pcomp[25][1][0][1] = 0;
	pcomp[25][1][1][0] = 1;
	pcomp[25][1][1][1] = 0;

	/* Case 27  (2, 2, 4) */

	pcomp[26][0][0][0] = 2;
	pcomp[26][0][0][1] = 2;
	pcomp[26][0][1][0] = 1;
	pcomp[26][0][1][1] = 1;
	pcomp[26][1][0][0] = 2;
	pcomp[26][1][0][1] = 0;
	pcomp[26][1][1][0] = 2;
	pcomp[26][1][1][1] = 0;

	/* Case 28  (2, 2, 4) */

	pcomp[27][0][0][0] = 1;
	pcomp[27][0][0][1] = 2;
	pcomp[27][0][1][0] = 2;
	pcomp[27][0][1][1] = 1;
	pcomp[27][1][0][0] = 2;
	pcomp[27][1][0][1] = 0;
	pcomp[27][1][1][0] = 2;
	pcomp[27][1][1][1] = 0;

	/* Case 29  (2, 2, 4) */

	pcomp[28][0][0][0] = 2;
	pcomp[28][0][0][1] = 1;
	pcomp[28][0][1][0] = 2;
	pcomp[28][0][1][1] = 2;
	pcomp[28][1][0][0] = 2;
	pcomp[28][1][0][1] = 0;
	pcomp[28][1][1][0] = 1;
	pcomp[28][1][1][1] = 0;

	/* Case 30  (2, 2, 4) */

	pcomp[29][0][0][0] = 2;
	pcomp[29][0][0][1] = 1;
	pcomp[29][0][1][0] = 1;
	pcomp[29][0][1][1] = 2;
	pcomp[29][1][0][0] = 2;
	pcomp[29][1][0][1] = 0;
	pcomp[29][1][1][0] = 2;
	pcomp[29][1][1][1] = 0;

	/* Case 31  (2, 2, 4) */

	pcomp[30][0][0][0] = 1;
	pcomp[30][0][0][1] = 1;
	pcomp[30][0][1][0] = 2;
	pcomp[30][0][1][1] = 2;
	pcomp[30][1][0][0] = 2;
	pcomp[30][1][0][1] = 0;
	pcomp[30][1][1][0] = 2;
	pcomp[30][1][1][1] = 0;

	/* Case 32  (2, 2, 4) */

	pcomp[31][0][0][0] = 1;
	pcomp[31][0][0][1] = 2;
	pcomp[31][0][1][0] = 1;
	pcomp[31][0][1][1] = 2;
	pcomp[31][1][0][0] = 2;
	pcomp[31][1][0][1] = 0;
	pcomp[31][1][1][0] = 2;
	pcomp[31][1][1][1] = 0;

	/* Case 33  (2, 2, 4) */

	pcomp[32][0][0][0] = 2;
	pcomp[32][0][0][1] = 1;
	pcomp[32][0][1][0] = 2;
	pcomp[32][0][1][1] = 0;
	pcomp[32][1][0][0] = 2;
	pcomp[32][1][0][1] = 0;
	pcomp[32][1][1][0] = 2;
	pcomp[32][1][1][1] = 1;

	/* Case 34  (2, 2, 4) */

	pcomp[33][0][0][0] = 2;
	pcomp[33][0][0][1] = 2;
	pcomp[33][0][1][0] = 2;
	pcomp[33][0][1][1] = 0;
	pcomp[33][1][0][0] = 1;
	pcomp[33][1][0][1] = 0;
	pcomp[33][1][1][0] = 2;
	pcomp[33][1][1][1] = 1;

	/* Case 35  (2, 2, 4) */

	pcomp[34][0][0][0] = 2;
	pcomp[34][0][0][1] = 2;
	pcomp[34][0][1][0] = 1;
	pcomp[34][0][1][1] = 0;
	pcomp[34][1][0][0] = 2;
	pcomp[34][1][0][1] = 0;
	pcomp[34][1][1][0] = 2;
	pcomp[34][1][1][1] = 1;

	/* Case 36  (2, 2, 4) */

	pcomp[35][0][0][0] = 1;
	pcomp[35][0][0][1] = 2;
	pcomp[35][0][1][0] = 2;
	pcomp[35][0][1][1] = 0;
	pcomp[35][1][0][0] = 2;
	pcomp[35][1][0][1] = 0;
	pcomp[35][1][1][0] = 2;
	pcomp[35][1][1][1] = 1;

	/* Case 37  (2, 2, 4) */

	pcomp[36][0][0][0] = 1;
	pcomp[36][0][0][1] = 2;
	pcomp[36][0][1][0] = 0;
	pcomp[36][0][1][1] = 2;
	pcomp[36][1][0][0] = 2;
	pcomp[36][1][0][1] = 0;
	pcomp[36][1][1][0] = 2;
	pcomp[36][1][1][1] = 1;

	/* Case 38  (2, 2, 4) */

	pcomp[37][0][0][0] = 0;
	pcomp[37][0][0][1] = 2;
	pcomp[37][0][1][0] = 1;
	pcomp[37][0][1][1] = 2;
	pcomp[37][1][0][0] = 2;
	pcomp[37][1][0][1] = 0;
	pcomp[37][1][1][0] = 2;
	pcomp[37][1][1][1] = 1;

	/* Case 39  (2, 2, 4) */

	pcomp[38][0][0][0] = 1;
	pcomp[38][0][0][1] = 2;
	pcomp[38][0][1][0] = 2;
	pcomp[38][0][1][1] = 0;
	pcomp[38][1][0][0] = 2;
	pcomp[38][1][0][1] = 0;
	pcomp[38][1][1][0] = 1;
	pcomp[38][1][1][1] = 2;

	/* Case 40  (2, 3, 3) */

	pcomp[39][0][0][0] = 2;
	pcomp[39][0][0][1] = 1;
	pcomp[39][0][1][0] = 2;
	pcomp[39][0][1][1] = 1;
	pcomp[39][1][0][0] = 1;
	pcomp[39][1][0][1] = 0;
	pcomp[39][1][1][0] = 2;
	pcomp[39][1][1][1] = 0;

	/* Case 41  (2, 3, 3) */

	pcomp[40][0][0][0] = 2;
	pcomp[40][0][0][1] = 1;
	pcomp[40][0][1][0] = 2;
	pcomp[40][0][1][1] = 1;
	pcomp[40][1][0][0] = 2;
	pcomp[40][1][0][1] = 0;
	pcomp[40][1][1][0] = 1;
	pcomp[40][1][1][1] = 0;

	/* Case 42  (2, 3, 3) */

	pcomp[41][0][0][0] = 2;
	pcomp[41][0][0][1] = 1;
	pcomp[41][0][1][0] = 1;
	pcomp[41][0][1][1] = 1;
	pcomp[41][1][0][0] = 2;
	pcomp[41][1][0][1] = 0;
	pcomp[41][1][1][0] = 2;
	pcomp[41][1][1][1] = 0;

	/* Case 43  (2, 3, 3) */

	pcomp[42][0][0][0] = 1;
	pcomp[42][0][0][1] = 1;
	pcomp[42][0][1][0] = 2;
	pcomp[42][0][1][1] = 1;
	pcomp[42][1][0][0] = 2;
	pcomp[42][1][0][1] = 0;
	pcomp[42][1][1][0] = 2;
	pcomp[42][1][1][1] = 0;

	/* Case 44  (2, 3, 3) */

	pcomp[43][0][0][0] = 2;
	pcomp[43][0][0][1] = 2;
	pcomp[43][0][1][0] = 1;
	pcomp[43][0][1][1] = 1;
	pcomp[43][1][0][0] = 1;
	pcomp[43][1][0][1] = 0;
	pcomp[43][1][1][0] = 2;
	pcomp[43][1][1][1] = 0;

	/* Case 45  (2, 3, 3) */

	pcomp[44][0][0][0] = 2;
	pcomp[44][0][0][1] = 2;
	pcomp[44][0][1][0] = 1;
	pcomp[44][0][1][1] = 1;
	pcomp[44][1][0][0] = 2;
	pcomp[44][1][0][1] = 0;
	pcomp[44][1][1][0] = 1;
	pcomp[44][1][1][1] = 0;

	/* Case 46  (2, 3, 3) */

	pcomp[45][0][0][0] = 1;
	pcomp[45][0][0][1] = 2;
	pcomp[45][0][1][0] = 2;
	pcomp[45][0][1][1] = 1;
	pcomp[45][1][0][0] = 2;
	pcomp[45][1][0][1] = 0;
	pcomp[45][1][1][0] = 1;
	pcomp[45][1][1][1] = 0;

	/* Case 47  (2, 3, 3) */

	pcomp[46][0][0][0] = 2;
	pcomp[46][0][0][1] = 1;
	pcomp[46][0][1][0] = 2;
	pcomp[46][0][1][1] = 0;
	pcomp[46][1][0][0] = 1;
	pcomp[46][1][0][1] = 0;
	pcomp[46][1][1][0] = 2;
	pcomp[46][1][1][1] = 1;

	/* Case 48  (2, 3, 3) */

	pcomp[47][0][0][0] = 2;
	pcomp[47][0][0][1] = 1;
	pcomp[47][0][1][0] = 2;
	pcomp[47][0][1][1] = 0;
	pcomp[47][1][0][0] = 2;
	pcomp[47][1][0][1] = 0;
	pcomp[47][1][1][0] = 1;
	pcomp[47][1][1][1] = 1;

	/* Case 49  (2, 3, 3) */

	pcomp[48][0][0][0] = 2;
	pcomp[48][0][0][1] = 2;
	pcomp[48][0][1][0] = 2;
	pcomp[48][0][1][1] = 0;
	pcomp[48][1][0][0] = 1;
	pcomp[48][1][0][1] = 0;
	pcomp[48][1][1][0] = 1;
	pcomp[48][1][1][1] = 1;

	/* Case 50  (2, 3, 3) */

	pcomp[49][0][0][0] = 2;
	pcomp[49][0][0][1] = 2;
	pcomp[49][0][1][0] = 1;
	pcomp[49][0][1][1] = 0;
	pcomp[49][1][0][0] = 1;
	pcomp[49][1][0][1] = 0;
	pcomp[49][1][1][0] = 2;
	pcomp[49][1][1][1] = 1;

	/* Case 51  (2, 3, 3) */

	pcomp[50][0][0][0] = 1;
	pcomp[50][0][0][1] = 2;
	pcomp[50][0][1][0] = 2;
	pcomp[50][0][1][1] = 0;
	pcomp[50][1][0][0] = 1;
	pcomp[50][1][0][1] = 0;
	pcomp[50][1][1][0] = 2;
	pcomp[50][1][1][1] = 1;

	/* Case 52  (2, 3, 3) */

	pcomp[51][0][0][0] = 2;
	pcomp[51][0][0][1] = 2;
	pcomp[51][0][1][0] = 1;
	pcomp[51][0][1][1] = 0;
	pcomp[51][1][0][0] = 2;
	pcomp[51][1][0][1] = 0;
	pcomp[51][1][1][0] = 1;
	pcomp[51][1][1][1] = 1;

	/* Case 53  (2, 3, 3) */

	pcomp[52][0][0][0] = 1;
	pcomp[52][0][0][1] = 2;
	pcomp[52][0][1][0] = 1;
	pcomp[52][0][1][1] = 0;
	pcomp[52][1][0][0] = 2;
	pcomp[52][1][0][1] = 0;
	pcomp[52][1][1][0] = 2;
	pcomp[52][1][1][1] = 1;

	/* Case 54  (2, 3, 3) */

	pcomp[53][0][0][0] = 2;
	pcomp[53][0][0][1] = 1;
	pcomp[53][0][1][0] = 0;
	pcomp[53][0][1][1] = 1;
	pcomp[53][1][0][0] = 2;
	pcomp[53][1][0][1] = 0;
	pcomp[53][1][1][0] = 2;
	pcomp[53][1][1][1] = 1;

	/* Case 55  (2, 3, 3) */

	pcomp[54][0][0][0] = 2;
	pcomp[54][0][0][1] = 2;
	pcomp[54][0][1][0] = 0;
	pcomp[54][0][1][1] = 1;
	pcomp[54][1][0][0] = 1;
	pcomp[54][1][0][1] = 0;
	pcomp[54][1][1][0] = 2;
	pcomp[54][1][1][1] = 1;

	/* Case 56  (2, 3, 3) */

	pcomp[55][0][0][0] = 2;
	pcomp[55][0][0][1] = 1;
	pcomp[55][0][1][0] = 0;
	pcomp[55][0][1][1] = 2;
	pcomp[55][1][0][0] = 1;
	pcomp[55][1][0][1] = 0;
	pcomp[55][1][1][0] = 2;
	pcomp[55][1][1][1] = 1;

	/* Case 57  (2, 3, 3) */

	pcomp[56][0][0][0] = 2;
	pcomp[56][0][0][1] = 1;
	pcomp[56][0][1][0] = 0;
	pcomp[56][0][1][1] = 2;
	pcomp[56][1][0][0] = 2;
	pcomp[56][1][0][1] = 0;
	pcomp[56][1][1][0] = 1;
	pcomp[56][1][1][1] = 1;

}	/* end set_prime_components */

EXPORT	int is_surface(
	BLK_CRX *blk_crx,
	SURFACE *s)
{
	int i;
	for (i = 0; i < blk_crx->blk_info->num_surfs; ++i)
	{
	    /*printf("#sur %d %d\n", s, blk_crx->blk_info->surfs[i]); */
	    if (s == blk_crx->blk_info->surfs[i])
	    return i;
	}
	screen("ERROR in is_surface, No surface matched\n");
	clean_up(ERROR);
}

LOCAL	int is_curve(
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
}

LOCAL	void blk_case01_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;
			
        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0] = is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

	/* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is= blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	
	/* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s);
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);

        /* the third surface 1-2 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s);
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
        p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	}
}       /* end blk_case01_comp3 */

LOCAL	void blk_case02_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3;
	SURFACE	*s;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;
        
	/* the first surface 1-2 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,1,1)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
        p3 = crx_in_jdir(blk_crx,1,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);

	/* the second surface 0-2 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
}       /* end blk_case02_comp3 */

LOCAL	void blk_case03_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3;
	SURFACE	*s;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;

	/* the first surface 1-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);
	
	/* the second surface 0-2 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
        blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
}       /* end blk_case03_comp3 */

LOCAL	void blk_case04_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_idir(blk_crx,1)->p;
	pc2 = curve_crx_in_kdir(blk_crx,1)->p;

	/* the first surface 0-2 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}

        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	else
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	
	/* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_kdir(blk_crx,0,1)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,p2,p1,pc2,s);
	    create_triangle(blk_mem,p2,pc2,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,p2,pc2,p1,s);
	    create_triangle(blk_mem,p2,p3,pc2,s);
	}
}       /* end blk_case04_comp3 */


LOCAL	void blk_case05_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

        /* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0] = is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_idir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;
	
	/* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}    

        /* the second surface 0-1 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,p2,pc1,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,p2,p1,pc1,s);
	}

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_kdir(blk_crx,0,0)->p; 
	p3 = crx_in_jdir(blk_crx,1,0)->p; 
	p4 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	}
}       /* end blk_case05_comp3 */

LOCAL	void blk_case06_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;
        
	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0] = is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

	/* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
        {
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	
	/* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	else
	    create_triangle(blk_mem,p1,pc1,pc2,s);
        
	/* the third surface 1-2 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,1)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
        {
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	}
}       /* end blk_case06_comp3 */

LOCAL	void blk_case07_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0] = is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);
        
	/* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,1,1)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
	p3 = crx_in_jdir(blk_crx,0,0)->p;
	p4 = crx_in_idir(blk_crx,1,0)->p;
	p5 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	    create_triangle(blk_mem,pc1,p3,pc2,s);
	    create_triangle(blk_mem,pc2,p3,p4,s);
	    create_triangle(blk_mem,p5,pc2,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p3,s);
	    create_triangle(blk_mem,pc2,p4,p3,s);
	    create_triangle(blk_mem,p5,p4,pc2,s);
	}
}       /* end blk_case07_comp3 */

LOCAL	void blk_case08_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;
        
	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0] = is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
        {
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	
        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	else
	    create_triangle(blk_mem,pc2,p1,pc1,s);

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	}
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
}       /* end blk_case08_comp3 */

LOCAL	void blk_case09_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3;
	SURFACE	*s;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;
        
	
        /* the first surface 0-2 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

 	/* the second surface 1-2 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,1,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
	
	p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,0)->p;
	p3 = crx_in_jdir(blk_crx,0,1)->p;
        if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
}       /* end blk_case09_comp3 */

LOCAL	void blk_case10_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
       
        FT_POINT	*p1,*p2,*p3,*p4;
	SURFACE	*s;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[2] == positive_component(s))
            create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

        /* the second surface 1-2 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
	p4 = crx_in_jdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
        {
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p2,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p4,p2,s);
	}
}       /* end blk_case10_comp3 */

LOCAL	void blk_case11_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{

        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;
        
        /* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0] =  is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

	/* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
        
	/* the second surface 0-1 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	}

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
	p3 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,p2,p3,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,p2,pc2,p3,s);
	}
}       /* end blk_case11_comp3 */

LOCAL	void blk_case12_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;

        /* the first surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,1)->p;
	p3 = crx_in_kdir(blk_crx,1,1)->p;
	p4 = crx_in_jdir(blk_crx,1,0)->p;
	p5 = crx_in_kdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[1] == positive_component(s))
        {
	    /*#bjet2  yellow: 1 pos  blue: 2 neg  */
	    /*create_triangle(blk_mem,pc2,p1,p2,s); orientation error  */
	    /*create_triangle(blk_mem,pc1,p5,p1,s); point error */
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc1,p5,p2,s);
	    create_triangle(blk_mem,p5,p3,p2,s);
	    create_triangle(blk_mem,p4,p5,pc1,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    /*#bjet2 */
	    /*create_triangle(blk_mem,pc2,p2,p1,s); */
	    /*create_triangle(blk_mem,pc1,p1,p5,s); */
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,p5,s);
	    create_triangle(blk_mem,p5,p2,p3,s);
	    create_triangle(blk_mem,p4,pc1,p5,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

	/* the second surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
            create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	}

        /* the third surface 0-2 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	else
	    create_triangle(blk_mem,p1,pc1,pc2,s);
}       /* end blk_case12_comp3 */

LOCAL	void blk_case13_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
        {
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
        
	/* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
            create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);
        
	/* the third surface 1-2 */
	s = crx_in_idir(blk_crx,1,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,0)->p;
	p3 = crx_in_kdir(blk_crx,0,1)->p;
	p4 = crx_in_idir(blk_crx,1,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
        {
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,p2,p3,p1,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,p2,p1,p3,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	}
}       /* end blk_case13_comp3 */

LOCAL	void blk_case14_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s);
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
        {
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
        
	/* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,0)->p;
	p2 = crx_in_jdir(blk_crx,1,0)->p;
	p3 = crx_in_kdir(blk_crx,1,1)->p;
	p4 = crx_in_idir(blk_crx,1,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc2,p1,p3,s);
	    create_triangle(blk_mem,p3,p1,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc2,p3,p1,s);
	    create_triangle(blk_mem,p3,p4,p1,s);
	}
}       /* end blk_case14_comp3 */

LOCAL	void blk_case15_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_kdir(blk_crx,0,1)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	}
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
}       /* end blk_case15_comp3 */


LOCAL	void blk_case16_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5,*p6;
	SURFACE	*s;
	CURVE	*c;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;

        /* the first surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	p4 = crx_in_kdir(blk_crx,1,1)->p;
	p5 = crx_in_jdir(blk_crx,1,0)->p;
	p6 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	    create_triangle(blk_mem,p1,p5,p4,s);
	    create_triangle(blk_mem,p5,p6,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	    create_triangle(blk_mem,p1,p4,p5,s);
	    create_triangle(blk_mem,p5,p4,p6,s);
	}
}       /* end blk_case16_comp3 */

LOCAL	void blk_case17_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    /*#bjet2 */
	    /*create_triangle(blk_mem,pc2,p1,pc1,s); */
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	else
	    /*#bjet2 */
	    /*create_triangle(blk_mem,pc2,pc1,p1,s); */
	    create_triangle(blk_mem,pc2,p1,pc1,s);

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,1)->p;
	p2 = crx_in_kdir(blk_crx,0,0)->p;
	p3 = crx_in_idir(blk_crx,1,0)->p;
	p4 = crx_in_idir(blk_crx,1,1)->p;
	p5 = crx_in_jdir(blk_crx,1,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    /*#bjet2 */
	    /*create_triangle(blk_mem,p2,pc1,p1,s); */
	    /*create_triangle(blk_mem,p2,p1,p3,s); */
	    /*create_triangle(blk_mem,p5,p2,p4,s); */
	    /*create_triangle(blk_mem,p2,p3,p4,s); */
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,p2,p3,p1,s);
	    create_triangle(blk_mem,p5,p4,p2,s);
	    create_triangle(blk_mem,p4,p3,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    /*#bjet2 */
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,p2,p1,p3,s);
	    create_triangle(blk_mem,p5,p2,p4,s);
	    create_triangle(blk_mem,p4,p2,p3,s);
	}
}       /* end blk_case17_comp3 */

LOCAL	void blk_case18_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);
        
	/* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,1,0)->p;
	p2 = crx_in_jdir(blk_crx,0,0)->p;
	p3 = crx_in_kdir(blk_crx,0,0)->p;
	p4 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	    create_triangle(blk_mem,pc2,p1,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	    create_triangle(blk_mem,pc2,p4,p1,s);
	}
        p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);
}       /* end blk_case18_comp3 */

LOCAL	void blk_case19_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	    	blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	    	blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	    	blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	    	blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

	/* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);
        
	/* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,0)->p;
	p2 = crx_in_jdir(blk_crx,0,1)->p;
	p3 = crx_in_kdir(blk_crx,0,1)->p;
	p4 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc1,p3,p1,s);
	    create_triangle(blk_mem,pc1,p4,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p3,s);
	    create_triangle(blk_mem,pc1,p3,p4,s);
	}
}       /* end blk_case19_comp3 */

LOCAL	void blk_case20_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_idir(blk_crx,1)->p;
	pc2 = curve_crx_in_kdir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	else
	{
            create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}

        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	else
	    create_triangle(blk_mem,pc2,pc1,p1,s);
        
	/* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);
        
        p1 = crx_in_jdir(blk_crx,0,1)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,pc2,p3,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	}
}       /* end blk_case20_comp3 */

LOCAL	void blk_case21_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	/*#bjet2 */
	/*p1 = curve_crx_in_kdir(blk_crx,1)->p; */
        /*p2 = curve_crx_in_idir(blk_crx,1)->p; */
        /*p3 = crx_in_kdir(blk_crx,1,1)->p; */
	/*p4 = crx_in_idir(blk_crx,1,1)->p; */
	p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	}
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_idir(blk_crx,1,0)->p;
	p4 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	}
}       /* end blk_case21_comp3 */

LOCAL	void blk_case22_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5;
	SURFACE	*s;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

        /* the second surface 1-2 */
	s = crx_in_jdir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,1)->p;
        p3 = crx_in_kdir(blk_crx,1,1)->p;
	p4 = crx_in_jdir(blk_crx,1,0)->p;
	p5 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	    create_triangle(blk_mem,p4,p3,p5,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	    create_triangle(blk_mem,p4,p5,p3,s);
	}
}       /* end blk_case22_comp3 */

LOCAL	void blk_case23_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5,*p6;
	SURFACE	*s;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
        p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	p4 = crx_in_kdir(blk_crx,1,1)->p;
	p5 = crx_in_idir(blk_crx,1,0)->p;
	p6 = crx_in_jdir(blk_crx,0,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	    create_triangle(blk_mem,p1,p4,p5,s);
	    create_triangle(blk_mem,p1,p5,p6,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	    create_triangle(blk_mem,p1,p5,p4,s);
	    create_triangle(blk_mem,p1,p6,p5,s);
	}
}       /* end blk_case23_comp3 */

LOCAL	void blk_case24_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is,a;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_jdir(blk_crx,1)->c;
        if(debugging("print_blk"))
        {
            int i;
            printf("blk_case24_comp3, found curve %p\n", c);
            for (i = 0; i < blk_crx->blk_info->num_curves; ++i)
                printf("blk_crx curve[%d] = %p\n", i, blk_crx->blk_info->curves[i]);
        }

	ic = blk_mem->ic[0] = is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_jdir(blk_crx,1)->p;

        /* the first surface 0-1 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
	a=positive_component(s);
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
/*	    printf("IN blk_case24_comp3, 2 tri_pts\n");
            print_general_vector("p1",Coords(p1),3,"\n");
            print_general_vector("pc2",Coords(pc2),3,"\n");
            print_general_vector("pc1",Coords(pc1),3,"\n");
            print_general_vector("p2",Coords(p2),3,"\n");
            printf("boundary flag %d %d %d %d\n",
		   Boundary_point(p1), Boundary_point(pc2),
                   Boundary_point(pc1),Boundary_point(p2));
*/

	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);

/*	    printf("IN blk_case24_comp3, 2.1 tri_pts\n");
            print_general_vector("p1",Coords(p1),3,"\n");
            print_general_vector("pc2",Coords(pc2),3,"\n");
            print_general_vector("pc1",Coords(pc1),3,"\n");
            print_general_vector("p2",Coords(p2),3,"\n");
            printf("boundary flag %d %d %d %d\n",
		   Boundary_point(p1), Boundary_point(pc2),
                   Boundary_point(pc1),Boundary_point(p2));
*/		   
	}

	/* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	}

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_kdir(blk_crx,0,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,pc1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,p2,pc1,s);
	}
}       /* end blk_case24_comp3 */
/*#bjetbond */
LOCAL	void blk_case25_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create Bonds */ /* the first curve */
	c1 = curve_crx_in_jdir(blk_crx,0)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

	/* the second curve */
	c2 = curve_crx_in_jdir(blk_crx,1)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_kdir(blk_crx,1)->p;
	pc4 = curve_crx_in_jdir(blk_crx,1)->p;
	
        /* the first surface 1-2 */
	s = crx_in_idir(blk_crx,0,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,1)->p;
        is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
        p1 = crx_in_jdir(blk_crx,1,0)->p;
	p2 = crx_in_kdir(blk_crx,0,1)->p;
        is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc3,s);
	    create_triangle(blk_mem,pc3,p2,pc4,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }

	}
	else
	{
	    create_triangle(blk_mem,p1,pc3,p2,s);
	    create_triangle(blk_mem,pc3,pc4,p2,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	    	if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}

        /* the second surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
        }
        
	/* the third surface 0-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,0,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,pc2,pc4,s);
	    create_triangle(blk_mem,pc4,pc2,p1,s);
	    create_triangle(blk_mem,pc1,pc3,p2,s);
	    create_triangle(blk_mem,pc1,pc4,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc4,pc2,s);
	    create_triangle(blk_mem,pc4,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,pc3,s);
	    create_triangle(blk_mem,pc1,pc3,pc4,s);
	}
}       /* end blk_case25_comp3 */

LOCAL	void blk_case26_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create Bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 1-2 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,0)->p;
	p2 = crx_in_jdir(blk_crx,0,1)->p;
	p3 = crx_in_kdir(blk_crx,0,1)->p;
	p4 = crx_in_idir(blk_crx,1,0)->p; 
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p3,s);
	    create_triangle(blk_mem,pc1,p3,p4,s);
	    create_triangle(blk_mem,pc2,pc1,p4,s);
	    create_triangle(blk_mem,pc2,p4,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p3,p1,s);
	    create_triangle(blk_mem,pc1,p4,p3,s);
	    create_triangle(blk_mem,pc2,p4,pc1,s);
	    create_triangle(blk_mem,pc2,p2,p4,s);
	}

        /* the third surface 0-1 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	}
}       /* end blk_case26_comp3 */

LOCAL	void blk_case27_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,1)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	p3 = crx_in_kdir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,pc2,p3,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
        
	/* the second surface 0-1 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);

        /* the third surface 1-2 */
	s = crx_in_jdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p3,p1,pc1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p3,pc1,p1,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	}
}       /* end blk_case27_comp3 */

LOCAL	void blk_case28_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,1)->p;

        /* the first surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

        p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

	/* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,0,1)->p;
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,pc1,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,p3,pc1,s);
	}

        /* the third surface 0-1 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s; 
        p1 = crx_in_idir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
            create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);
}       /* end blk_case28_comp3 */
/*#bjetbond */
LOCAL	void blk_case29_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_jdir(blk_crx,0)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_kdir(blk_crx,1)->p;
	
	/* the second curve */
	c2 = curve_crx_in_jdir(blk_crx,1)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_jdir(blk_crx,1)->p;
	pc4 = curve_crx_in_idir(blk_crx,1)->p;

        /* the first surface 1-2 */
	s = crx_in_idir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	
        p1 = crx_in_jdir(blk_crx,0,1)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c2,s);
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc4,p2,p1,s);
	    create_triangle(blk_mem,pc4,pc3,p2,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else 
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else 
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}
	else
	{
	    create_triangle(blk_mem,pc4,p1,p2,s);
	    create_triangle(blk_mem,pc4,p2,pc3,s);
	    if(ic2 != ic1)
	    {
		if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else 
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
		if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else 
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}
	
	/* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
            create_triangle(blk_mem,pc1,p1,pc4,s);
	    create_triangle(blk_mem,pc1,pc4,pc3,s);
	    create_triangle(blk_mem,pc1,pc3,pc2,s);
	    create_triangle(blk_mem,pc2,pc3,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc4,p1,s);
	    create_triangle(blk_mem,pc1,pc3,pc4,s);
	    create_triangle(blk_mem,pc1,pc2,pc3,s);
	    create_triangle(blk_mem,pc2,p2,pc3,s);
	}
	
	/* the third surface 0-1 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	}
}       /* end blk_case29_comp3 */

LOCAL	void blk_case30_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_kdir(blk_crx,1)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,p2,p3,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,p2,pc2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
        
	/* the second surface 0-1 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
            create_triangle(blk_mem,p1,pc2,pc1,s);
	else
	    create_triangle(blk_mem,p1,pc1,pc2,s);

        /* the third surface 1-2 */
	s = crx_in_jdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,1,0)->p;
	p2 = crx_in_kdir(blk_crx,0,1)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
	p4 = crx_in_jdir(blk_crx,0,0)->p;
	p5 = crx_in_kdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	    create_triangle(blk_mem,pc1,p4,p1,s);
	    create_triangle(blk_mem,pc1,p5,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,p3,p2,s);
	    create_triangle(blk_mem,pc1,p1,p4,s);
	    create_triangle(blk_mem,pc1,p4,p5,s);
	}
}       /* end blk_case30_comp3 */

LOCAL	void blk_case31_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;
  
	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_kdir(blk_crx,1)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,pc2,p3,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	
	/* the second surface 1-2 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
	p3 = crx_in_idir(blk_crx,0,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	}

        /* the third surface 0-1 */
	s = crx_in_idir(blk_crx,0,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);
}       /* end blk_case31_comp3 */

LOCAL	void blk_case32_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4;
	SURFACE	*s;
	CURVE	*c;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,1,1)->p;
        p4 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	}

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_idir(blk_crx,1,0)->p;
        p4 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	}
}       /* end blk_case32_comp3 */
/*#bjetbond */
LOCAL	void blk_case33_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_jdir(blk_crx,0)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
	/* the second curve */
	c2 = curve_crx_in_jdir(blk_crx,1)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_jdir(blk_crx,1)->p;
	/*#bjet2 */
	pc4 = curve_crx_in_idir(blk_crx,0)->p;

        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	p1 = crx_in_kdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else 
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else 
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}
	else
	{
	    /*#bjet2 */
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES) 
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else 
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES) 
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else 
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}

	/* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	}
        p1 = crx_in_jdir(blk_crx,1,0)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc4,p2,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc4,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	}

	/* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
	p2 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc4,pc1,s);
	    create_triangle(blk_mem,pc4,pc3,pc1,s);
	    create_triangle(blk_mem,pc1,pc3,pc2,s);
	    create_triangle(blk_mem,pc3,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc4,s);
	    create_triangle(blk_mem,pc4,pc1,pc3,s);
	    create_triangle(blk_mem,pc1,pc2,pc3,s);
	    create_triangle(blk_mem,pc3,pc2,p2,s);
	}
}       /* end blk_case33_comp3 */

LOCAL	void blk_case34_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;
	
        /* the first surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,1)->p;
	p3 = crx_in_kdir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	
	/* the second surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
	p2 = crx_in_jdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p1,p2,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p1,p3,p2,s);
	}

	/* the third surface 0-2 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_idir(blk_crx,0,1)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,p3,p2,s);
	}
}       /* end blk_case34_comp3 */

LOCAL	void blk_case35_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5;
	SURFACE	*s;
	CURVE	*c;
	int	is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 0;

	/* the first surface 0-2 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
        p4 = crx_in_kdir(blk_crx,1,0)->p;
	p5 = curve_crx_in_idir(blk_crx,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p3,p2,p4,s);
	    create_triangle(blk_mem,p3,p4,p5,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p3,p4,p2,s);
	    create_triangle(blk_mem,p3,p5,p4,s);
	}

        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,1,1)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
        p3 = curve_crx_in_idir(blk_crx,1)->p;
        p4 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	     create_triangle(blk_mem,p1,p2,p3,s);
	     create_triangle(blk_mem,p1,p3,p4,s);
	}
	else
	{
	     create_triangle(blk_mem,p1,p3,p2,s);
	     create_triangle(blk_mem,p1,p4,p3,s);
	}

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = curve_crx_in_idir(blk_crx,1)->p;
        p3 = crx_in_jdir(blk_crx,0,0)->p;
	p4 = crx_in_kdir(blk_crx,1,1)->p;
	p5 = crx_in_idir(blk_crx,1,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p2,p3,p4,s);
	    create_triangle(blk_mem,p3,p5,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p2,p4,p3,s);
	    create_triangle(blk_mem,p3,p4,p5,s);
	}
}       /* end blk_case35_comp3 */

LOCAL	void blk_case36_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
	p4 = crx_in_jdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	    create_triangle(blk_mem,p2,pc1,p3,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	    create_triangle(blk_mem,p2,p3,pc1,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

	/* the third surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	}
}       /* end blk_case36_comp3 */
/*#bjetbond */
LOCAL	void blk_case37_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_idir(blk_crx,1)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_idir(blk_crx,1)->p;
	pc2 = curve_crx_in_kdir(blk_crx,1)->p;
	
	/* the second curve */
	c2 = curve_crx_in_idir(blk_crx,0)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_idir(blk_crx,0)->p;
	pc4 = curve_crx_in_kdir(blk_crx,0)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
        p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc3,p2,pc4,s);
	    create_triangle(blk_mem,pc4,p2,p1,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}
	else
	{
	    create_triangle(blk_mem,pc3,pc4,p2,s);
	    create_triangle(blk_mem,pc4,p1,p2,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}

	/* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,1,1)->p;
	p2 = crx_in_jdir(blk_crx,0,0)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	}

	/* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	    create_triangle(blk_mem,p2,p1,pc1,s);
	}
	else
	{
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	    create_triangle(blk_mem,p2,pc1,p1,s);
	}
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc4,s);
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,p2,s);
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	}
}       /* end blk_case37_comp3 */
/*#bjetbond */
LOCAL	void blk_case38_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_idir(blk_crx,1)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_idir(blk_crx,1)->p;
	pc2 = curve_crx_in_kdir(blk_crx,0)->p;
	
	/* the second curve */
	c2 = curve_crx_in_idir(blk_crx,0)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_idir(blk_crx,0)->p;
	pc4 = curve_crx_in_kdir(blk_crx,1)->p;
	
	/* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,1,0)->p;
	p2 = crx_in_idir(blk_crx,0,0)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
		blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
		blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
		blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
		blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,0,0)->p;
        is_pos_c = is_positive_curve(c2,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p2,pc3,p1,s);
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}
	else
	{
	    create_triangle(blk_mem,p2,p1,pc3,s);
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}

	/* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	    create_triangle(blk_mem,p1,pc4,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	    create_triangle(blk_mem,p1,p2,pc4,s);
	}
        p1 = crx_in_idir(blk_crx,1,0)->p;
	p2 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	}

        /* the third surface 0-1 */
	s = crx_in_jdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
	p2 = crx_in_jdir(blk_crx,0,0)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc4,s);
	    create_triangle(blk_mem,p2,pc4,pc2,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,pc4,pc2,s);
	    create_triangle(blk_mem,p2,pc2,pc4,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	}
}       /* end blk_case38_comp3 */

LOCAL	void blk_case39_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4;
	SURFACE	*s;
	int	is;

        blk_mem->num_surfaces = 2;
	blk_mem->num_curves = 0;

	/* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
        p3 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);

        p1 = crx_in_idir(blk_crx,1,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_kdir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
            create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);

        p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
        if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);
}       /* end blk_case39_comp3 */


LOCAL	void blk_case40_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_idir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,1)->p;
	
        /* the first surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_idir(blk_crx,1,1)->p;
	p3 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,pc2,p3,s);
	    create_triangle(blk_mem,pc2,pc1,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,p3,pc2,s);
	    create_triangle(blk_mem,pc2,p3,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_kdir(blk_crx,0,0)->p;
        p3 = crx_in_kdir(blk_crx,0,1)->p;
	p4 = crx_in_jdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,pc1,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,pc2,p3,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,p4,pc1,s);
	}
	    
	/* the third surface 0-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
        if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	else
	    create_triangle(blk_mem,pc2,p1,pc1,s);
}       /* end blk_case40_comp3 */

LOCAL	void blk_case41_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
        /* the first surface 0-1 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_idir(blk_crx,1,1)->p;
	p3 = crx_in_kdir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p2,p3,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p2,pc2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	else
	    create_triangle(blk_mem,pc1,pc2,p1,s);

        /* the third surface 1-2 */
	s = crx_in_idir(blk_crx,1,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
	p3 = crx_in_kdir(blk_crx,0,0)->p;
	p4 = crx_in_jdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p1,pc1,p3,s);
	    create_triangle(blk_mem,p2,p4,pc2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p1,p3,pc1,s);
	    create_triangle(blk_mem,p2,pc2,p4,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	}
}       /* end blk_case41_comp3 */

LOCAL	void blk_case42_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
        pc2 = curve_crx_in_jdir(blk_crx,1)->p;
	
        /* the first surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
	p3 = crx_in_idir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,p2,p3,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,p2,pc2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
        
	/* the second surface 0-1 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	}

        /* the third surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,p1,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,p1,pc2,p2,s);
	}
}       /* end blk_case42_comp3 */

LOCAL	void blk_case43_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_jdir(blk_crx,1)->p;
	
        /* the first surface 0-1 */
	s = crx_in_idir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	}

        /* the third surface 1-2 */
	s = crx_in_idir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,0)->p;
	p2 = crx_in_jdir(blk_crx,0,0)->p;
	p3 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,p2,pc2,p1,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,p2,p1,pc2,s);
	    create_triangle(blk_mem,pc2,p3,p2,s);
	}
}       /* end blk_case43_comp3 */

/*#bjetbond */
LOCAL	void blk_case44_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_jdir(blk_crx,0)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
	/* the second curve */
	c2 = curve_crx_in_jdir(blk_crx,1)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_jdir(blk_crx,1)->p;
	pc4 = curve_crx_in_kdir(blk_crx,1)->p;
	
        /* the first surface 1-2 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_idir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc4,s);
	    create_triangle(blk_mem,p2,p3,pc4,s);
	    create_triangle(blk_mem,p3,pc3,pc4,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,p2,s);
	    create_triangle(blk_mem,p2,pc4,p3,s);
	    create_triangle(blk_mem,p3,pc4,pc3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}
        p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}

        /* the second surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,1,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	}
	    
        /* the third surface 0-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,pc4,s);
	    create_triangle(blk_mem,pc1,pc3,pc4,s);
	    create_triangle(blk_mem,pc1,pc2,pc3,s);
	    create_triangle(blk_mem,pc2,p2,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,pc1,s);
	    create_triangle(blk_mem,pc1,pc4,pc3,s);
	    create_triangle(blk_mem,pc1,pc3,pc2,s);
	    create_triangle(blk_mem,pc2,pc3,p2,s);
	}
}       /* end blk_case44_comp3 */

LOCAL	void blk_case45_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
        /* the first surface 1-2 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
	p3 = crx_in_jdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,pc2,p3,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	
        /* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	}
	
        /* the third surface 0-1 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	}
}       /* end blk_case45_comp3 */


LOCAL	void blk_case46_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,1)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_kdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
	/*  the first surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_kdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

        p1 = crx_in_idir(blk_crx,1,0)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
	p4 = crx_in_jdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,p1,p4,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p4,p1,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	    /*#bjet2 */
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	}

        /* the third surface 0-1 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc1,p2,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc1,p1,p2,s);
	}
}       /* end blk_case46_comp3 */

LOCAL	void blk_case47_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_idir(blk_crx,0)->p;
	pc2 = curve_crx_in_jdir(blk_crx,1)->p;
	
	/* the first surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
        p3 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,p1,p2,p3,s);
	else
	    create_triangle(blk_mem,p1,p3,p2,s);

        p1 = crx_in_jdir(blk_crx,1,0)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}

        /* the second surface 0-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	else
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    
        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
	p3 = crx_in_jdir(blk_crx,0,1)->p;
	p4 = crx_in_kdir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p2,p1,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    create_triangle(blk_mem,p2,pc2,p3,s);
	    create_triangle(blk_mem,pc2,p4,p3,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,p1,p2,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    create_triangle(blk_mem,p2,p3,pc2,s);
	    create_triangle(blk_mem,pc2,p3,p4,s);
	}
}       /* end blk_case47_comp3 */
/*#bjetbond */
LOCAL	void blk_case48_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_idir(blk_crx,0)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_idir(blk_crx,0)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;
	
	/* the second curve */
	c2 = curve_crx_in_idir(blk_crx,1)->c;
	ic2 = blk_mem->ic[1]=is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_idir(blk_crx,1)->p;
	pc4 = curve_crx_in_jdir(blk_crx,1)->p;
	
        /* the first surface 0-1 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	} 
	p1 = crx_in_idir(blk_crx,1,1)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc4,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }

	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,p2,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	else
	    create_triangle(blk_mem,p1,pc2,pc1,s);

        p1 = crx_in_idir(blk_crx,1,0)->p;
	p2 = crx_in_jdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	    create_triangle(blk_mem,p1,p2,pc3,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	    create_triangle(blk_mem,p1,pc3,p2,s);
	}

        /* the third surface 0-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,pc2,pc4,s);
	    create_triangle(blk_mem,p2,pc4,pc2,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,pc4,pc2,s);
	    create_triangle(blk_mem,p2,pc2,pc4,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	}
}       /* end blk_case48_comp3 */

LOCAL	void blk_case49_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
        p3 = crx_in_jdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
	p2 = crx_in_jdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,p3,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,p3,s);
	}

        /* the third surface 1-2 */
	s = crx_in_idir(blk_crx,1,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	}
}       /* end blk_case49_comp3 */

LOCAL	void blk_case50_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_idir(blk_crx,0)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;
	
        /* the first surface 0-2 */
	s = crx_in_jdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,1,0)->p;
        p2 = crx_in_idir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,pc1,pc2,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 1-2 */
	s = crx_in_idir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,0)->p;
	p2 = crx_in_idir(blk_crx,0,0)->p;
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc1,p1,pc2,s);
	    create_triangle(blk_mem,p1,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc1,pc2,p1,s);
	    create_triangle(blk_mem,p1,pc2,p2,s);
	}
        p1 = crx_in_kdir(blk_crx,1,1)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);
	
	/* the third surface 0-1 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_kdir(blk_crx,1,0)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	p4 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,p2,pc2,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	    create_triangle(blk_mem,p1,p4,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,pc2,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	    create_triangle(blk_mem,p1,p2,p4,s);
	}
}       /* end blk_case50_comp3 */

LOCAL	void blk_case51_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_jdir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_jdir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
        if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	    create_triangle(blk_mem,p1,p2,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	    create_triangle(blk_mem,p1,p3,p2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
        p4 = crx_in_kdir(blk_crx,1,1)->p;
  	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc1,p4,p3,s);
	    create_triangle(blk_mem,p2,pc1,p3,s);
	    create_triangle(blk_mem,pc2,pc1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc1,p3,p4,s);
	    create_triangle(blk_mem,p2,p3,pc1,s);
	    create_triangle(blk_mem,pc2,p2,pc1,s);
	}

        /* the third surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,p3,p2,pc1,s);
	    create_triangle(blk_mem,pc2,p3,pc1,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,p3,pc1,p2,s);
	    create_triangle(blk_mem,pc2,pc1,p3,s);
	}
 }      /* end blk_case51_comp3 */

LOCAL	void blk_case52_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2;
	SURFACE	*s;
	CURVE	*c;
	int	ic,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 1;

	/* Create bonds */
	c = curve_crx_in_idir(blk_crx,0)->c;
	ic = blk_mem->ic[0]= is_curve(blk_crx,c);
	blk_mem->curves[ic] = c;
	pc1 = curve_crx_in_idir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,p1,pc1,p3,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p2,p1,s);
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,p1,p3,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic] = Bond(pc2,pc1);
	    else if (is_pos_c == NO)
	        blk_mem->bonds[ic] = Bond(pc1,pc2);
	}

        /* the second surface 0-1 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	p3 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	    create_triangle(blk_mem,pc2,p2,p3,s);
	}
	else
	{
	   create_triangle(blk_mem,pc2,p1,pc1,s);
	   create_triangle(blk_mem,pc2,p2,p1,s);
	   create_triangle(blk_mem,pc2,p3,p2,s);
	}

        /* the third surface 1-2 */
	s = crx_in_jdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 6;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,0)->p;
	p2 = crx_in_jdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc1,p1,s);
	    create_triangle(blk_mem,pc2,p1,p2,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc1,s);
	    create_triangle(blk_mem,pc2,p2,p1,s);
	}
}       /* end blk_case52_comp3 */
/*#bjetbond */
LOCAL	void blk_case53_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_jdir(blk_crx,1)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,1)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
	/* the second curve */
	c2 = curve_crx_in_idir(blk_crx,0)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_idir(blk_crx,0)->p;
	pc4 = curve_crx_in_jdir(blk_crx,0)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,0)->p;
	p3 = crx_in_kdir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p2,p1,pc3,s);
	    create_triangle(blk_mem,pc1,p2,pc3,s);
	    create_triangle(blk_mem,pc1,pc3,pc4,s);
	    create_triangle(blk_mem,pc1,pc4,p3,s);
	    create_triangle(blk_mem,pc1,p3,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p2,pc3,p1,s);
	    create_triangle(blk_mem,pc1,pc3,p2,s);
	    create_triangle(blk_mem,pc1,pc4,pc3,s);
	    create_triangle(blk_mem,pc1,p3,pc4,s);
	    create_triangle(blk_mem,pc1,pc2,p3,s);
	}

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
	p3 = crx_in_idir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc3,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	    create_triangle(blk_mem,pc4,p3,pc2,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}
	else
	{
	    create_triangle(blk_mem,p1,pc3,p2,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	    create_triangle(blk_mem,pc4,pc2,p3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}
        p1 = crx_in_kdir(blk_crx,1,1)->p;
	is_pos_c = is_positive_curve(c1,s);
        if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
        /* the third surface 0-1 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	    create_triangle(blk_mem,p1,pc2,pc4,s);
	    create_triangle(blk_mem,p1,p3,pc1,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p3,p2,pc2,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	    create_triangle(blk_mem,p1,pc4,pc2,s);
	    create_triangle(blk_mem,p1,pc1,p3,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p3,pc2,p2,s);
	}
}       /* end blk_case53_comp3 */
/*#bjetbond */
LOCAL	void blk_case54_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_jdir(blk_crx,0)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_idir(blk_crx,1)->p;
	
	/* the second curve */
	c2 = curve_crx_in_idir(blk_crx,0)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_idir(blk_crx,0)->p;
	pc4 = curve_crx_in_jdir(blk_crx,1)->p;
	
        /* the first surface 0-2 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
        p1 = crx_in_jdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc3,p2,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc3,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}

        /* the second surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p2,pc2,pc1,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p2,pc1,pc2,s);
	}
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	else
	    create_triangle(blk_mem,p1,pc4,pc3,s);

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
	p2 = crx_in_kdir(blk_crx,1,1)->p;
	
	if (blk_crx->comps[1] == positive_component(s))
	{
	    /*#bjet2  */
            /*create_triangle(blk_mem,pc3,pc4,p1,s); */
            create_triangle(blk_mem,pc3,pc1,p1,s);
	    create_triangle(blk_mem,pc3,pc4,pc2,s);
	    create_triangle(blk_mem,pc3,pc2,pc1,s);
	    create_triangle(blk_mem,pc4,p2,pc2,s);
	}
	else
	{
	    /*#bjet2  */
	    /*create_triangle(blk_mem,pc3,p1,pc4,s); */
	    create_triangle(blk_mem,pc3,p1,pc1,s);
	    create_triangle(blk_mem,pc3,pc2,pc4,s);
	    create_triangle(blk_mem,pc3,pc1,pc2,s);
	    create_triangle(blk_mem,pc4,pc2,p2,s);
	}

}       /* end blk_case54_comp3 */

/*#bjetbond */
LOCAL	void blk_case55_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_jdir(blk_crx,0)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,0)->p;
	pc2 = curve_crx_in_kdir(blk_crx,1)->p;
	
	/* the second curve */
	c2 = curve_crx_in_idir(blk_crx,0)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_idir(blk_crx,0)->p;
	pc4 = curve_crx_in_jdir(blk_crx,1)->p;
	
        /* the first surface 0-1 */
	s = crx_in_jdir(blk_crx,1,1)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p2,pc1,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,p2,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
        p1 = crx_in_kdir(blk_crx,0,1)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc3,pc4,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	    }
	}
	else
	{
	    create_triangle(blk_mem,p1,pc4,pc3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds[ic2] = Bond(pc4,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc4);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc4,pc3);
	    }
	}
        /* the second surface 0-2 */
	s = crx_in_idir(blk_crx,1,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
        p1 = crx_in_idir(blk_crx,0,1)->p;
	if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	else
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	
        p1 = crx_in_jdir(blk_crx,0,0)->p;
	p2 = crx_in_idir(blk_crx,1,0)->p;
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc3,p2,s);
	    create_triangle(blk_mem,p2,pc3,pc4,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,pc3,s);
	    create_triangle(blk_mem,p2,pc4,pc3,s);
	}

        /* the third surface 1-2 */
	s = crx_in_kdir(blk_crx,1,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 18;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,1,0)->p;
	p2 = crx_in_kdir(blk_crx,1,1)->p;
	p3 = crx_in_idir(blk_crx,0,0)->p;
	p4 = crx_in_jdir(blk_crx,0,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,pc2,pc3,p1,s);
	    create_triangle(blk_mem,pc3,pc2,pc1,s);
	    create_triangle(blk_mem,pc1,pc2,p4,s);
	    create_triangle(blk_mem,pc2,pc4,p4,s);
	    create_triangle(blk_mem,pc2,p2,pc4,s);
	    create_triangle(blk_mem,pc4,p2,p4,s);
	}
	else
	{
	    create_triangle(blk_mem,pc2,p1,pc3,s);
	    create_triangle(blk_mem,pc3,pc1,pc2,s);
	    create_triangle(blk_mem,pc1,p4,pc2,s);
	    create_triangle(blk_mem,pc2,p4,pc4,s);
	    create_triangle(blk_mem,pc2,pc4,p2,s);
	    create_triangle(blk_mem,pc4,p4,p2,s);
	}
}       /* end blk_case55_comp3 */

LOCAL	void blk_case56_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*p5,*p6;
	SURFACE	*s;
	int	ic,is;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 0;

        /* the first surface 0-1 */
	s = crx_in_kdir(blk_crx,1,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,0,1)->p;
        p2 = crx_in_kdir(blk_crx,1,0)->p;
        p3 = crx_in_jdir(blk_crx,1,1)->p;
        if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
        else
	    create_triangle(blk_mem,p1,p2,p3,s);

        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 12;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,0)->p;
        p2 = crx_in_idir(blk_crx,0,0)->p;
        p3 = crx_in_jdir(blk_crx,0,1)->p;
	p4 = crx_in_kdir(blk_crx,1,1)->p;
	p5 = crx_in_jdir(blk_crx,1,0)->p;
	p6 = crx_in_idir(blk_crx,1,1)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,p3,p2,s);
	    create_triangle(blk_mem,p1,p4,p3,s);
	    create_triangle(blk_mem,p1,p6,p4,s);
	    create_triangle(blk_mem,p1,p5,p6,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,p2,p3,s);
	    create_triangle(blk_mem,p1,p3,p4,s);
	    create_triangle(blk_mem,p1,p4,p6,s);
	    create_triangle(blk_mem,p1,p6,p5,s);
	}

        /* the third surface 0-2 */
	s = crx_in_kdir(blk_crx,0,1)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 3;
	blk_mem->surfs[is] = s;
	p1 = crx_in_kdir(blk_crx,0,1)->p;
        p2 = crx_in_jdir(blk_crx,0,0)->p;
        p3 = crx_in_idir(blk_crx,1,0)->p;
        if (blk_crx->comps[2] == positive_component(s))
	    create_triangle(blk_mem,p1,p3,p2,s);
	else
	    create_triangle(blk_mem,p1,p2,p3,s);
}       /* end blk_case56_comp3 */
/*#bjetbond */
LOCAL	void blk_case57_comp3(
        BLK_CRX *blk_crx,
        BLK_TRI *blk_mem)
{
        FT_POINT	*p1,*p2,*p3,*p4,*pc1,*pc2,*pc3,*pc4;
	SURFACE	*s;
	CURVE	*c1,*c2;
	int	ic1,ic2,is;
	int	is_pos_c;

        blk_mem->num_surfaces = 3;
	blk_mem->num_curves = 2;

	/* Create bonds */
	c1 = curve_crx_in_jdir(blk_crx,1)->c;
	ic1 = blk_mem->ic[0]= is_curve(blk_crx,c1);
	blk_mem->curves[ic1] = c1;
	pc1 = curve_crx_in_jdir(blk_crx,1)->p;
	pc2 = curve_crx_in_jdir(blk_crx,0)->p;
	
	/* the second curve */
	c2 = curve_crx_in_idir(blk_crx,1)->c;
	ic2 = blk_mem->ic[1]= is_curve(blk_crx,c2);
	blk_mem->curves[ic2] = c2;
	pc3 = curve_crx_in_idir(blk_crx,1)->p;
	pc4 = curve_crx_in_kdir(blk_crx,0)->p;
	
        /* the first surface 0-2 */
	s = crx_in_jdir(blk_crx,0,0)->s;
	is = blk_mem->is[0]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_jdir(blk_crx,0,0)->p;
        p2 = crx_in_kdir(blk_crx,0,1)->p;
        is_pos_c = is_positive_curve(c1,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p2,p1,pc1,s);
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	    else
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	}
	else
	{
	    create_triangle(blk_mem,p2,pc1,p1,s);
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    if (is_pos_c == YES)
	        blk_mem->bonds[ic1] = Bond(pc2,pc1);
	    else
	        blk_mem->bonds[ic1] = Bond(pc1,pc2);
	}

        p1 = crx_in_kdir(blk_crx,1,0)->p;
	is_pos_c = is_positive_curve(c2,s);
	if (blk_crx->comps[2] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc3,pc1,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc1,pc3);
	        else
	            blk_mem->bonds[ic2] = Bond(pc3,pc1);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc1,pc3);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc3,pc1);
	    }
	}
	else
	{
	    create_triangle(blk_mem,p1,pc1,pc3,s);
	    if(ic2 != ic1)
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds[ic2] = Bond(pc3,pc1);
	        else
	            blk_mem->bonds[ic2] = Bond(pc1,pc3);
	    }
	    else
	    {
	        if (is_pos_c == YES)
	            blk_mem->bonds1[ic2] = Bond(pc3,pc1);
	        else
	            blk_mem->bonds1[ic2] = Bond(pc1,pc3);
	    }
	}
	
        /* the second surface 1-2 */
	s = crx_in_kdir(blk_crx,0,0)->s;
	is = blk_mem->is[1]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 15;
	blk_mem->surfs[is] = s;
        p1 = crx_in_jdir(blk_crx,0,1)->p;
	p2 = crx_in_idir(blk_crx,1,1)->p;
	p3 = crx_in_jdir(blk_crx,1,0)->p;
        p4 = crx_in_kdir(blk_crx,0,0)->p;
	if (blk_crx->comps[1] == positive_component(s))
	{
	    create_triangle(blk_mem,p4,p3,pc2,s);
	    create_triangle(blk_mem,p3,pc1,pc2,s);
	    create_triangle(blk_mem,p3,p2,pc1,s);
	    create_triangle(blk_mem,pc1,p2,pc3,s);
	    create_triangle(blk_mem,pc1,pc3,p1,s);
	}
	else
	{
	    create_triangle(blk_mem,p4,pc2,p3,s);
	    create_triangle(blk_mem,p3,pc2,pc1,s);
	    create_triangle(blk_mem,p3,pc1,p2,s);
	    create_triangle(blk_mem,pc1,pc3,p2,s);
	    create_triangle(blk_mem,pc1,p1,pc3,s);
	}

        /* the third surface 0-1 */
	s = crx_in_idir(blk_crx,1,0)->s;
	is = blk_mem->is[2]= is_surface(blk_crx,s); 
	blk_mem->num_null_sides[is] = 9;
	blk_mem->surfs[is] = s;
	p1 = crx_in_idir(blk_crx,1,0)->p;
	if (blk_crx->comps[0] == positive_component(s))
	    create_triangle(blk_mem,p1,pc1,pc3,s);
	else
	    create_triangle(blk_mem,p1,pc3,pc1,s);
	
        p1 = crx_in_idir(blk_crx,0,1)->p;
	p2 = crx_in_jdir(blk_crx,1,1)->p;
	if (blk_crx->comps[0] == positive_component(s))
	{
	    create_triangle(blk_mem,p1,pc1,pc2,s);
	    create_triangle(blk_mem,p1,p2,pc1,s);
	}
	else
	{
	    create_triangle(blk_mem,p1,pc2,pc1,s);
	    create_triangle(blk_mem,p1,pc1,p2,s);
	}
}       /* end blk_case57_comp3 */

LOCAL	int count_side_comp(
	COMPONENT c1,
	COMPONENT c2,
	COMPONENT c3,
	COMPONENT c4)
{
	int nc = 1;
	if (c1 != c2)
	{
	    ++nc;
	    if (c3 != c1 && c3 != c2)
	    {
	        ++nc;
		if (c4 != c1 && c4 != c2 && c4 != c3)
		    ++nc;
	    }
	    else if (c4 != c1 && c4 != c2)
	        ++nc;
	}
	else if (c2 != c3)
	{
	    ++nc;
	    if (c4 != c2 && c4 != c3)
	        ++nc;
	}
	return nc;
}	/* end count_side_comp */


LOCAL	int check_consistence_of_crx(
	BLK_CRX *blk_crx,
	boolean include_curve_crx)
{
	int i,j,k,num_crx;
	BBI_POINT *crxs[18];
	COMPONENT ***comp = blk_crx->comp;
	
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
		        screen("ERROR: in check_consistence_of_crx for "
			       "crx_in_x_direction\n");
			clean_up(ERROR);
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
		        screen("ERROR: in check_consistence_of_crx for "
			       "crx_in_y_direction\n");
			clean_up(ERROR);
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
		        screen("ERROR: in check_consistence_of_crx for "
			       "crx_in_z_direction\n");
			clean_up(ERROR);
		    }
	            ++num_crx;
	        }
	    }
	}
	if (! include_curve_crx)
	    return num_crx;
	    
	for (i = 0; i < 2; ++i)
	{
	    if (is_curve_crx(comp[i][0][0],comp[i][1][0],
	                     comp[i][0][1],comp[i][1][1]))
	    {
	        crxs[num_crx] = curve_crx_in_idir(blk_crx,i);
		if (crxs[num_crx] == NULL)
		{
		    screen("ERROR: in check_consistence_of_crx for "
		           "curve_crx_in_x_direction\n");
	 	    clean_up(ERROR);
		}
		++num_crx;
	    }
	    if (is_curve_crx(comp[0][i][0],comp[1][i][0],
	                     comp[0][i][1],comp[1][i][1]))
	    {
	        crxs[num_crx] = curve_crx_in_jdir(blk_crx,i);
		if (crxs[num_crx] == NULL)
		{
		    screen("ERROR: in check_consistence_of_crx for "
		           "curve_crx_in_y_direction\n");
	 	    clean_up(ERROR);
		}
		++num_crx;
	    }
	    if (is_curve_crx(comp[0][0][i],comp[1][0][i],
	                     comp[0][1][i],comp[1][1][i]))
	    {
	        crxs[num_crx] = curve_crx_in_kdir(blk_crx,i);
		if (crxs[num_crx] == NULL)
		{
		    screen("ERROR: in check_consistence_of_crx for "
		           "curve_crx_in_z_direction\n");
	 	    clean_up(ERROR);
		}
		++num_crx;
	    }
	}
	return num_crx;
}
LOCAL	int is_positive_curve(
	CURVE *curve,
	SURFACE *surf)
{
	CURVE **c;

	for (c = surf->pos_curves; c && *c; ++c)
	    if (curve == *c) return YES;
	
	for (c = surf->neg_curves; c && *c; ++c)
	    if (curve == *c) return NO;
}	/* end is_positive_curve */


#endif /* defined(THREED) */
