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
*                               tricrx.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#if defined(THREED)

#define DEBUG_STRING "crx_intfc"

#include <front/fdecs.h>

struct _CRX_SORT {
  	double *compare_coord;
  	CRXING *crx;
};
typedef struct _CRX_SORT CRX_SORT;

enum _EDGE_TYPE {
  	UNPHYSICAL_EDGE = 0,
	PHYSICAL_EDGE = 1
};
typedef enum _EDGE_TYPE EDGE_TYPE;

struct _GRID_PT {
	struct _GRID_PT	*prev;
	int		ip[MAXD];
	COMPONENT	comp;
	GRID_DIRECTION	cur_dir;
};
typedef struct _GRID_PT GRID_PT;

/*set and check components */
LOCAL	COMPONENT	next_side_comp_at_crx(GRID_PT*,CRXING*);
LOCAL	COMPONENT	this_side_comp_at_crx(GRID_PT*,CRXING*);
LOCAL	EDGE_TYPE	edge_comp_walk(GRID_PT*,INTERFACE*,COMPONENT*,CRX_TYPE);
LOCAL	boolean	check_comp_at(int*,GRID_DIRECTION,INTERFACE*,int*,int*,boolean);
LOCAL	int	walk_comp_along_grid_line(INTERFACE*,int*,int*,int*,int*,
					  GRID_DIRECTION);
LOCAL	int 	check_and_unset_bad_comp(int*,int*,INTERFACE*);
LOCAL	boolean 	unset_comp_exist(int*,int*,INTERFACE*);
LOCAL	boolean	physical_edge(int*,GRID_DIRECTION,INTERFACE*,int*,int*,
			CRX_TYPE);

/*deal with edge crxings */
LOCAL	int	crossings_on_edge(double**,double*,double*,int,int);
LOCAL	void	eliminate_same_crossings(int*,int*,INTERFACE*);
LOCAL 	int	fill_missing_crx(INTERFACE*,int*,int*,GRID_DIRECTION,int);
LOCAL	void	adjust_for_min_spacing(CRXING*,double,double*,int,int);
LOCAL	void	multi_crx_debug(CRX_SORT*,int,GRID_PT*,double*,double*,int);
LOCAL	void	print_crx_sort(CRX_SORT*,int,GRID_PT*);
LOCAL	void	print_edge_crxings(GRID_PT*,INTERFACE*);
LOCAL	void	set_grid_crx_edge(RECT_GRID*,int*,GRID_DIRECTION,
				  double*,double*,int*,int*);
LOCAL	int	rm_unphy_crx_along_grid_line(INTERFACE*,int*,int*,int*,int*,
					     GRID_DIRECTION,CRX_TYPE);

/*insert grid crxings */
LOCAL   int     count_block_crossings(RECT_GRID*,int*,TRI**,int,int*);
LOCAL   int     add_to_edge_list(TRI*,int,CRX_STORE*,int*,double*,int,int);
LOCAL   void    add_to_crx_list(int*,int,INTERFACE*,TRI*,SURFACE*,CRXING*,
                                CRX_STORE*,int*,int*,double*,int,int);
LOCAL   void    insert_block_crossings(INTERFACE*,RECT_GRID*,CRXING*,int**,
                                int*,TRI**,SURFACE**,int,int*,int*);
LOCAL   void    linear_interp_coefs_3d_tri(double*,double*,TRI*);
LOCAL   boolean    set_comp_at_vertex(CRXING*,FT_POINT*,TRI*,SURFACE*,int);

/*grid based reconstruction */
LOCAL	void	 fill_block_crx(int,int,int,BLK_CRX*,int*,
			       GRID_DIRECTION,INTERFACE*);
LOCAL	void	 fill_block_curve_crx(int,int,BLK_CRX*,int*,
			       GRID_DIRECTION,INTERFACE*);
LOCAL	void	 prt_block_crx(INTERFACE *);
LOCAL   boolean     is_crx(int*,int*,int,int,int);
LOCAL   void     install_curve_points_state(INTERFACE *);
LOCAL   boolean     install_btri_states_from_crx(INTERFACE*,BOND_TRI*,CRXING*,size_t,ORIENTATION);

LOCAL   void   check_curve_connect(CURVE*, SURFACE*);
void   print_edge_crossings(int *, int*, INTERFACE *);

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */
LOCAL   int     crx_ascend(const void*,const void*);
LOCAL	int     crx_descend(const void *,const void *);
#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */


EXPORT  void    set_expanded_grid(
        RECT_GRID       *dual_grid,
        RECT_GRID       *expanded_grid)
{
        int       gmax[3];
        int       i, dim = dual_grid->dim;

        for (i = 0; i < dim; ++i)
            gmax[i] = dual_grid->gmax[i]+dual_grid->lbuf[i]+dual_grid->ubuf[i];

        set_rect_grid(dual_grid->VL,dual_grid->VU,dual_grid->GL,dual_grid->GU,
                  NOBUF,NOBUF,gmax,dim,&dual_grid->Remap,expanded_grid);

}       /*end set_expanded_grid*/

EXPORT 	boolean reconstruct_intfc3d_in_box_lgb(
	INTERFACE  *intfc,
	int *smin,
	int *smax,
	boolean full_reconst,
	VOLUME_FRAC *volume_frac)
{
	RECT_GRID	gr = topological_grid(intfc);
	int		*gmax = gr.gmax;
	Table		*T = table_of_interface(intfc);
	BLK_TRI		****blk_mem, *bm, *blk_mem_store;
	COMPONENT	***compon;
	COMPONENT	c;
	COMPONENT	*comp = T->components;
	int		ix, iy, iz, ixx, iyy, izz;
	int             i, j, k, ic, nbc, ip[3];
	static SURFACE	**s;
	static BLK_CRX	*blk_crx;
	BLK_INFO        blk_info;
	int       	n_fr_blk = 0;
	boolean		do_volume_frac = YES;
	double		*L = gr.L;
	double		*h = gr.h;
	double		***area,***vol_frac;

	DEBUG_ENTER(reconstruct_intfc3d_in_box_lgb)

	blk_info.num_surfs = 0;
        for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
            ++blk_info.num_surfs;

	compon = (table_of_interface(intfc))->compon3d;
	for (iz = smin[2]; iz < smax[2]; ++iz)
	    for (iy = smin[1]; iy < smax[1]; ++iy)
	        for (ix = smin[0]; ix < smax[0]; ++ix)
		    if (compon[iz][iy][ix] == ONFRONT)
		    	n_fr_blk++;


	uni_array(&blk_info.surfs,blk_info.num_surfs,sizeof(SURFACE*));
	uni_array(&blk_info.cur_tris,blk_info.num_surfs,sizeof(TRI*));
	for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
        {
            blk_info.surfs[i] = *s;
	    if (full_reconst || volume_frac != NULL)
	    {
            	(*s)->num_tri = 0;
		blk_info.cur_tris[i] = NULL;
	    }
	    else
	    	blk_info.cur_tris[i] = last_tri(*s);
        }

	if (blk_crx == NULL)
	    blk_crx = alloc_blk_crx(YES);
	if (volume_frac != NULL)
	{
	    blk_info.do_volume_frac = YES;
	    blk_crx->cell_volume = 1.0;
	    blk_crx->comp_vfrac = volume_frac->comp_vfrac;
	    for (i = 0; i < 3; ++i)
	    {
	    	blk_crx->cell_volume *= h[i];
		blk_crx->h[i] = h[i];
	    }
	    area = T->area;
	    vol_frac = T->vol_frac;
	}
	else
	    blk_info.do_volume_frac = NO;

	tri_array(&blk_mem,smax[2]-smin[2],smax[1]-smin[1],smax[0]-smin[0],
	          sizeof(BLK_TRI*));
	uni_array(&blk_mem_store,n_fr_blk,sizeof(BLK_TRI));

	nbc = 0;
	for (iz = smin[2]; iz < smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy < smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix < smax[0]; ++ix)
	        {
	            ixx = ix - smin[0];
	            iyy = iy - smin[1];
	            izz = iz - smin[2];
	            if (compon[iz][iy][ix] == ONFRONT)
	            {
	                bm = blk_mem[izz][iyy][ixx] = &blk_mem_store[nbc++];
	                bm->blk_info = blk_crx->blk_info = &blk_info;
			blk_crx->num_comps = 0;
	                for (i = 0; i < 2; ++i)
	                {
	                    for (j = 0; j < 2; ++j)
	                    {
	                        for (k = 0; k < 2; ++k)
	                        {
	                            c = comp[d_index3d(ix+i,iy+j,iz+k,gmax)];
				    for (ic = 0; ic < blk_crx->num_comps; ++ic)
				    {
				    	if (c == blk_crx->comps[ic])
		                        {
				            ++blk_crx->nv[ic];
				            break;
				        }
				    }
				    if (ic == blk_crx->num_comps)
				    {
				    	blk_crx->comps[ic] = c;
					blk_crx->nv[ic] = 1;
					++blk_crx->num_comps;
				    }
	                            blk_crx->comp[i][j][k] = c;
	                            blk_crx->ix[i][j][k] = i;
	                            blk_crx->iy[i][j][k] = j;
	                            blk_crx->iz[i][j][k] = k;
				    if (do_volume_frac)
				    {
				    	blk_crx->corner_coords[i][j][k][0] =
						L[0] + (ix + i)*h[0];
				    	blk_crx->corner_coords[i][j][k][1] =
						L[1] + (iy + j)*h[1];
				    	blk_crx->corner_coords[i][j][k][2] =
						L[2] + (iz + k)*h[2];
				    }
	                        }
	                    }
	                }
			/* Put blk_crx->comps[i] in ascending order */
			for (i = 0; i < blk_crx->num_comps-1; ++i)
			{
			    for (j = i+1; j < blk_crx->num_comps; ++j)
			    {
			    	if (blk_crx->nv[i] > blk_crx->nv[j])
				{
				    int nv_tmp;
				    COMPONENT c_tmp;
				    nv_tmp = blk_crx->nv[i];
				    blk_crx->nv[i] = blk_crx->nv[j];
				    blk_crx->nv[j] = nv_tmp;
				    c_tmp = blk_crx->comps[i];
				    blk_crx->comps[i] = blk_crx->comps[j];
				    blk_crx->comps[j] = c_tmp;
				}
			    }
			}
			for (i = 0; i < 2; ++i)
			{
			    for (j = 0; j < 2; ++j)
			    {
				if (blk_crx->comp[0][i][j] !=
				    blk_crx->comp[1][i][j])
				{
				    ip[0] = ix;
				    ip[1] = iy + i;
				    ip[2] = iz + j;
				    fill_block_crx(0,i,j,blk_crx,ip,EAST,intfc);
				}
				else
				    blk_crx->crx[0][i][j]->p = NULL;

				if (blk_crx->comp[j][0][i] !=
				    blk_crx->comp[j][1][i])
				{
				    ip[0] = ix + j;
				    ip[1] = iy;
				    ip[2] = iz + i;
				    fill_block_crx(1,i,j,blk_crx,ip,NORTH,intfc);
				}
				else
				    blk_crx->crx[1][i][j]->p = NULL;

				if (blk_crx->comp[i][j][0] !=
				    blk_crx->comp[i][j][1])
				{
				    ip[0] = ix + i;
				    ip[1] = iy + j;
				    ip[2] = iz;
				    fill_block_crx(2,i,j,blk_crx,ip,UPPER,intfc);
				}
				else
				    blk_crx->crx[2][i][j]->p = NULL;
			    }
			}
	                if (!construct_comp2_blk(blk_crx,bm))
	                {
			    free_these(4, blk_mem, blk_mem_store,
				blk_info.surfs, blk_info.cur_tris);
	                    DEBUG_LEAVE(reconstruct_intfc3d_in_box_lgb)
	                    return FUNCTION_FAILED;
	                }
			if (blk_info.do_volume_frac)
			{
			    area[ix][iy][iz] = bm->area;
			    vol_frac[ix][iy][iz] = bm->volume_fraction;
			}

	                if (ixx != 0)
	                    stitch_adj_blk(blk_mem[izz][iyy][ixx-1],bm);
	                if (iyy != 0)
	                    stitch_adj_blk(blk_mem[izz][iyy-1][ixx],bm);
	                if (izz != 0)
	                    stitch_adj_blk(blk_mem[izz-1][iyy][ixx],bm);
		    }
	            else
		    {
	                blk_mem[izz][iyy][ixx] = NULL;
		    	if (blk_info.do_volume_frac)
			{
			    area[ix][iy][iz] = 0.0;
		    	    if (compon[iz][iy][ix] == volume_frac->comp_vfrac)
			    	vol_frac[ix][iy][iz] = 1.0;
		    	    else
			    	vol_frac[ix][iy][iz] = 0.0;
			}
		    }
	        }
	    }
	}
	for (iz = smin[2]; iz < smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy < smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix < smax[0]; ++ix)
	        {
	            ixx = ix - smin[0];
	            iyy = iy - smin[1];
	            izz = iz - smin[2];
	            if (compon[iz][iy][ix] == ONFRONT)
	            {
	                bm = blk_mem[izz][iyy][ixx];
	                if (bm->num_null_sides == 0)
	                    continue;
			bm->blk_info = &blk_info;
	                if (ix != smax[0]-1)
	                    remove_null_pair(bm,blk_mem[izz][iyy][ixx+1],0);
	                if (iy != smax[1]-1)
	                    remove_null_pair(bm,blk_mem[izz][iyy+1][ixx],1);
	                if (iz != smax[2]-1)
	                    remove_null_pair(bm,blk_mem[izz+1][iyy][ixx],2);
	            }
	        }
	    }
	}

	for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
	{
	    if (blk_info.cur_tris[i] != NULL)
	    {
		last_tri(*s) = blk_info.cur_tris[i];
		last_tri(*s)->next = tail_of_tri_list(*s);
		first_tri(*s)->prev = head_of_tri_list(*s);
	    }
	}
	for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
	    if ((*s)->num_tri == 0)
	    	delete_surface(*s);

	free_these(4, blk_mem, blk_mem_store,
		blk_info.surfs, blk_info.cur_tris);

	DEBUG_LEAVE(reconstruct_intfc3d_in_box_lgb)
	return FUNCTION_SUCCEEDED;
}	/* end reconstruct_intfc3d_in_box_lgb */



EXPORT 	boolean reconstruct_intfc3d_in_box(
	INTERFACE  *intfc,
	int *smin,
	int *smax,
	boolean full_reconst,
	VOLUME_FRAC *volume_frac)
{
	RECT_GRID	gr = topological_grid(intfc);
	int		*gmax = gr.gmax;
	Table		*T = table_of_interface(intfc);
	BLK_TRI		****blk_mem, *bm, *blk_mem_store;
	COMPONENT	***compon;
	COMPONENT	c;
	COMPONENT	*comp = T->components;
	int		ix, iy, iz, ixx, iyy, izz;
	int             ii, i, j, k, ic, nbc, ip[3];
	SURFACE		**s;
	static BLK_CRX	*blk_crx = NULL;
	BLK_INFO        blk_info;
	int       	n_fr_blk = 0;
	boolean		do_volume_frac = YES;
	double		*L = gr.L;
	double		*h = gr.h;
	double		***area,***vol_frac;
	CURVE		**cc;
	int             num_c,num_curve_crx;
        int             sgmax[3];
	char		db_name[40];
	static CURVE    **new_c=NULL;
	static int      tnc, *n_c = NULL;
	
	DEBUG_ENTER(reconstruct_intfc3d_in_box)
        
	sgmax[0] = smax[0]-smin[0];
        sgmax[1] = smax[1]-smin[1];
        sgmax[2] = smax[2]-smin[2]; 

	blk_info.num_surfs = 0;
        for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
            ++blk_info.num_surfs;
        
	blk_info.num_curves = 0;
        for (i = 0, cc = intfc->curves; cc && *cc; ++i, ++cc)
	{
            ++blk_info.num_curves;
	}
	num_c = blk_info.num_curves;

	compon = (table_of_interface(intfc))->compon3d;
	for (iz = smin[2]; iz < smax[2]; ++iz)
	    for (iy = smin[1]; iy < smax[1]; ++iy)
	        for (ix = smin[0]; ix < smax[0]; ++ix)
		    if (compon[iz][iy][ix] == ONFRONT)
		    	n_fr_blk++;
	
	uni_array(&blk_info.surfs,blk_info.num_surfs,sizeof(SURFACE*));
	uni_array(&blk_info.cur_tris,blk_info.num_surfs,sizeof(TRI*));
        uni_array(&blk_info.curves,blk_info.num_curves,sizeof(CURVE*));
	
	for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
        {
            blk_info.surfs[i] = *s;
	    if (full_reconst || volume_frac != NULL)
	    {
            	(*s)->num_tri = 0;
		blk_info.cur_tris[i] = NULL;
	    }
	    else
	    	blk_info.cur_tris[i] = last_tri(*s);
        }
        
	for (i = 0, cc = intfc->curves; cc && *cc; ++i, cc++)
        {
            blk_info.curves[i] = *cc;
        }

	if (blk_crx == NULL)
	    blk_crx = alloc_blk_crx(YES);
	if (volume_frac != NULL)
	{
	    blk_info.do_volume_frac = YES;
	    blk_crx->cell_volume = 1.0;
	    blk_crx->comp_vfrac = volume_frac->comp_vfrac;
	    for (i = 0; i < 3; ++i)
	    {
	    	blk_crx->cell_volume *= h[i];
		blk_crx->h[i] = h[i];
	    }
	    area = T->area;
	    vol_frac = T->vol_frac;
	}
	else
	    blk_info.do_volume_frac = NO;

	tri_array(&blk_mem,smax[2]-smin[2],smax[1]-smin[1],smax[0]-smin[0],
	          sizeof(BLK_TRI*));
	uni_array(&blk_mem_store,n_fr_blk,sizeof(BLK_TRI));

	nbc = 0;
	for (iz = smin[2]; iz < smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy < smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix < smax[0]; ++ix)
	        {

	            ixx = ix - smin[0];
	            iyy = iy - smin[1];
	            izz = iz - smin[2];
	            if (compon[iz][iy][ix] == ONFRONT && is_crx(comp, gmax, ix, iy, iz))
		    {
	                bm = blk_mem[izz][iyy][ixx] = &blk_mem_store[nbc++];
	                bm->blk_info = blk_crx->blk_info = &blk_info;
			blk_crx->num_comps = 0;
	                for (i = 0; i < 2; ++i)
	                {
	                    for (j = 0; j < 2; ++j)
	                    {
	                        for (k = 0; k < 2; ++k)
	                        {
	                            c = comp[d_index3d(ix+i,iy+j,iz+k,gmax)];
				    for (ic = 0; ic < blk_crx->num_comps; ++ic)
				    {
				    	if (c == blk_crx->comps[ic])
		                        {
				            ++blk_crx->nv[ic];
				            break;
				        }
				    }
				    if (ic == blk_crx->num_comps)
				    {
				    	blk_crx->comps[ic] = c;
					blk_crx->nv[ic] = 1;
					++blk_crx->num_comps;
				    }
	                            blk_crx->comp[i][j][k] = c;
	                            blk_crx->ix[i][j][k] = i;
	                            blk_crx->iy[i][j][k] = j;
	                            blk_crx->iz[i][j][k] = k;
				    if (do_volume_frac)
				    {
				    	blk_crx->corner_coords[i][j][k][0] =
						L[0] + (ix + i)*h[0];
				    	blk_crx->corner_coords[i][j][k][1] =
						L[1] + (iy + j)*h[1];
				    	blk_crx->corner_coords[i][j][k][2] =
						L[2] + (iz + k)*h[2];
				    }
	                        }
	                    }
	                }
			/* Put blk_crx->comps[i] in ascending order */
			for (i = 0; i < blk_crx->num_comps-1; ++i)
			{
			    for (j = i+1; j < blk_crx->num_comps; ++j)
			    {
			    	if (blk_crx->nv[i] > blk_crx->nv[j])
				{
				    int nv_tmp;
				    COMPONENT c_tmp;
				    nv_tmp = blk_crx->nv[i];
				    blk_crx->nv[i] = blk_crx->nv[j];
				    blk_crx->nv[j] = nv_tmp;
				    c_tmp = blk_crx->comps[i];
				    blk_crx->comps[i] = blk_crx->comps[j];
				    blk_crx->comps[j] = c_tmp;
				}
			    }
			}
			for (i = 0; i < 2; ++i)
			{
			    for (j = 0; j < 2; ++j)
			    {
				if (blk_crx->comp[0][i][j] !=
				    blk_crx->comp[1][i][j])
				{
				    ip[0] = ix;
				    ip[1] = iy + i;
				    ip[2] = iz + j;
				    fill_block_crx(0,i,j,blk_crx,ip,EAST,intfc);
				}
				else
				{
				    blk_crx->crx[0][i][j]->p = NULL;
				    blk_crx->crx[0][i][j]->s = NULL;
				}
				if (blk_crx->comp[j][0][i] !=
				    blk_crx->comp[j][1][i])
				{
				    ip[0] = ix + j;
				    ip[1] = iy;
				    ip[2] = iz + i;
				    fill_block_crx(1,i,j,blk_crx,ip,NORTH,intfc);
				}
				else
				{
				    blk_crx->crx[1][i][j]->p = NULL;
				    blk_crx->crx[1][i][j]->s = NULL;
				}
				if (blk_crx->comp[i][j][0] !=
				    blk_crx->comp[i][j][1])
				{
				    ip[0] = ix + i;
				    ip[1] = iy + j;
				    ip[2] = iz;
				    fill_block_crx(2,i,j,blk_crx,ip,UPPER,intfc);
				}
				else
				{
				    blk_crx->crx[2][i][j]->p = NULL;
				    blk_crx->crx[2][i][j]->s = NULL;
			    	}
			    }
			}
	                
                        for( i = 0; i < 2; ++i)
                        {
                            if(is_curve_crx(blk_crx->comp[i][0][0],
					    blk_crx->comp[i][1][0],
                                            blk_crx->comp[i][0][1],
					    blk_crx->comp[i][1][1]))
                            {
                                ip[0] = ix + i;
                                ip[1] = iy;
                                ip[2] = iz;
                                fill_block_curve_crx(0,i,blk_crx,ip,EAST,intfc);   
                            }       
                            else
			    {
                                blk_crx->curve_crx[0][i]->p = NULL;
				blk_crx->curve_crx[0][i]->c = NULL;
			    }
                            if(is_curve_crx(blk_crx->comp[0][i][0],
					    blk_crx->comp[0][i][1],
                                            blk_crx->comp[1][i][0],
					    blk_crx->comp[1][i][1]))
                            {
                                ip[0] = ix;
                                ip[1] = iy + i;
                                ip[2] = iz;
                                fill_block_curve_crx(1,i,blk_crx,ip,NORTH,intfc);
                            }
                            else
			    {
                                 blk_crx->curve_crx[1][i]->p = NULL;
				 blk_crx->curve_crx[1][i]->c = NULL;
			    }
                            if(is_curve_crx(blk_crx->comp[0][0][i],
					    blk_crx->comp[1][0][i],
                                            blk_crx->comp[0][1][i],
					    blk_crx->comp[1][1][i]))
                            {
                                ip[0] = ix;
                                ip[1] = iy;
                                ip[2] = iz + i;
                                fill_block_curve_crx(2,i,blk_crx,ip,UPPER,intfc);
                            }
                            else
			    {
                                 blk_crx->curve_crx[2][i]->p = NULL; 
				 blk_crx->curve_crx[2][i]->c = NULL;
			    }
                        }
                        
                        num_curve_crx = 0;
                        for (ii = 0; ii < 2; ++ii)
                        {
                           if (blk_crx->curve_crx[0][ii]->c != NULL)
                               ++num_curve_crx;
                           if (blk_crx->curve_crx[1][ii]->c != NULL)
                               ++num_curve_crx;
                           if (blk_crx->curve_crx[2][ii]->c != NULL)
                               ++num_curve_crx;
                        }
                       
                        if (blk_crx->num_comps == 2)
                        {
                           if (num_curve_crx == 0)
                               blk_crx->blk_type = COMP2_BLOCK;
                           else if (num_curve_crx != 0)
                               blk_crx->blk_type = BDRY_BLOCK;
                        }
                        else if (blk_crx->num_comps == 3)
                            blk_crx->blk_type = COMP3_BLOCK;
                        
                        switch (blk_crx->blk_type)
                        {
                        case COMP2_BLOCK:
			    if(!construct_comp2_blk(blk_crx,bm))
                            {
                                free(blk_mem);
                                free(blk_mem_store);
                                DEBUG_LEAVE(reconstruct_crx_intfc3d)
                                return FUNCTION_FAILED;
                            }
                            break;
                        case COMP3_BLOCK:
			    if(NO && ix==28 && iy==11  && iz == 6) 
			    {
				sprintf(db_name, "blk_chk%d-%d-%d", ix, iy, iz);
				set_debug_name(db_name);
			        blk_crx->debug_flag = YES;
			        add_to_debug("chk_bm");
			    }
			    else
			    {
			        blk_crx->debug_flag = NO;
			    }
			    if (!construct_comp3_blk(blk_crx,bm))
                            {
                                free(blk_mem);
                                free(blk_mem_store);
                                DEBUG_LEAVE(reconstruct_crx_intfc3d)
                                return FUNCTION_FAILED;
                            }
			    /*reomve_from_debug("chk_bm"); */
                            break;
                        case BDRY_BLOCK:
                            if (!construct_bdry_blk(blk_crx,bm))
                            {
                                free(blk_mem);
                                free(blk_mem_store);
                                DEBUG_LEAVE(reconstruct_crx_intfc3d)
                                return FUNCTION_FAILED;
                            }
                            break;
                        default:
                            screen("UNKNOWN BLOCK: code needed!\n");
                            clean_up(ERROR);
                        }

			if(debugging("print_blk"))
                            printf("block ix,iy,iz =[%d,%d,%d], before stich_adj_blk %d %d %d\n",
                                      ix,iy,iz, ixx, iyy, izz);

			if (blk_info.do_volume_frac)
			{
			    area[ix][iy][iz] = bm->area;
			    vol_frac[ix][iy][iz] = bm->volume_fraction;
			}

	                if (ixx != 0)
	                    stitch_adj_blk(blk_mem[izz][iyy][ixx-1],bm);
	                if (iyy != 0)
	                    stitch_adj_blk(blk_mem[izz][iyy-1][ixx],bm);
	                if (izz != 0)
	                    stitch_adj_blk(blk_mem[izz-1][iyy][ixx],bm);
		    }
	            else
		    {
	                blk_mem[izz][iyy][ixx] = NULL;
		    	if (blk_info.do_volume_frac)
			{
			    area[ix][iy][iz] = 0.0;
		    	    if (compon[iz][iy][ix] == volume_frac->comp_vfrac)
			    	vol_frac[ix][iy][iz] = 1.0;
		    	    else
			    	vol_frac[ix][iy][iz] = 0.0;
			}
		    }
	        }
	    }
	}

	
	if(n_c == NULL)
	{
	    uni_array(&new_c, 100, sizeof(CURVE*));
	    uni_array(&n_c, 100, sizeof(int));
	}
	tnc = make_curves_from_blk(new_c, n_c, sgmax,blk_mem,intfc->curves,num_c);

	for (iz = smin[2]; iz < smax[2]; ++iz)
	{
	    for (iy = smin[1]; iy < smax[1]; ++iy)
	    {
	        for (ix = smin[0]; ix < smax[0]; ++ix)
	        {
	            ixx = ix - smin[0];
	            iyy = iy - smin[1];
	            izz = iz - smin[2];

		    if (compon[iz][iy][ix] == ONFRONT && is_crx(comp, gmax, ix, iy, iz))
	            {
	                bm = blk_mem[izz][iyy][ixx];
	                if (bm->num_null_sides == 0)
	                    continue;
			bm->blk_info = &blk_info;
	                if (ix != smax[0]-1)
	                    remove_null_pair(bm,blk_mem[izz][iyy][ixx+1],0);
	                if (iy != smax[1]-1)
	                    remove_null_pair(bm,blk_mem[izz][iyy+1][ixx],1);
			
			if(NO && ix==10 && iy==15 && (iz == 13 || iz == 12)) 
			    add_to_debug("chk_bm");
                
			if (iz != smax[2]-1)
	                    remove_null_pair(bm,blk_mem[izz+1][iyy][ixx],2);
	                
			remove_from_debug("chk_bm");
		    }
	        }
	    }
	}

	for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
	{
	    if (blk_info.cur_tris[i] != NULL)
	    {
		last_tri(*s) = blk_info.cur_tris[i];
		last_tri(*s)->next = tail_of_tri_list(*s);
		first_tri(*s)->prev = head_of_tri_list(*s);
	    }
	}

redo_delete_c:	
	for (cc = intfc->curves; cc && *cc; ++cc)
	{
	    if ((*cc)->num_points == 0)
	    {
	    	delete_curve(*cc);
		goto redo_delete_c;
	    }
	}

redo_delete_s:
	for (i = 0, s = intfc->surfaces; s && *s; ++i, ++s)
	{
	    if ((*s)->num_tri == 0)
	    {
	    	delete_surface(*s);
		goto redo_delete_s;
	    }
	}

        install_curve_points_state(intfc);
	
	order_interface(intfc);
	for (cc = intfc->curves; cc && *cc; ++cc)
	{
	    reorder_curve_link_list(*cc);
	}
	
	free_these(5, blk_mem, blk_mem_store,
	    blk_info.surfs, blk_info.cur_tris, blk_info.curves);
	
	DEBUG_LEAVE(reconstruct_crx_intfc3d)
	return FUNCTION_SUCCEEDED;
}	/* end reconstruct_intfc3d_in_box */

/* ref: copy_tri_state_to_btri */
/* assume only one physical surface */
/* crx should be a curve crx, then lcomp ucomp is the positive comp and  */
/* negative comp of the physical surface. */
/* orient == POSITIVE_ORIENTATION means starting point,  */
/* orient == NEGATIVE_ORIENTATION for ending point */

LOCAL  boolean install_btri_states_from_crx(
	INTERFACE    *intfc,
	BOND_TRI     *btri,
	CRXING	     *crx,
	size_t	     sizest,
	ORIENTATION  orient)
{
static Locstate    obs_st=NULL;
Locstate	   sl, sr;
SURFACE		   *s;

	if(obs_st == NULL)
	{
	    /*it is wrong, because intfc will be destroyed */
	    /*obs_st = alloc_intfc_state(intfc, sizest); */
	    /*g_alloc_state   tested */
	    /*also see i_Static_point */
	    alloc_state(intfc, &obs_st, sizest);
	    obstacle_state(intfc, obs_st, sizest);
	}

	s = btri->surface;

	if(positive_component(s) == crx->lcomp)
	    sr = left_state(crx->pt);
	else  if(positive_component(s) == crx->ucomp)
	    sr = right_state(crx->pt);
	else
	    sr = obs_st;

	if(negative_component(s) == crx->lcomp)
	    sl = left_state(crx->pt);
	else  if(negative_component(s) == crx->ucomp)
	    sl = right_state(crx->pt);
	else
	    sl = obs_st;

	if(orient == POSITIVE_ORIENTATION)
	{
	    ft_assign(left_start_btri_state(btri), sl, sizest);
	    ft_assign(right_start_btri_state(btri), sr, sizest);
	    return YES;
	}

	if(orient == NEGATIVE_ORIENTATION)
	{
	    ft_assign(left_end_btri_state(btri), sl, sizest);
	    ft_assign(right_end_btri_state(btri), sr, sizest);
	    return YES;
	}

	printf("ERROR fill_btri_states_from_crx, wrong orient = %d\n", orient);
	clean_up(ERROR);
}


LOCAL   void     install_curve_points_state(
       			INTERFACE   *intfc)
{
	Table 	    *T = table_of_interface(intfc);
        CRXING      *crx;
        CURVE       **c;
        BOND        *b;
        BOND_TRI    **btri;
        FT_POINT       *p;
        int         k;
        size_t      sizest = size_of_state(intfc);
                  
        /*add_to_debug("install_c_st"); */
	
	for(c = intfc->curves; c && *c; c++)
        {
            p = (*c)->first->start;
            b = (*c)->first;
	    /*insert_curve_face_crossings ft_assigns Index_of_point as face index */
	    
	    k = T->curve_crx_lists[Index_of_point(p)][0];
            crx = &T->curve_crx_store[k];

	    if(debugging("install_c_st"))
	        print_wall_curve_crx0("inst_c_st", p, Index_of_point(p), crx);

	    for(btri = Btris(b); btri && *btri; btri++)
	        install_btri_states_from_crx(intfc,*btri,crx,sizest,POSITIVE_ORIENTATION);
            
            for(b = (*c)->first; b; b = b->next)
            {
                p = b->end;

	        k = T->curve_crx_lists[Index_of_point(p)][0];
                crx = &T->curve_crx_store[k];
	
		if(debugging("install_c_st"))
	            print_wall_curve_crx0("inst_c_st", p, Index_of_point(p), crx);
        
		for(btri = Btris(b); btri && *btri; btri++)
		{
	            install_btri_states_from_crx(intfc,*btri,crx,sizest,NEGATIVE_ORIENTATION);
		}
	    }
        }

        /*remove_from_debug("install_c_st"); */
}       /* end install_curve_points_state */

EXPORT	SURFACE  * find_surf_with_comp(
	INTERFACE    *intfc, 
	int	     c0, 
	int	     c1)
{
SURFACE    **s;

	for(s=intfc->surfaces; s && *s; s++)
	    if( (positive_component(*s) == c0 && negative_component(*s) == c1) ||
	        (positive_component(*s) == c1 && negative_component(*s) == c0) )
	    {
	        return *s;
	    }
	return NULL;
}

EXPORT	boolean	curves_on_bdry_side(
	int       dir,
	int       side,
	INTERFACE *intfc)
{
	RECT_GRID *gr = computational_grid(intfc);
	CURVE     **c;
	int       idir, iside;
	
	for (c = intfc->curves; c && *c; ++c)
	{
	    if (!is_bdry(*c))
		continue;
	    (void) rect_bdry_side_for_curve(&idir,&iside,*c,gr);
	    if ((idir == dir) && (iside == side))
		return YES;
	}
	return NO;
}		/*end curves_on_bdry_side*/

EXPORT	void set_crx_storage_for_reconstruction(
	INTERFACE *intfc,
	VOLUME_FRAC *volume_frac)
{
	RECT_GRID *gr = &topological_grid(intfc);
	int *gmax = gr->gmax;
	int dim = gr->dim;
	Table *T = table_of_interface(intfc);
	int n_segs,n_crx,i,n_reg_nodes;

	n_segs = 0;
	n_reg_nodes = 1;
	switch (dim)
	{
        case 1:
            n_segs = gmax[0];
            n_reg_nodes = gmax[0] + 1;
            set_grid_lines(gr);
            break;
	case 2:
	    n_segs = gmax[0]*(gmax[1]+1) + gmax[1]*(gmax[0]+1);
	    n_reg_nodes = (gmax[0]+1)*(gmax[1]+1);
	    set_grid_lines(gr);
	    break;
	case 3:
	    for (i = 0; i < dim; ++i)
	    {
	    	n_segs += gmax[i]*(gmax[(i+1)%3] + 1)*(gmax[(i+2)%3] + 1);
	    	n_reg_nodes *= gmax[i] + 1;
	    }
	}
	T->n_segs = n_segs;

	uni_array(&T->seg_crx_count,n_segs,INT);
	for (i = 0; i < n_segs; ++i)
	    T->seg_crx_count[i] = 0;

	n_crx = count_grid_intfc_crossings(intfc);
	T->n_crx = n_crx;
	init_seg_crx_lists(intfc,n_crx,n_segs);
	uni_array(&T->components,n_reg_nodes,sizeof(COMPONENT));
	if (dim == 3 && volume_frac != NULL)
	{
	    tri_array(&T->area,gmax[0],gmax[1],gmax[2],FLOAT);
	    tri_array(&T->vol_frac,gmax[0],gmax[1],gmax[2],FLOAT);
	}
	else
	    T->area = T->vol_frac = NULL;
}	/*end set_crx_storage_for_reconstruction*/


EXPORT	boolean remove_unphysical_crossings3d(
	INTERFACE *intfc,
	int *smin,
	int *smax)
{
        boolean   status;
	RECT_GRID gr = topological_grid(intfc);
	int 	  *gmax = gr.gmax;
	Table	  *T = table_of_interface(intfc);
	COMPONENT *comp = T->components;
	int       i, n_reg_node;

	DEBUG_ENTER(remove_unphysical_crossings3d)

	n_reg_node = (gmax[0]+1)*(gmax[1]+1)*(gmax[2]+1);
	/*  fill_comp_from_solid_intfc */
	/*  set_edge_flag_for_wall */
	if(!use_wall_edge())
	    for (i = 0; i < n_reg_node; ++i)
	        comp[i] = NO_COMP;
	
	if(debugging("tst_comp3d1"))
	{
 	    printf("#prev comp bf\n");
	    show_grid_components(smin,smax,2,intfc);
	    /*remove_from_debug("show_comp_prev"); */
	}

	status = track_comp_through_crxings3d(smin,smax,gmax,intfc,SINGLE);

	DEBUG_LEAVE(remove_unphysical_crossings3d)
	return FUNCTION_SUCCEEDED;
}	/* end remove_unphysical_crossings3d */

LOCAL  boolean is_crx(int *comp, int *gmax, int ix, int iy, int iz)
{
int i, j, k, c;

	c = comp[d_index3d(ix,iy,iz,gmax)];
	for (i = 0; i < 2; ++i)
	    for (j = 0; j < 2; ++j)
	        for (k = 0; k < 2; ++k)
		{
	            if(c != comp[d_index3d(ix+i,iy+j,iz+k,gmax)])
		        return YES;
	        }
	return NO;
}


LOCAL	void prt_block_crx(
	INTERFACE       *intfc)
{
	Table *T = table_of_interface(intfc);
	RECT_GRID *gr = &topological_grid(intfc);
	int *gmax = gr->gmax;
	int l,nc,list;
	CRXING *crx;
	SURFACE   *s;

	if(pp_mynode()!=1)
	    return;
        
	list = T->seg_crx_lists[4379][0];
	crx = &T->crx_store[list];
	s = Surface_of_hs(crx->hs);

	/*if(ip[0] == 11 && ip[1] == 12 && ip[2] == 20 && dir == 1) */
	    printf("#block_crxp %p %p %p %p\n", crx->hs, s, Hyper_surf(s), crx);

}	/* end fill_block_crx */



LOCAL	void fill_block_crx(
	int            i,
	int            j,
	int            k,
	BLK_CRX        *blk_crx,
	int            *ip,
	GRID_DIRECTION dir,
	INTERFACE       *intfc)
{
	Table *T = table_of_interface(intfc);
	RECT_GRID *gr = &topological_grid(intfc);
	int *gmax = gr->gmax;

	int l,nc,list;
	CRXING *crx;

	l = seg_index3d(ip[0],ip[1],ip[2],dir,gmax);
	nc = T->seg_crx_count[l];
	if (nc != 0)
	{
	    list = T->seg_crx_lists[l][0];
	    crx = &T->crx_store[list];
	    blk_crx->crx[i][j][k]->s = Surface_of_hs(crx->hs);
	    
	    /*if(ip[0] == 11 && ip[1] == 12 && ip[2] == 20 && dir == 1) */
	    /*    print_wall_crx("fill_blk_crx",ip,dir,l,crx); */
	    
	    blk_crx->crx[i][j][k]->p = crx->pt;
	}
	else
	{
	    printf("ERROR fill_block_crx, can not find crx %d  %d %d\n", i, j, k);
	    print_int_vector("ip=",ip,3,"  ");
	    printf("dir = %d\n", dir);
	    clean_up(ERROR);
	}
}	/* end fill_block_crx */

LOCAL   void fill_block_curve_crx(
        int            i,	/*direction = 0,1,2 */
        int            j,	/*first or second = 0,1 */
        BLK_CRX        *blk_crx,
        int            *ip,
        GRID_DIRECTION dir,
        INTERFACE      *intfc)
{
	Table 		*T = table_of_interface(intfc);
	RECT_GRID 	*gr = &topological_grid(intfc);
	int 		*gmax = gr->gmax;
        int 		l,nc,list;
        CRXING 		*crx;
	
        l = face_index3d(ip[0],ip[1],ip[2],dir,gmax);
        nc = T->curve_crx_count[l];
        
	if (nc != 0)
        {
            list = T->curve_crx_lists[l][0];
            crx = &T->curve_crx_store[list];
	    
	    /*if(ip[0] == 11 && ip[1] == 12 && ip[2] == 20 && dir == 1) */
	    /*    print_wall_curve_crx("fill_blk_c_crx",ip,dir,l,crx); */
	
            blk_crx->curve_crx[i][j]->c = Curve_of_hsb(crx->hsb);
            blk_crx->curve_crx[i][j]->p = crx->pt;
        }
	else
	{
	    printf("ERROR fill_block_curve_crx, can not find crx %d  %d\n", i, j);
	    print_int_vector("ip=",ip,3,"  ");
	    printf("dir = %d\n", dir);
	    clean_up(ERROR);
	}

}       /* end fill_block_curve_crx */

EXPORT 	boolean track_comp_through_crxings3d(
	int 	 *smin,
	int 	 *smax,
	int 	 *gmax,
	INTERFACE *intfc,
	CRX_TYPE crx_type)
{
	int count = 0;
	DEBUG_ENTER(track_comp_through_crxings3d)

	/* eliminate duplicate crossings */
	/*adjust_crossings(smin,smax,intfc); */

	/* assign components and isolate unphysical clusters */
	fill_physical_comps(smin,smax,gmax,intfc);

	/* this is different from prev alg. if there are NO_COMP found,  */
	/* the compoennt3d is always called.  */
	fill_comp_with_component3d(smin,smax,gmax,intfc);
	
	/* annihilate unphysical clusters */
	while(unset_comp_exist(smin,smax,intfc))
	{
	    remove_unphysical_crxings(smin,smax,gmax,intfc,crx_type);
	    if (count++ == 4)
	    {
	    	screen("ERROR: unset component still exist after 4 rounds!\n");
		clean_up(ERROR);
	    }
	}
	
	/* check and repair crossings */
	if (!check_and_repair_crx(intfc,smin,smax))
	{
	    DEBUG_LEAVE(track_comp_through_crxings3d)
	    return FUNCTION_FAILED;
	}

	DEBUG_LEAVE(track_comp_through_crxings3d)
	return FUNCTION_SUCCEEDED;

}	/* end track_comp_through_crxings3d */

LOCAL	boolean unset_comp_exist(
	int      *smin,
        int      *smax,
        INTERFACE *intfc)
{
	RECT_GRID	gr = topological_grid(intfc);
	Table		*T = table_of_interface(intfc);
	COMPONENT       *comp = T->components;
	int		*gmax = gr.gmax;
	int		ip[3];

	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	        {
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    	return YES;
	        }
	    }
	}
	return NO;
}	/* end unset_comp_exist */

LOCAL	int check_and_unset_bad_comp(
	int      *smin,
	int      *smax,
	INTERFACE *intfc)
{
	RECT_GRID	gr = topological_grid(intfc);
	GRID_DIRECTION 	dir[6] = {WEST,EAST,SOUTH,NORTH,LOWER,UPPER};
	int 		ip[3],i,k,num_bad_nb;
	int		status = YES;
	int		*gmax = gr.gmax;
	Table		*T = table_of_interface(intfc);
	COMPONENT       *comp = T->components;
	int		*ef = T->edge_flag;
	boolean		***unset;
	DEBUG_ENTER(check_and_unset_bad_comp)

	tri_array(&unset,gmax[0]+1,gmax[1]+1,gmax[2]+1,sizeof(boolean));
	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	        {
		    num_bad_nb = 0;
	            for (i = 0; i < 6; ++i)
	            {
	                if (ip[0] == smin[0] && dir[i] == WEST)
	                    continue;
	                if (ip[0] == smax[0] && dir[i] == EAST)
	                    continue;
	                if (ip[1] == smin[1] && dir[i] == SOUTH)
	                    continue;
	                if (ip[1] == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (ip[2] == smin[2] && dir[i] == LOWER)
	                    continue;
	                if (ip[2] == smax[2] && dir[i] == UPPER)
	                    continue;
			
			k = seg_index3d(ip[0],ip[1],ip[2],dir[i],gmax);
			if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			    continue;

			if (physical_edge(ip,dir[i],intfc,smin,smax,MULTIPLE) 
					!= YES)
	                {
			    ++num_bad_nb;
			    status = NO;
	                }
	            }
		    if (num_bad_nb != 0)
		    	unset[ip[0]][ip[1]][ip[2]] = YES;
		    else
		    	unset[ip[0]][ip[1]][ip[2]] = NO;
	        }
	    }
	}
	if (status == YES)
	{
	    free_these(1,unset);
	    DEBUG_LEAVE(check_and_unset_bad_comp)
	    return status;
	}
	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	        {
		    if (unset[ip[0]][ip[1]][ip[2]] == YES)
		    	comp[d_index3d(ip[0],ip[1],ip[2],gmax)] = NO_COMP;
	        }
	    }
	}
	free_these(1,unset);
	DEBUG_LEAVE(check_and_unset_bad_comp)
	return status;
}	/* end check_and_unset_bad_comp */

EXPORT	void fill_physical_comps(
        int      *smin,
        int      *smax,
        int      *gmax,
        INTERFACE *intfc)
{
	int 		k,l,nc,list,step,i;
	int 		ip[3];
	double           coords[3], *L, *h;
	Table		*T = table_of_interface(intfc);
	COMPONENT 	*comp = T->components;
	int		*ef = T->edge_flag;
	COMPONENT 	cn;
	CRXING          *crx;
	boolean		status;
	RECT_GRID	gr = topological_grid(intfc);

	DEBUG_ENTER(fill_physical_comps)
	
	L = gr.L;
	h = gr.h;

	if (intfc->surfaces == NULL  ||  
	    is_outside_surfaces(intfc,&gr) ||
	    T->n_crx == 0)
	{
	    for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	        for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	            for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	    	        comp[d_index3d(ip[0],ip[1],ip[2],gmax)] 
				= intfc->default_comp;
	    
	    DEBUG_LEAVE(fill_physical_comps)
	    return;
	}
	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
		for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
		{
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    {
			/* if smin[0] or smax[0] is not NO_COMP, walk... 
			   will fill comp in direction, if for example smin[0] 
			   is NO_COMP, two case. 
			   (1) edge has no crx, need to continue because code 
			       can not determine the component.
			   (2) edge has one crx, smin[0]+1 will determine the 
			       component, and walk.. will fill the comp at 
			       sin[0] 
			*/
			if(ip[0] == smin[0] || ip[0] == smax[0])
				continue;

		    	k = seg_index3d(ip[0],ip[1],ip[2],WEST,gmax);
			if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			    continue;

		    	nc = T->seg_crx_count[k];
		    	if (nc == 0)
			{
		    	    k = seg_index3d(ip[0],ip[1],ip[2],EAST,gmax);
			    if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			        continue;
		    	    nc = T->seg_crx_count[k];
		    	    if (nc == 0)
		    	    	continue;
		    	    list = T->seg_crx_lists[k][0];
		    	    crx = T->crx_store+list;
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] 
			    			= crx->lcomp;
			}
			else
			{
		    	    list = T->seg_crx_lists[k][nc-1];
		    	    crx = T->crx_store+list;
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] 
			    			= crx->ucomp;
			}
			

		    }

		    if (ip[0] != 0)
		    {
	            	cn = comp[d_index3d(ip[0]-1,ip[1],ip[2],gmax)];
		    	if (cn == NO_COMP)
			{
			    step = walk_comp_along_grid_line(intfc,smin,smax,
					                     gmax,ip,WEST);
			}
		    }
		    if (ip[0] != smax[0])
		    {
	            	cn = comp[d_index3d(ip[0]+1,ip[1],ip[2],gmax)];
		    	if (cn == NO_COMP)
		    	{
			    step = walk_comp_along_grid_line(intfc,smin,smax,
					                     gmax,ip,EAST);
			    ip[0] += step;
		    	}
		    }
		}
	    }
	}
	
	if(debugging("tst_comp3d1"))
	{
 	    printf("#inside comp x af\n");
	    show_grid_components(smin,smax,2,intfc);
	}

	status = check_and_unset_bad_comp(smin,smax,intfc);
	

	if (status == YES)
	{
	    if (debugging("crx_intfc"))
	    {
	        printf("After first check: status = %d\n",status);
	        show_grid_components(smin,smax,2,intfc);
	    }
	    DEBUG_LEAVE(fill_physical_comps)
	    return;
	}
	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	    {
	    	for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
		{
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    {
			if(ip[1] == smin[1] || ip[1] == smax[1])
				continue;

		    	k = seg_index3d(ip[0],ip[1],ip[2],SOUTH,gmax);
			if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			    continue;
		    	nc = T->seg_crx_count[k];
		    	if (nc == 0)
			{
		    	    k = seg_index3d(ip[0],ip[1],ip[2],NORTH,gmax);
			    if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			        continue;
		    	    nc = T->seg_crx_count[k];
		    	    if (nc == 0)
		    	    	continue;
		    	    list = T->seg_crx_lists[k][0];
		    	    crx = T->crx_store+list;
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] 
			    			= crx->lcomp;
			}
			else
			{
		    	    list = T->seg_crx_lists[k][nc-1];
		    	    crx = T->crx_store+list;
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] 
			    			= crx->ucomp;
			}
		    }

		    if (ip[1] != 0)
		    {
	            	cn = comp[d_index3d(ip[0],ip[1]-1,ip[2],gmax)];
		    	if (cn == NO_COMP)
			    step = walk_comp_along_grid_line(intfc,smin,smax,
					                     gmax,ip,SOUTH);
		    }
		    if (ip[1] != smax[1])
		    {
	            	cn = comp[d_index3d(ip[0],ip[1]+1,ip[2],gmax)];
		    	if (cn == NO_COMP)
		    	{
			    step = walk_comp_along_grid_line(intfc,smin,smax,
					                     gmax,ip,NORTH);
			    ip[1] += step;
		    	}
		    }
		}
	    }
	}
	
	if(debugging("show_3c_comp"))
	{
 	    printf("#inside comp y bf\n");
	    show_grid_components(smin,smax,2,intfc);
	}

	status = check_and_unset_bad_comp(smin,smax,intfc);
	
	if(debugging("show_3c_comp"))
	{
 	    printf("#inside comp y af\n");
	    show_grid_components(smin,smax,2,intfc);
	}
	
	if (status == YES)
	{
	    if (debugging("crx_intfc"))
	    {
	        printf("After second check: status = %d\n",status);
	        show_grid_components(smin,smax,2,intfc);
	    }
	    DEBUG_LEAVE(fill_physical_comps)
	    return;
	}
	for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	{
	    for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	    {
	        for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
		{
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    {
		        if(ip[2] == smin[2] || ip[2] == smax[2])
				continue;

		    	k = seg_index3d(ip[0],ip[1],ip[2],LOWER,gmax);
			if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			    continue;
		    	nc = T->seg_crx_count[k];
		    	if (nc == 0)
			{
		    	    k = seg_index3d(ip[0],ip[1],ip[2],UPPER,gmax);
			    if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			        continue;
		    	    nc = T->seg_crx_count[k];
		    	    if (nc == 0)
		    	    	continue;
		    	    list = T->seg_crx_lists[k][0];
		    	    crx = T->crx_store+list;
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] 
			    			= crx->lcomp;
			}
			else
			{
		    	    list = T->seg_crx_lists[k][nc-1];
		    	    crx = T->crx_store+list;
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] 
			    			= crx->ucomp;
			}
		    }

		    if (ip[2] != 0)
		    {
	            	cn = comp[d_index3d(ip[0],ip[1],ip[2]-1,gmax)];
		    	if (cn == NO_COMP)
			    step = walk_comp_along_grid_line(intfc,smin,smax,
					                     gmax,ip,LOWER);
		    }
		    if (ip[2] != smax[2])
		    {
	            	cn = comp[d_index3d(ip[0],ip[1],ip[2]+1,gmax)];
		    	if (cn == NO_COMP)
		    	{
			    step = walk_comp_along_grid_line(intfc,smin,smax,
					                     gmax,ip,UPPER);
			    ip[2] += step;
		    	}
		    }
		}
	    }
	}
	
	if(debugging("show_3c_comp"))
	{
 	    printf("#inside comp z af\n");
	    show_grid_components(smin,smax,2,intfc);
	}
	
	status = check_and_unset_bad_comp(smin,smax,intfc);
	
	if (status == YES)
	{
	    if (debugging("crx_intfc"))
	    {
	        printf("After third check: status = %d\n",status);
	        show_grid_components(smin,smax,2,intfc);
	    }
	    DEBUG_LEAVE(fill_physical_comps)
	    return;
        }

        for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
		{
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    {
		        if(ip[0] == smin[0] || ip[0] == smax[0])
				continue;

		        k = seg_index3d(ip[0],ip[1],ip[2],WEST,gmax);
			if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			    continue;
			nc = T->seg_crx_count[k];
			if (nc == 0)
			{
			    k = seg_index3d(ip[0],ip[1],ip[2],EAST,gmax);
			    if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			        continue;
			    nc = T->seg_crx_count[k];
			    if (nc == 0)
			        continue;
			    list = T->seg_crx_lists[k][0];
			    crx = T->crx_store+list;
			    comp[d_index3d(ip[0],ip[1],ip[2],gmax)]
			                        = crx->lcomp;
			}
			else
			{
			    list = T->seg_crx_lists[k][nc-1];
			    crx = T->crx_store+list;
			    comp[d_index3d(ip[0],ip[1],ip[2],gmax)]
			                        = crx->ucomp;
			}
		    }

		    if (ip[0] != 0)
		    {
		        cn = comp[d_index3d(ip[0]-1,ip[1],ip[2],gmax)];
			if (cn == NO_COMP)
			    step = walk_comp_along_grid_line(intfc,smin,smax,
			                                     gmax,ip,WEST);
		    }
		    if (ip[0] != smax[0])
		    {
		        cn = comp[d_index3d(ip[0]+1,ip[1],ip[2],gmax)];
			if (cn == NO_COMP)
			{
			    step = walk_comp_along_grid_line(intfc,smin,smax,
			                                     gmax,ip,EAST);
			    ip[0] += step;
			}
		    }
		}
	    }
	}
	status = check_and_unset_bad_comp(smin,smax,intfc);
	
	if(debugging("show_3c_comp"))
	{
 	    printf("#inside comp x1 af\n");
	    show_grid_components(smin,smax,2,intfc);
	}
	
	if (status == YES)
	{
	    DEBUG_LEAVE(fill_physical_comps)
	    return;
	}
	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
            {
	        for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
		{
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    {
		 	if(ip[1] == smin[1] || ip[1] == smax[1])
				continue;

		        k = seg_index3d(ip[0],ip[1],ip[2],SOUTH,gmax);
			if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			    continue;
			nc = T->seg_crx_count[k];
			if (nc == 0)
			{
			    k = seg_index3d(ip[0],ip[1],ip[2],NORTH,gmax);
			    if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			        continue;
			    nc = T->seg_crx_count[k];
			    if (nc == 0)
			        continue;
			    list = T->seg_crx_lists[k][0];
			    crx = T->crx_store+list;
			    comp[d_index3d(ip[0],ip[1],ip[2],gmax)]
			                        = crx->lcomp;
			}
			else
			{
			    list = T->seg_crx_lists[k][nc-1];
			    crx = T->crx_store+list;
			    comp[d_index3d(ip[0],ip[1],ip[2],gmax)]
			                        = crx->ucomp;
			}
		    }

		    if (ip[1] != 0)
		    {
		        cn = comp[d_index3d(ip[0],ip[1]-1,ip[2],gmax)];
			if (cn == NO_COMP)
			    step = walk_comp_along_grid_line(intfc,smin,smax,
			                                     gmax,ip,SOUTH);
		    }
		    if (ip[1] != smax[1])
		    {
		        cn = comp[d_index3d(ip[0],ip[1]+1,ip[2],gmax)];
			if (cn == NO_COMP)
			{
			    step = walk_comp_along_grid_line(intfc,smin,smax,
			                                     gmax,ip,NORTH);
			    ip[1] += step;
			}
		    }
		}
	    }
	}
	status = check_and_unset_bad_comp(smin,smax,intfc);
	
	if(debugging("show_3c_comp"))
	{
 	    printf("#inside comp y1 af\n");
	    show_grid_components(smin,smax,2,intfc);
	}

	if (status == YES)
	{
	    DEBUG_LEAVE(fill_physical_comps)
	    return;
        }
	for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	{
	    for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	    {
	        for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
		{
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    {
		 	if(ip[2] == smin[2] || ip[2] == smax[2])
				continue;

		         k = seg_index3d(ip[0],ip[1],ip[2],LOWER,gmax);
			 if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			     continue;
			 nc = T->seg_crx_count[k];
			 if (nc == 0)
		         {
			     k = seg_index3d(ip[0],ip[1],ip[2],UPPER,gmax);
			     if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
			         continue;
			     nc = T->seg_crx_count[k];
			     if (nc == 0)
			         continue;
			     list = T->seg_crx_lists[k][0];
                             crx = T->crx_store+list;
			     comp[d_index3d(ip[0],ip[1],ip[2],gmax)]
			                         = crx->ucomp;
			 }
		    }

		    if (ip[2] != 0)
		    {
		        cn = comp[d_index3d(ip[0],ip[1],ip[2]-1,gmax)];
			if (cn == NO_COMP)
			    step = walk_comp_along_grid_line(intfc,smin,smax,
			                                     gmax,ip,LOWER);
		    }

		    if (ip[2] != smax[2])
		    {
		        cn = comp[d_index3d(ip[0],ip[1],ip[2]+1,gmax)];
			if (cn == NO_COMP)
			{
			    step = walk_comp_along_grid_line(intfc,smin,smax,
			                                     gmax,ip,UPPER);
			    ip[2] += step;
			}
		    }
		}
	    }	    
	}
       	
        status = check_and_unset_bad_comp(smin,smax,intfc);
 	
	if(debugging("tst_comp3d1"))
	{
 	    printf("#inside comp z1 af\n");
	    show_grid_components(smin,smax,2,intfc);
	}
	
	DEBUG_LEAVE(fill_physical_comps)
	
}	/* end fill_physical_comps */

EXPORT	void  fill_comp_with_component3d(
	int      *smin,
        int      *smax,
        int      *gmax,
        INTERFACE *intfc)
{
	Table		*T = table_of_interface(intfc);
	COMPONENT 	*comp = T->components;
	RECT_GRID	*gr = &topological_grid(intfc);
	int 		i, ip[3];
	double           coords[3], *L=gr->L, *h=gr->h;
	double		nodetol = 1.0e-8;
	boolean		status;
	
	DEBUG_ENTER(fill_comp_with_component3d)

	intfc->modified = NO;

	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
                for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
		{
		    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] == NO_COMP)
		    {
			for (i = 0; i < 3; i++)
			    coords[i] = L[i] + h[i]*(ip[i] + nodetol);

			comp[d_index3d(ip[0],ip[1],ip[2],gmax)] =
				          component(coords,intfc);
		    }
		}
	    }
	}
	status = check_and_unset_bad_comp(smin,smax,intfc);
	
	if (debugging("crx_intfc"))
	{
	    printf("After fill_comp_with_component3d, bad comps are found.\n");
	    show_grid_components(smin,smax,2,intfc);
	}
	
	DEBUG_LEAVE(fill_comp_with_component3d)
}

EXPORT	int record_unphysical_ips(
	int      *smin,
	int      *smax,
	INTERFACE *intfc,
	int	 **ips)
{
	GRID_DIRECTION 	dir[6] = {WEST,EAST,SOUTH,NORTH,LOWER,UPPER};
	int 		ip[3],i,num_ip = 0;
	RECT_GRID 	*gr = &topological_grid(intfc);

	DEBUG_ENTER(record_unphysical_ips)

	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	        {
	            for (i = 0; i < 6; ++i)
	            {
	                if (ip[0] == smin[0] && dir[i] == WEST)
	                    continue;
	                if (ip[0] == smax[0] && dir[i] == EAST)
	                    continue;
	                if (ip[1] == smin[1] && dir[i] == SOUTH)
	                    continue;
	                if (ip[1] == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (ip[2] == smin[2] && dir[i] == LOWER)
	                    continue;
	                if (ip[2] == smax[2] && dir[i] == UPPER)
	                    continue;
	                if (physical_edge(ip,dir[i],intfc,smin,smax,MULTIPLE) 
				!= YES)
	                {
			    ips[num_ip][0] = ip[0];
			    ips[num_ip][1] = ip[1];
			    ips[num_ip][2] = ip[2];
			    ++num_ip;
			    break;
	                }
	            }
	        }
	    }
	}
	DEBUG_LEAVE(record_unphysical_ips)
	return num_ip;
}	/* end record_unphysical_ips */


void show_line_components3d(int*, int*, int, INTERFACE*);

EXPORT	void remove_unphysical_crxings(
        int      *smin,
        int      *smax,
        int      *gmax,
        INTERFACE *intfc,
	CRX_TYPE crx_type)
{
	int 		ix, iy, iz, step;
	int 		ip[3];
	Table		*T = table_of_interface(intfc);
	COMPONENT 	*comp = T->components;
	COMPONENT 	c,cn;

	/* remove unphysical crossings */

	for (ix = smin[0]; ix <= smax[0]; ++ix)
	{
	    ip[0] = ix;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	    
		if((ix == 12 && iy == 7 && pp_mynode()==9) ||
		   (ix == 12 && iy == 27 && pp_mynode()==11) )
		{
		    /*printf("rm_crx\n"); */
		    /*add_to_debug("rm_crx"); */
		}

		for (iz = smin[2]; iz <= smax[2]; ++iz)
		{
	            ip[2] = iz;
	            c = comp[d_index3d(ix,iy,iz,gmax)];
		    if (c == NO_COMP)
			continue;
		    
		    if(debugging("rm_crx"))
		    {
			show_line_components3d(smin, smax, 2, intfc);
			remove_from_debug("rm_crx");
		    }

		    if (iz != 0)
			step = rm_unphy_crx_along_grid_line(intfc,smin,smax,
					             gmax,ip,LOWER,crx_type);
		    if(debugging("rm_crx"))
			show_line_components3d(smin, smax, 2, intfc);
		    
		    if (iz != smax[2])
		    {
			step = rm_unphy_crx_along_grid_line(intfc,smin,smax,
					             gmax,ip,UPPER,crx_type);
		    	iz += step;
		    }
		    if(debugging("rm_crx"))
			show_line_components3d(smin, smax, 2, intfc);
	
		}
		remove_from_debug("rm_crx");
	    }
	}
	
	if(debugging("rm_crx"))
	    show_grid_components(smin, smax, 2, intfc);

	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (ix = smin[0]; ix <= smax[0]; ++ix)
	    {
	    	ip[0] = ix;
	    	for (iy = smin[1]; iy <= smax[1]; ++iy)
		{
	            ip[1] = iy;
	            c = comp[d_index3d(ix,iy,iz,gmax)];
		    if (c == NO_COMP)
			continue;

		    if (iy != 0)
			step = rm_unphy_crx_along_grid_line(intfc,smin,smax,
					             gmax,ip,SOUTH,crx_type);
		    if (iy != smax[1])
		    {
			step = rm_unphy_crx_along_grid_line(intfc,smin,smax,
					             gmax,ip,NORTH,crx_type);
		    	iy += step;
		    }
		}
	    }
	}

	if(debugging("rm_crx"))
	    show_grid_components(smin, smax, 2, intfc);


	for (iy = smin[1]; iy <= smax[1]; ++iy)
	{
	    ip[1] = iy;
	    for (iz = smin[2]; iz <= smax[2]; ++iz)
	    {
	    	ip[2] = iz;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
		{
	    	    ip[0] = ix;
	            c = comp[d_index3d(ix,iy,iz,gmax)];
		    if (c == NO_COMP)
			continue;
		    if (ix != 0)
			step = rm_unphy_crx_along_grid_line(intfc,smin,smax,
					             gmax,ip,WEST,crx_type);
		    if (ix != smax[0])
		    {
			step = rm_unphy_crx_along_grid_line(intfc,smin,smax,
					             gmax,ip,EAST,crx_type);
		    	ix += step;
		    }
		}
	    }
	}
}	/* end remove_unphysical_crxings */

EXPORT	boolean next_ip_in_dir(
	const int *ip,
	int       dir,
	int       *ipn,
	int *smin,
	int *smax)
{
	int i;

	for (i = 0; i < 3; ++i)
	    ipn[i] = ip[i];
	switch (dir)
	{
	case WEST:
	    ipn[0] -= 1;
	    if (ipn[0] < smin[0])
	        return NO;
	    break;
	case EAST:
	    ipn[0] += 1;
	    if (ipn[0] > smax[0])
	        return NO;
	    break;
	case SOUTH:
	    ipn[1] -= 1;
	    if (ipn[1] < smin[1])
	        return NO;
	    break;
	case NORTH:
	    ipn[1] += 1;
	    if (ipn[1] > smax[1])
	        return NO;
	    break;
	case LOWER:
	    ipn[2] -= 1;
	    if (ipn[2] < smin[2])
	        return NO;
	    break;
	case UPPER:
	    ipn[2] += 1;
	    if (ipn[2] > smax[2])
	        return NO;
	}
	return YES;
}	/* end next_ip_in_dir */

LOCAL	COMPONENT next_side_comp_at_crx(
	GRID_PT *gp,
	CRXING *crx)
{
	switch (gp->cur_dir)
	{
	case WEST:
	case SOUTH:
	case LOWER:
	    return crx->lcomp;
	case EAST:
	case NORTH:
	case UPPER:
	    return crx->ucomp;
	}
	return NO_COMP;
}		/*end next_side_comp_at_crx*/

LOCAL	COMPONENT this_side_comp_at_crx(
	GRID_PT *gp,
	CRXING *crx)
{
	switch (gp->cur_dir)
	{
	case WEST:
	case SOUTH:
	case LOWER:
	    return crx->ucomp;
	case EAST:
	case NORTH:
	case UPPER:
	    return crx->lcomp;
	}
	return NO_COMP;
}		/*end this_side_comp_at_crx*/

EXPORT	boolean check_and_repair_crx(
	INTERFACE *intfc,
	int      *smin,
	int      *smax)
{
	GRID_DIRECTION 	dir[6] = {EAST,NORTH,UPPER,WEST,SOUTH,LOWER};
	int 		ip[3],ipn[3],i;
	int 		num_crx_fill = 0;
	int		nic,l,nc,list;
	RECT_GRID	gr = topological_grid(intfc);
	int 		*gmax = gr.gmax;
	CRXING		*crx;
	Table		*T = table_of_interface(intfc);
	COMPONENT       *comp = T->components;
	int	     	*ef = T->edge_flag;
	boolean		dir_ic[6];
	DEBUG_ENTER(check_and_repair_crx)

	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	        {
		    nic = 0;
	            for (i = 0; i < 6; ++i)
	            {
			dir_ic[i] = NO;
	                if (ip[0] == smax[0] && dir[i] == EAST)
	                    continue;
	                if (ip[1] == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (ip[2] == smax[2] && dir[i] == UPPER)
	                    continue;
	                if (ip[0] == smin[0] && dir[i] == WEST)
	                    continue;
	                if (ip[1] == smin[1] && dir[i] == SOUTH)
	                    continue;
	                if (ip[2] == smin[2] && dir[i] == LOWER)
	                    continue;
			
			l = seg_index3d(ip[0],ip[1],ip[2],dir[i],gmax);
			if(use_wall_edge()  &&  ef[l] != INSIDE_WALL)
			    continue;

			if (check_comp_at(ip,dir[i],intfc,smin,smax,NO) != YES)
	                {
			    dir_ic[i] = YES;
			    ++nic;
			}
		    }
		    /*fix comp in node, if a node has NO_COMP, nic == 6,  */
		    /*it must be fixed here. */
		    if (nic > 3)
		    {
		    	for (i = 0; i < 6; ++i)
			{
			    if (!dir_ic[i]) continue;
			    /*note: dir_ic[i] == YES,  */
			    /*ef[k] == INSIDE_WALL or do not  */
			    /*have use_wall_edge */
			    l = seg_index3d(ip[0],ip[1],ip[2],dir[i],gmax);
			    nc = T->seg_crx_count[l];
			    if (nc != 0)
			    {
				if (i < 3)
				{
			    	    list = T->seg_crx_lists[l][0];
				    crx = &T->crx_store[list];
				    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] =
						crx->lcomp;
				}
				else
				{
			    	    list = T->seg_crx_lists[l][nc-1];
				    crx = &T->crx_store[list];
				    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] =
						crx->ucomp;
				}
				    
			    }
			    else if (next_ip_in_dir(ip,dir[i],ipn,smin,smax))
			    {
			    	comp[d_index3d(ip[0],ip[1],ip[2],gmax)] =
				    comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]; 
			    }
			}
		    }    /*if(nic>3) */
		}
	    }
	}
	for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
	{
	    for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
	    {
	        for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
	        {
	            for (i = 0; i < 3; ++i)
	            {
	                if (ip[0] == smax[0] && dir[i] == EAST)
	                    continue;
	                if (ip[1] == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (ip[2] == smax[2] && dir[i] == UPPER)
	                    continue;
			
			l = seg_index3d(ip[0],ip[1],ip[2],dir[i],gmax);
			if(use_wall_edge()  &&  ef[l] != INSIDE_WALL)
			    continue;

	                if (check_comp_at(ip,dir[i],intfc,smin,smax,YES) != YES)
	                {
			    (void) next_ip_in_dir(ip,dir[i],ipn,smin,smax);
			    num_crx_fill = fill_missing_crx(intfc,ip,ipn,
			    		dir[i],num_crx_fill);
			    if (num_crx_fill == MAX_CRX_FILL)
			    {
			    	(void) printf("WARNING in "
					  "check_and_repair_crx(), "
					  "too many missing crossings.\n");
			    	DEBUG_LEAVE(check_and_repair_crx)
			    	return NO;
			    }
	                }
			if (num_crx_fill == MAX_CRX_FILL)
			{
			    (void) printf("WARNING in "
					  "check_and_repair_crx(), "
					  "too many missing crossings.\n");
			    DEBUG_LEAVE(check_and_repair_crx)
			    return NO;
			}
	            }
	        }
	    }
	}
	if (num_crx_fill != 0)
	{
	    T->n_crx += num_crx_fill;
	    (void) printf("WARNING %d crossings are missing, will fill!\n",
		   num_crx_fill);
	}
	DEBUG_LEAVE(check_and_repair_crx)
	return YES;
}	/* end check_and_repair_crx */

LOCAL int fill_missing_crx(
	INTERFACE *intfc,
	int *ip,
	int *ipn,
	GRID_DIRECTION  dir,
	int num_crx_fill)
{
	COMPONENT	***comp3d;
	Table		*T = table_of_interface(intfc);
	COMPONENT	*comp = T->components;
	COMPONENT 	lcomp, ucomp;
	CRXING 		*crx_fill = &T->crx_store[T->n_crx];
	HYPER_SURF 	*hs;
	SURFACE		*surf;
	Locstate 	sl, su;
	FT_POINT 		*p;
	RECT_GRID 	*expanded_dual_grid = &topological_grid(intfc);
	double 		coords[MAXD];
	int             *gmax = expanded_dual_grid->gmax;
	int		l;

	DEBUG_ENTER(fill_missing_crx)
	
	comp3d = (table_of_interface(intfc))->compon3d;
	for (l = 0; l < 3; ++l)
	{
	    coords[l] = expanded_dual_grid->L[l] + 
	    		ip[l]*expanded_dual_grid->h[l];
	}
	if (dir == EAST)
	    coords[0] += 0.5*expanded_dual_grid->h[0];
	else if (dir == NORTH)
	    coords[1] += 0.5*expanded_dual_grid->h[1];
	else if (dir == UPPER)
	    coords[2] += 0.5*expanded_dual_grid->h[2];
	lcomp = comp[d_index3d(ip[0],ip[1],ip[2],gmax)];
	ucomp = comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)];
	
	/* check_comp_at return NO, lcomp==ucomp is possible. */
	if(lcomp == ucomp)
	{
	    l = seg_index3d(ip[0],ip[1],ip[2],dir,gmax);
	    T->seg_crx_count[l] = 0;
	    
	    DEBUG_LEAVE(fill_missing_crx)
	    return num_crx_fill;
	}
	surf = find_surf_with_comp(intfc, lcomp, ucomp);
	
	printf("#fill_missing  %p  %d  %d \n", surf, lcomp, ucomp);
	if(surf == NULL || is_wall_surface(surf))
	{
	    printf("ERROR fill_missing_crx, surf is inconsistent.\n");
	    clean_up(ERROR);
	}
	
	p = Point(coords);
	sl = left_state(p);
	su = right_state(p);

	intfc->modified = NO;
	
	/*add_to_debug("line_tri"); */
	/*printf("#bf nearest intfc.\n"); */
	
	(void) nearest_intfc_state(coords,lcomp,intfc,
				sl,NULL,&hs);
	if (negative_component(surf) == lcomp)
	    (void) nearest_intfc_state(coords,ucomp,intfc,
				su,NULL,&hs);
	else
	{
	    (void) nearest_intfc_state(coords,ucomp,intfc,
			   sl,NULL,&hs);
	    (void) nearest_intfc_state(coords,lcomp,intfc,
			   su,NULL,&hs);
	}

	/*printf("#af nearest intfc.\n"); */
	/*remove_from_debug("line_tri"); */

	printf("num_crx_fill %d dir = %d\n", num_crx_fill, dir);
	print_int_vector("ip", ip, 3, "\n");

	crx_fill[num_crx_fill].pt  = p;
	crx_fill[num_crx_fill].hs = Hyper_surf(surf);
	crx_fill[num_crx_fill].lcomp = lcomp;
	crx_fill[num_crx_fill].ucomp = ucomp;
	l = seg_index3d(ip[0],ip[1],ip[2],dir,gmax);
	T->seg_crx_count[l] = 1;
	T->seg_crx_lists[l] = &(T->seg_crx_lists_store[
			T->n_crx + num_crx_fill]);
	T->seg_crx_lists[l][0] = T->n_crx + num_crx_fill;
	
	/*printf("#seg finish.\n"); */

	/* ip is the edge of the top grid, comp3d is the center of the top grid. */
	/*0 <= ip <= gmax */
	if (dir == EAST)
	{
	    if(ip[2] >= 1 && ip[1] >= 1)
		comp3d[ip[2]-1][ip[1]-1][ip[0]] = ONFRONT;
	    if (ip[2] >= 1 && ip[1] < gmax[1])
		comp3d[ip[2]-1][ip[1]][ip[0]] = ONFRONT;
	    if (ip[2] < gmax[2] && ip[1] >= 1)
		comp3d[ip[2]][ip[1]-1][ip[0]] = ONFRONT;
	    if (ip[1] < gmax[1] && ip[2] < gmax[2])
		comp3d[ip[2]][ip[1]][ip[0]] = ONFRONT;
	}
	else if (dir == NORTH)
	{
	    if(ip[2] >= 1 && ip[0] >= 1)
		comp3d[ip[2]-1][ip[1]][ip[0]-1] = ONFRONT;
	    if (ip[2] >= 1 && ip[0] < gmax[0])
		comp3d[ip[2]-1][ip[1]][ip[0]] = ONFRONT;
	    if (ip[2] < gmax[2] && ip[0] >= 1)
		comp3d[ip[2]][ip[1]][ip[0]-1] = ONFRONT;
	    if (ip[0] < gmax[0] && ip[2] < gmax[2])
		comp3d[ip[2]][ip[1]][ip[0]] = ONFRONT;
	}
	else if (dir == UPPER)
	{
	    if(ip[1] >= 1 && ip[0] >= 1)
		comp3d[ip[2]][ip[1]-1][ip[0]-1] = ONFRONT;
	    if (ip[1] >= 1 && ip[0] < gmax[0])
		comp3d[ip[2]][ip[1]-1][ip[0]] = ONFRONT;
	    if (ip[1] < gmax[1] && ip[0] >= 1)
		comp3d[ip[2]][ip[1]][ip[0]-1] = ONFRONT;
	    if (ip[1] < gmax[1] && ip[0] < gmax[0])
		comp3d[ip[2]][ip[1]][ip[0]] = ONFRONT;
	}
	if (debugging("comp_crx"))
	{
	    int ipmin[3],ipmax[3];
	    for (l = 0; l < 3; ++l)
	    {
		ipmin[l] = ip[l] - 5;
		ipmax[l] = ip[l] + 5;
	    }
	    (void) printf("X-view of grid components:\n");
	    show_grid_components(ipmin,ipmax,0,intfc);
	    (void) printf("Z-view of grid components:\n");
	    show_grid_components(ipmin,ipmax,2,intfc);
	}
	
	DEBUG_LEAVE(fill_missing_crx)
	return num_crx_fill+1;
}	/* end fill_missing_crx */

EXPORT	void adjust_crossings(
	int      *smin,
	int      *smax,
	INTERFACE *intfc)
{
	RECT_GRID      *gr = &topological_grid(intfc);
	int 	       ix,iy,iz;
	GRID_DIRECTION dir[3] = {EAST,NORTH,UPPER};
	int 	       ip[3],i,j,k,l,m,nc,list1;
	CRXING 	       *crx1;
	double 	       grid_crds;
	double 	       *L = gr->L;
	double 	       *h = gr->h;
	Table	       *T = table_of_interface(intfc);
	int		*gmax = gr->gmax;

	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            ip[0] = ix;
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && dir[i] == EAST)
	                    continue;
	                if (iy == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (iz == smax[2] && dir[i] == UPPER)
	                    continue;
	                k = seg_index3d(ix,iy,iz,dir[i],gmax);
	                nc = T->seg_crx_count[k];
			if (nc != 0)
			{
	                    list1 = T->seg_crx_lists[k][0];
	                    crx1 = T->crx_store+list1;
	                    grid_crds = L[i] + ip[i]*h[i];
			    adjust_for_min_spacing(crx1,grid_crds,h,nc,i);
			} 
	            }
	        }
	    }
	}
}		/*end adjust_crossings */

	
 	void print_edge_crossings(
	int      *smin,
	int      *smax,
	INTERFACE *intfc)
{
	RECT_GRID      *gr = &topological_grid(intfc);
	int 	       ix,iy,iz;
	GRID_DIRECTION 	dir[6] = {WEST,EAST,SOUTH,NORTH,LOWER,UPPER};
	int 	       ip[3],i,k,j,m,nc,list1, d0, d1;
	CRXING 	       *crx1;
	double 	       grid_crds;
	double 	       *L = gr->L;
	double 	       *h = gr->h;
	Table	       *T = table_of_interface(intfc);
	COMPONENT      *comp = T->components;
	int		*gmax = gr->gmax;

	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            ip[0] = ix;
	            for (i = 0; i < 6; ++i)
	            {
	                if (ix == smax[0] && dir[i] == EAST)
	                    continue;
	                if (iy == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (iz == smax[2] && dir[i] == UPPER)
	                    continue;
			if (ix == smin[0] && dir[i] == WEST)
	                    continue;
	                if (iy == smin[1] && dir[i] == SOUTH)
	                    continue;
	                if (iz == smin[2] && dir[i] == LOWER)
	                    continue;

	                k = seg_index3d(ix,iy,iz,dir[i],gmax);
	                nc = T->seg_crx_count[k];

			/*if(dir[i] != UPPER) */
			/*    continue; */
			if( (ip[0] == 11 && ip[1] == 19 && ip[2] == 24 && pp_mynode() == 9) ||
			    (ip[0] == 5 && ip[1] == 8 && ip[2] == 2 && pp_mynode() == 13) )
			/*if (nc != 0  && ix == 15 && iy == 18 && iz == 13 && dir[i] ==UPPER) */
			{
			    print_int_vector("#ip found", ip, 3, "\n");
			    
			    /*d0 = d_index3d(ix,iy,iz,gmax); */
			    /*d1 = d_index3d(ix,iy+1,iz,gmax); */
			    /*if(debugging("tst_edge")) */
			    /*    printf("i = %d  nc = %d  ef = %d  comp = %d  %d\n",i, nc, T->edge_flag[k], comp[d0], comp[d1]); */

			    for(j=0; j<nc; j++)
			    {
			        list1 = T->seg_crx_lists[k][j];
	                        crx1 = T->crx_store+list1;
	                        /*grid_crds = L[i] + ip[i]*h[i]; */
	                        /*adjust_for_min_spacing(crx1,grid_crds,h,nc,i); */
	                        print_wall_crx("fc ", ip, dir[i], k, crx1);
			    }
			} 
	            }
	        }
	    }
	}
}		/*end adjust_crossings */

LOCAL	void eliminate_same_crossings(
	int      *smin,
	int      *smax,
	INTERFACE *intfc)
{
	RECT_GRID      *gr = &topological_grid(intfc);
	int 	       ix,iy,iz;
	GRID_DIRECTION dir[3] = {EAST,NORTH,UPPER};
	int 	       ip[3],i,k,l,m,nc,list1;
	CRXING 	       *crx1;
	double 	       grid_crds;
	double 	       *L = gr->L;
	double 	       *h = gr->h;
	Table	       *T = table_of_interface(intfc);
	int	       *gmax = gr->gmax;

	for (iz = smin[2]; iz <= smax[2]; ++iz)
	{
	    ip[2] = iz;
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        ip[1] = iy;
	        for (ix = smin[0]; ix <= smax[0]; ++ix)
	        {
	            ip[0] = ix;
	            for (i = 0; i < 3; ++i)
	            {
	                if (ix == smax[0] && dir[i] == EAST)
	                    continue;
	                if (iy == smax[1] && dir[i] == NORTH)
	                    continue;
	                if (iz == smax[2] && dir[i] == UPPER)
	                    continue;
	                k = seg_index3d(ix,iy,iz,dir[i],gmax);
	                nc = T->seg_crx_count[k];
	                if (nc != 0)
	                {
			    for (l = 0; l < nc; ++l)
	                    {
	                        list1 = T->seg_crx_lists[k][l];
	                        crx1 = T->crx_store+list1;
	                        if (crx1->lcomp == NO_COMP)
	                        {
	                            for (m = l; m < nc - 1; ++m)
	                                T->seg_crx_lists[k][m] =
	                                        T->seg_crx_lists[k][m+1];
	                            --l;
	                            --nc;
	                            --T->seg_crx_count[k];
	                        }
	                    }
			    if (nc != 0)
			    {
	                        list1 = T->seg_crx_lists[k][0];
	                        crx1 = T->crx_store+list1;
	                        grid_crds = L[i] + ip[i]*h[i];
	                        adjust_for_min_spacing(crx1,grid_crds,h,nc,i);
			    }
	                }
	            }
	        }
	    }
	}
}		/*end eliminate_same_crossings*/

LOCAL void adjust_for_min_spacing(
	CRXING		*crxings,
	double		crds_start,
	double		*h,
	int		n_crx,
	int		dir)
{
	int	i;
	double	crds_end;
	double	mgs = 0.004*h[dir];/*TOLERANCE*/
	double   pmin, pmax;
	double   ps=HUGE, pe=-HUGE, nps, npe, m, b;

	crds_end = crds_start + h[dir];
	pmin = crds_start + mgs;
	pmax = crds_end - mgs;
	for (i = 0; i < n_crx; i++)
	{
	      ps = min(Coords(crxings[i].pt)[dir],ps);
	      pe = max(Coords(crxings[i].pt)[dir],pe);
	}
	if ((pmin <= ps) && (pe <= pmax))
	    return;
	if (n_crx == 1)
	{
	    if (ps <= pmin)
	        Coords(crxings[0].pt)[dir] = pmin;
	    if (pe >= pmax)
	        Coords(crxings[0].pt)[dir] = pmax;
	    return;
	}
	nps = (pmax - pmin)/h[dir];

	b = Coords(crxings[0].pt)[dir];
	if (pe <= pmin)
	{
	    for (i = 0; i < n_crx; i++)
		Coords(crxings[i].pt)[dir] = 
				pmin + nps*(Coords(crxings[i].pt)[dir] - b);
	    return;
	}

	b = Coords(crxings[n_crx-1].pt)[dir];
	if (ps >= pmax)
	{
	    for (i = 0; i < n_crx; i++)
		Coords(crxings[i].pt)[dir] = 
				pmax - nps*(b - Coords(crxings[i].pt)[dir]);
	    return;
	}
	
	nps = max(ps,pmin);
	npe = min(pe,pmax);
	m = (npe - nps)/(pe - ps);
	b = (nps*pe - npe*ps)/(pe - ps);
	for (i = 0; i < n_crx; ++i)
	{    
	    Coords(crxings[i].pt)[dir] = m*Coords(crxings[i].pt)[dir] + b;
	}
}		/*end adjust_for_min_spacing*/

LOCAL	boolean check_comp_at(
	int            *ip,
	GRID_DIRECTION dir,
	INTERFACE      *intfc,
	int            *smin,
	int            *smax,
	boolean	       vocal)
{
	RECT_GRID	gr = topological_grid(intfc);
	Table		*T = table_of_interface(intfc);
	COMPONENT 	*comp = T->components;
	int 		*gmax = gr.gmax;
	int 		ipn[MAXD];
	int 		k,nc,list;
	CRXING 		*crx;
	double fp[MAXD];

	if (next_ip_in_dir(ip,dir,ipn,smin,smax) != YES)
	    return YES;

	k = seg_index3d(ip[0],ip[1],ip[2],dir,gmax);
	nc = T->seg_crx_count[k];
	if (nc == 0)
	{
	    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] ==
		comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)])
	        return YES;
	    else
	    {
		if (vocal)
		{
		    rect_grid_corner(&gr,ip,fp);
	            (void) printf("\nWARNING in check_comp_at(), "
	                      "Inconsistent component at "
	                      "neighboring grid point\n");
	            (void) printf("  ip  = ( %7d %7d %7d ) comp = %2d  ",
			      ip[0],ip[1],ip[2],
			      comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    (void) printf("fp  = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
		    rect_grid_corner(&gr,ipn,fp);
	            (void) printf("  ipn = ( %7d %7d %7d ) comp = %2d  ",
			      ipn[0],ipn[1],ipn[2],
			      comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    (void) printf("fpn = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
	            (void) printf("  Number of crossings = %d\n\n",nc);
		}
		return NO;
	    }
	}
	else if (nc == 1)
	{
	    list = T->seg_crx_lists[k][0];
	    crx = &(T->crx_store[list]);
	    switch (dir)
	    {
	    case EAST:
	    case NORTH:
	    case UPPER:
	        if (crx->lcomp != comp[d_index3d(ip[0],ip[1],ip[2],gmax)])
	        {
		    if (vocal)
		    {
	            	(void) printf("WARNING in check_comp_at(), "
	                          "lower component of cross does "
	                          "not agree with grid\n");
		    	rect_grid_corner(&gr,ip,fp);
	            	(void) printf("  ip  = ( %7d %7d %7d ) comp = %2d  ",
			      ip[0],ip[1],ip[2],
			      comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	(void) printf("fp  = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
		    	rect_grid_corner(&gr,ipn,fp);
	            	(void) printf("  ipn = ( %7d %7d %7d ) comp = %2d  ",
			      ipn[0],ipn[1],ipn[2],
			      comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    	(void) printf("fpn = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
	            	(void) printf("  Number of crossings = %d\n\n",nc);
			(void) printf("crx->lcomp = %d  crx->ucomp = %d\n",
					crx->lcomp,crx->ucomp);
		    }
	            return NO;
	        }
	        if (crx->ucomp != comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)])
	        {
		    if (vocal)
		    {
	            	(void) printf("WARNING in check_comp_at(), "
	                          "upper component of cross does "
	                          "not agree with grid\n");
		    	rect_grid_corner(&gr,ip,fp);
	            	(void) printf("  ip  = ( %7d %7d %7d ) comp = %2d  ",
			      ip[0],ip[1],ip[2],
			      comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	(void) printf("fp  = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
		    	rect_grid_corner(&gr,ipn,fp);
	            	(void) printf("  ipn = ( %7d %7d %7d ) comp = %2d  ",
			      ipn[0],ipn[1],ipn[2],
			      comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    	(void) printf("fpn = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
	            	(void) printf("  Number of crossings = %d\n\n",nc);
			(void) printf("crx->lcomp = %d  crx->ucomp = %d\n",
					crx->lcomp,crx->ucomp);
		    }
	            return NO;
	        }
		/*
	        if (crx->ucomp == crx->lcomp)
	        {
		    if (vocal)
		    {
	            	(void) printf("WARNING in check_comp_at(), "
	                          "lower component of cross equal "
	                          "to upper component\n");
		    	rect_grid_corner(&gr,ip,fp);
	            	(void) printf("  ip  = ( %7d %7d %7d ) comp = %2d  ",
			      ip[0],ip[1],ip[2],
			      comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	(void) printf("fp  = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
		    	rect_grid_corner(&gr,ipn,fp);
	            	(void) printf("  ipn = ( %7d %7d %7d ) comp = %2d  ",
			      ipn[0],ipn[1],ipn[2],
			      comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    	(void) printf("fpn = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
	            	(void) printf("  Number of crossings = %d\n\n",nc);
			(void) printf("crx->lcomp = %d  crx->ucomp = %d\n",
					crx->lcomp,crx->ucomp);
		    }
	            return NO;
	        }
		*/
	        return YES;
	    case WEST:
	    case SOUTH:
	    case LOWER:
	        if (crx->ucomp != comp[d_index3d(ip[0],ip[1],ip[2],gmax)])
	        {
		    if (vocal)
		    {
	            	(void) printf("WARNING in check_comp_at(), "
	                          "upper component of cross does "
	                          "not agree with grid\n");
		    	rect_grid_corner(&gr,ip,fp);
	            	(void) printf("  ip  = ( %7d %7d %7d ) comp = %2d  ",
			      ip[0],ip[1],ip[2],
			      comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	(void) printf("fp  = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
		    	rect_grid_corner(&gr,ipn,fp);
	            	(void) printf("  ipn = ( %7d %7d %7d ) comp = %2d  ",
			      ipn[0],ipn[1],ipn[2],
			      comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    	(void) printf("fpn = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
	            	(void) printf("  Number of crossings = %d\n\n",nc);
		    }
	            return NO;
	        }
	        if (crx->lcomp != comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)])
	        {
		    if (vocal)
		    {
	            	(void) printf("WARNING in check_comp_at(), "
	                          "lower component of cross does "
	                          "not agree with grid\n");
		    	rect_grid_corner(&gr,ip,fp);
	            	(void) printf("  ip  = ( %7d %7d %7d ) comp = %2d  ",
			      ip[0],ip[1],ip[2],
			      comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	(void) printf("fp  = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
		    	rect_grid_corner(&gr,ipn,fp);
	            	(void) printf("  ipn = ( %7d %7d %7d ) comp = %2d  ",
			      ipn[0],ipn[1],ipn[2],
			      comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    	(void) printf("fpn = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
	            	(void) printf("  Number of crossings = %d\n\n",nc);
		    }
	            return NO;
	        }
		/*
	        if (crx->ucomp == crx->lcomp)
	        {
		    if (vocal)
		    {
	            	(void) printf("WARNING in check_comp_at(), "
	                          "lower component of cross equal "
	                          "to upper component\n");
		    	rect_grid_corner(&gr,ip,fp);
	            	(void) printf("  ip  = ( %7d %7d %7d ) comp = %2d  ",
			      ip[0],ip[1],ip[2],
			      comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	(void) printf("fp  = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
		    	rect_grid_corner(&gr,ipn,fp);
	            	(void) printf("  ipn = ( %7d %7d %7d ) comp = %2d  ",
			      ipn[0],ipn[1],ipn[2],
			      comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    	(void) printf("fpn = ( %g %g %g )\n",
			      fp[0],fp[1],fp[2]);
	            	(void) printf("  Number of crossings = %d\n\n",nc);
		    }
	            return NO;
	        }
		*/
	        return YES;
	    default:
		if (vocal)
	            (void) printf("WARNING in check_comp_at(), "
	                      "invalid dir %d\n",dir);
	        return NO;
	    }
	}
	else
	{
	    GRID_PT 	gp;
	    int 	i;
	    COMPONENT 	next_comp;

	    for (i = 0; i < MAXD; ++i) 
	        gp.ip[i] = ip[i];
	    gp.cur_dir = dir;
	    gp.comp = comp[d_index3d(ip[0],ip[1],ip[2],gmax)];
	    
	    if (edge_comp_walk(&gp,intfc,&next_comp,MULTIPLE) == UNPHYSICAL_EDGE)
	    {
		if (vocal)
		{
	            (void) printf("WARNING in check_comp_at() UNPHYSICAL_EDGE "
			      "detected.\n");
		    print_edge_crxings(&gp,intfc);
		}
		return NO;
	    }
	    return YES;
	}
}		/*end check_comp_at*/

LOCAL	boolean physical_edge(
	int            *ip,
	GRID_DIRECTION dir,
	INTERFACE      *intfc,
	int            *smin,
	int            *smax,
	CRX_TYPE        crx_type)
{
	RECT_GRID	gr = topological_grid(intfc);
	Table		*T = table_of_interface(intfc);
	COMPONENT 	*comp = T->components;
	int 		*gmax = gr.gmax;
	int 		ipn[MAXD];
	int 		k,nc,list;
	CRXING 		*crx;
	double fp[MAXD];

	if (next_ip_in_dir(ip,dir,ipn,smin,smax) != YES)
	    return YES;

	k = seg_index3d(ip[0],ip[1],ip[2],dir,gmax);
	nc = T->seg_crx_count[k];
	if (nc == 0)
	{
	    if (comp[d_index3d(ip[0],ip[1],ip[2],gmax)] ==
		comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)])
	        return YES;
	    else if (comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)] == NO_COMP)
	    	return YES;
	    else
            {
		return NO;
	    }
	}
	else if (nc == 1)
	{
	    list = T->seg_crx_lists[k][0];
	    crx = &(T->crx_store[list]);
	    switch (dir)
	    {
	    case EAST:
	    case NORTH:
	    case UPPER:
	        if (crx->lcomp != comp[d_index3d(ip[0],ip[1],ip[2],gmax)] ||
		    (crx->ucomp == crx->lcomp && 
		    positive_component(crx->hs) != negative_component(crx->hs)))
		{
		    if(debugging("bad_comp"))
		    {
		    	printf("ip  = %d %d %d  comp = %d\n",ip[0],ip[1],ip[2],
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	printf("lucomp %d %d\n", crx->lcomp, crx->ucomp);
		    	printf("ipn = %d %d %d  comp = %d\n",ipn[0],ipn[1],
			    ipn[2],comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    }
	            return NO;
		}
	        return YES;
	    case WEST:
	    case SOUTH:
	    case LOWER:
	        if (crx->ucomp != comp[d_index3d(ip[0],ip[1],ip[2],gmax)] ||
		    (crx->ucomp == crx->lcomp &&
                    positive_component(crx->hs) != negative_component(crx->hs)))
		{
		    if(debugging("bad_comp"))
		    {
		    	printf("ip  = %d %d %d  comp = %d\n",ip[0],ip[1],ip[2],
		    	    comp[d_index3d(ip[0],ip[1],ip[2],gmax)]);
		    	printf("lucomp %d %d\n", crx->lcomp, crx->ucomp);
		    	printf("ipn = %d %d %d  comp = %d\n",ipn[0],ipn[1],
			    ipn[2],comp[d_index3d(ipn[0],ipn[1],ipn[2],gmax)]);
		    }
	            return NO;
		}
	        return YES;
	    default:
	        return NO;
	    }
	}
	else
	{
	    /* need to fix it. */
	    GRID_PT 	gp;
	    int 	i;
	    COMPONENT 	next_comp;

	    if (crx_type == SINGLE)
	    	return NO;
	    for (i = 0; i < MAXD; ++i) 
	        gp.ip[i] = ip[i];
	    gp.cur_dir = dir;
	    gp.comp = comp[d_index3d(ip[0],ip[1],ip[2],gmax)];
	    
	    if(debugging("bad_comp"))
	    {
		printf("ip = %d %d %d  dir = %s  nc = %d\n",
			ip[0],ip[1],ip[2],grid_direction_name(dir),nc);
	    }
	    if (edge_comp_walk(&gp,intfc,&next_comp,MULTIPLE) == 
				UNPHYSICAL_EDGE)
		return NO;
	    return YES;
	}
}		/*end physical_edge*/

LOCAL 	EDGE_TYPE edge_comp_walk(
	GRID_PT 	*gp,
	INTERFACE	*intfc,
	COMPONENT	*next_comp,
	CRX_TYPE	crx_type)
{
	int 		*ip = gp->ip;
	GRID_DIRECTION 	dir = gp->cur_dir;
	int 		seg_index,crx_count,list;
	RECT_GRID 	*expanded_dual_grid = &topological_grid(intfc);
	int   		i,axis,polarity;
	double 		start[MAXD],end[MAXD];
	COMPONENT 	current_comp;
	CRXING    	**crxing;
	CRX_SORT  	*crx_sort;
	double		**crxing_point;
	int		unphysical_crxing_count = 0,unphys_crxing;
	EDGE_TYPE	status_of_edge;
	Table		*T = table_of_interface(intfc);
	int		*gmax = expanded_dual_grid->gmax;

        seg_index = seg_index3d(ip[0],ip[1],ip[2],dir,gmax);
	crx_count = T->seg_crx_count[seg_index];
	set_grid_crx_edge(expanded_dual_grid,ip,
			  dir,start,end,&axis,&polarity);

	if (crx_count == 0)
	{
	    *next_comp = gp->comp;
	    return PHYSICAL_EDGE;
	}

        /* track the components through the crossings of the edge */

	uni_array(&crxing,crx_count,sizeof(CRXING *));
	uni_array(&crxing_point,crx_count,sizeof(double *));
	uni_array(&crx_sort,crx_count,sizeof(CRX_SORT));
	for (i = 0; i < crx_count; ++i)
	{
	    list = T->seg_crx_lists[seg_index][i];
	    crx_sort[i].crx = crxing[i] = &(T->crx_store[list]);
	    crxing_point[i] = Coords(crxing[i]->pt);
	    crx_sort[i].compare_coord = &crxing_point[i][axis];
	}

	if (debugging("tolerance"))
	    (void) crossings_on_edge(crxing_point,start,end,axis,crx_count);

	if (crx_count == 1)
	{
	    if (gp->comp == this_side_comp_at_crx(gp,crxing[0]))
	    {
	        *next_comp = next_side_comp_at_crx(gp,crxing[0]);
		status_of_edge = PHYSICAL_EDGE;
	    }
	    else
	    {
	        status_of_edge = UNPHYSICAL_EDGE;
	    }
	    free_these(3,crxing,crx_sort,crxing_point);
	    return status_of_edge;
	}

	if (polarity == + 1) 
	    qsort((POINTER)crx_sort,crx_count,sizeof(CRX_SORT),crx_ascend);
	else
	    qsort((POINTER)crx_sort,crx_count,sizeof(CRX_SORT),crx_descend);

	current_comp = gp->comp;

	for (i = 0; i < crx_count; ++i)
	{
	    if(debugging("sect_comp"))
		printf("cur_comp %d,  this %d,  next %d un_cnt %d\n", current_comp, 
			this_side_comp_at_crx(gp,crx_sort[i].crx),
			next_side_comp_at_crx(gp,crx_sort[i].crx), 
			unphysical_crxing_count);

	    if (current_comp != this_side_comp_at_crx(gp,crx_sort[i].crx))
	    {
	        if (debugging("multi_crx"))
		{
		    (void) printf("WARNING in edge_comp_walk(), "
				  "unphysical crossing detected, count = %d\n",
				  unphysical_crxing_count);
		    (void) printf("      crossing %d  current_comp = %d  "
				  "grid_point->comp = %d  "
				  "this_side_comp_at_crx() = %d\n",
				  i,current_comp,gp->comp,
				  this_side_comp_at_crx(gp,crx_sort[i].crx));
		}
		++unphysical_crxing_count;
		unphys_crxing = i;
	    }
	    else
	        current_comp = next_side_comp_at_crx(gp,crx_sort[i].crx);
	}

	if (unphysical_crxing_count == 0) 
	{
	    if (crx_type == SINGLE)
	    {
		if (current_comp == this_side_comp_at_crx(gp,crx_sort[0].crx))
		{
	    	    *next_comp = current_comp;
		    T->seg_crx_count[seg_index] = 0;
		    status_of_edge = PHYSICAL_EDGE;
		}
		else if (current_comp == next_side_comp_at_crx(gp,
							       crx_sort[0].crx))
		{
	    	    *next_comp = current_comp;
		    T->seg_crx_count[seg_index] = 1;
		    status_of_edge = PHYSICAL_EDGE;
		}
		else
		    status_of_edge = UNPHYSICAL_EDGE;
	    }
	    else
	    {
		status_of_edge = PHYSICAL_EDGE;
	    	*next_comp = current_comp;
	    }
	}
	else 
	{
	    status_of_edge = UNPHYSICAL_EDGE;
	    /*if (debugging("multi_crx")) */
	    /*    multi_crx_debug(crx_sort,crx_count,gp,start,end,unphys_crxing); */
	}

	free_these(3,crxing,crx_sort,crxing_point);
        return status_of_edge;
}		/*end edge_comp_walk*/

LOCAL 	void 	print_edge_crxings(
	GRID_PT  *gp,
	INTERFACE *intfc)
{
	CRX_SORT  	  *crx_sort;
	CRXING    	  **crxing;
	RECT_GRID 	  *expanded_dual_grid = &topological_grid(intfc);
	double 		  start[MAXD],end[MAXD];
	double		  **crxing_point;
	int 		  *ip = gp->ip;
	GRID_DIRECTION 	  dir = gp->cur_dir;
	int 		  seg_index,crx_count,list;
	int   		  i,axis,polarity;
	static const char *sdir[] = { "X","Y","Z" };
	Table		  *T = table_of_interface(intfc);
	int		  *gmax = expanded_dual_grid->gmax;

        seg_index = seg_index3d(ip[0],ip[1],ip[2],dir,gmax);
	crx_count = T->seg_crx_count[seg_index];
	set_grid_crx_edge(expanded_dual_grid,ip,
			  dir,start,end,&axis,&polarity);

	(void) printf("ip  ( %3d %3d %3d )  comp = %2d crx_cnt = %2d  ",
		      ip[0],ip[1],ip[2],gp->comp,crx_count);
	(void) printf("dir = %s\n",grid_direction_name(dir));
	(void) printf("\nstart = ( %g %g %g)  "
		      "end = ( %g %g %g )  axis = %s  polarity = %d\n",
		      start[0],start[1],start[2],end[0],end[1],end[2],
		      sdir[axis],polarity);

	if (crx_count == 0)
	{
	    (void) printf("edge has no crossings\n");
	    return;
	}

        /* track the components through the crossings of the edge */

	uni_array(&crxing,crx_count,sizeof(CRXING *));
	uni_array(&crxing_point,crx_count,sizeof(double *));
	uni_array(&crx_sort,crx_count,sizeof(CRX_SORT));
	for (i = 0; i < crx_count; ++i)
	{
	    list = T->seg_crx_lists[seg_index][i];
	    crx_sort[i].crx = crxing[i] = &(T->crx_store[list]);
	    crxing_point[i] = Coords(crxing[i]->pt);
	    crx_sort[i].compare_coord = &crxing_point[i][axis];
	}
	
	if (polarity == + 1) 
	    qsort((POINTER)crx_sort,crx_count,sizeof(CRX_SORT),crx_ascend);
	else
	    qsort((POINTER)crx_sort,crx_count,sizeof(CRX_SORT),crx_descend);

	print_crx_sort(crx_sort,crx_count,gp);

	free(crxing);
	free(crx_sort);
	free(crxing_point);
}		/*end print_edge_crxings*/

LOCAL 	int 	crossings_on_edge(
	double 		**crossing,
	double 		*start,
	double 		*end,
	int 		axis,
	int 		crx_count)
{
	double 	   lesser,greater;
	int   	   i,j,i_crx;
	static const char *sdir[] = {"X","Y","Z"};
  
	lesser = min(start[axis],end[axis]);
	greater = max(start[axis],end[axis]);

	for (i_crx = 0; i_crx < crx_count; ++i_crx)
	{
	    for (i = 1; i < 3; ++i)
	    {
	        j = (axis + i)%3;
		if (crossing[i_crx][j] != start[j])
		{
		    (void) printf("TOLERANCE in crossings_on_edge(), crossing "
				  "not on grid line, axis = %d  delta = %g\n",
				  j,crossing[i_crx][j]-start[j]);
		    (void) printf("         number of crossings = %d  "
				  "i_crx = %d  axis = %1s  lesser = %g  "
				  "greater = %g\n",
				  crx_count,i_crx,sdir[axis],lesser,greater);
		    (void) printf("         start =    ( %g %g %g )  "
				  "end = ( %g %g %g )\n",
				  start[0],start[1],start[2],
				  end[0],end[1],end[2]);
		    (void) printf("         crossing = ( %g %g %g )\n",
				  crossing[i_crx][0],crossing[i_crx][1],
				  crossing[i_crx][2]);
		    (void) printf("         crossing - start = ( %g %g %g )\n",
				  crossing[i_crx][0]-start[0],
				  crossing[i_crx][1]-start[1],
				  crossing[i_crx][2]-start[2]);
		    return NO;
		}
	    }

	    if (crossing[i_crx][axis]<lesser || crossing[i_crx][axis]>greater)
            {
	        (void) printf("TOLERANCE in crossings_on_edge(), "
			      "crossing not between grid points.\n");
		(void) printf("       number of crossings = %d  i_crx = %d  "
			      "axis = %1s  lesser = %g  greater = %g\n",
			      crx_count,i_crx,sdir[axis],lesser,greater);
		(void) printf("       start =    ( %g %g %g )  "
			      "end = ( %g %g %g )\n",
			      start[0],start[1],start[2],end[0],end[1],end[2]);
		(void) printf("       crossing = ( %g %g %g )\n",
			      crossing[i_crx][0],crossing[i_crx][1],
			      crossing[i_crx][2]);
		return NO;
	    }
	}
	return YES;
}  		/*end crossings_on_edge*/

LOCAL 	void 	print_crx_sort(
	CRX_SORT 	*crx_sort,
	int 		nc,
	GRID_PT		*gp)
{
  	int i;
	(void) printf("%2s  %8s  %30s  %3s  %3s  %3s  %3s   %5s\n","i",
		      "compare","crx->pt          ","l_c","u_c","ths","nxt",
		      "delta");
	for (i = 0; i < nc; ++i)
	{
	    (void) printf("%2d  %g  ( %g %g %g )  "
			  "%3d  %3d  %3d  %3d",i,
			  *(crx_sort[i].compare_coord),
			  Coords(crx_sort[i].crx->pt)[0],
			  Coords(crx_sort[i].crx->pt)[1],
			  Coords(crx_sort[i].crx->pt)[2],
			  crx_sort[i].crx->lcomp,crx_sort[i].crx->ucomp,
			  this_side_comp_at_crx(gp,crx_sort[i].crx),
			  next_side_comp_at_crx(gp,crx_sort[i].crx));
	    if (i < nc-1) 
	        (void) printf(" %g\n",*(crx_sort[i].compare_coord) - 
			      *(crx_sort[i+1].compare_coord));
	    else
	        (void) printf("\n");
	}
} 		/*end print_crx_sort*/

LOCAL 	void 	set_grid_crx_edge(
	RECT_GRID 	*rect_grid,
	int 		*ip,
	GRID_DIRECTION 	dir,
	double 		*start,
	double 		*end,
	int 		*axis,
	int 		*polarity)
{
	int 	i;
	double 	L[MAXD],h[MAXD];

	for (i = 0; i < 3; ++i)
	{
	    L[i] = rect_grid->L[i];
	    h[i] = rect_grid->h[i];
	    end[i] = start[i] = L[i] + ip[i] * h[i];
	}


	switch (dir)
	{
	case EAST: 
	    end[0] = start[0] + h[0];
	    *axis = 0;
	    *polarity = + 1;
	    break;
	case WEST:
	    end[0] = start[0] - h[0];
	    *axis = 0;
	    *polarity = - 1;
	    break;
	case NORTH:
	    end[1] = start[1] + h[1];
	    *axis = 1;
	    *polarity = + 1;
	    break;
	case SOUTH:
	    end[1] = start[1] - h[1];
	    *axis = 1;
	    *polarity = - 1;
	    break;
	case UPPER:
	    end[2] = start[2] + h[2];
	    *axis = 2;
	    *polarity = + 1;
	    break;
	case LOWER:
	    end[2] = start[2] - h[2];
	    *axis = 2;
	    *polarity = - 1;
	    break;
        default:
	    screen("ERROR in set_grid_crx_edge(), "
		   "unknown direction = %d\n",dir);
	    clean_up(ERROR);
	}
}  		/*end set_grid_crx_edge*/

LOCAL	void	multi_crx_debug(
	CRX_SORT	*crx_sort,
	int		crx_count,
	GRID_PT		*gp,
	double		*start,
	double		*end,
	int		unphys_crxing)
{
  	double		**polyline;
	double		h;
	int 		i, j;
	SURFACE		*surf;
	char 		fname[256];
	static int	error_cnt = 0;

	print_crx_sort(crx_sort,crx_count,gp);
	bi_array(&polyline,crx_count+2,3,sizeof(double));
	for (j = 0; j < 3; ++j)
	{
	    polyline[0][j] = start[j];
	    polyline[crx_count+1][j] = end[j];
	}
	for (i = 0; i < crx_count; ++i)
	    for (j = 0; j < 3; ++j)
	        polyline[i+1][j] = Coords(crx_sort[i].crx->pt)[j];
	(void) sprintf(fname,"edge_crx_%d",error_cnt);
	gview_polyline("edge_comp_walk",fname,polyline,crx_count+2,pYELLOW,3);

	for (i = 0; i < crx_count+1; ++i)
	{
	    (void) sprintf(fname,"seg_of_crx_%d_error%d",i,error_cnt);
	    gview_polyline("edge_comp_walk",fname,polyline+i,2,pWHITE,6);
	}

	for (h = 0, i = 0; i < 3; ++i)
	    h += fabs(start[i]-end[i]);
	surf = crx_sort[0].crx->hs->obj.s;
	(void) sprintf(fname,"surf_%d",error_cnt);
	gview_local_surface(surf,"edge_comp_walk",fname,pBLUE,
			    Coords(crx_sort[unphys_crxing].crx->pt),
			    0.3*h);/*TOLERANCE*/
	++error_cnt;
	free(polyline);
}		/*end multi_crx_debug*/

LOCAL	int	walk_comp_along_grid_line(
	INTERFACE      *intfc,
	int            *smin,
	int            *smax,
	int            *gmax,
	int            *ip,
	GRID_DIRECTION dir)
{
	Table	  *T = table_of_interface(intfc);
	COMPONENT *comp = T->components;
	COMPONENT current_comp;
	COMPONENT cn;
	CRXING	  *crx;
	int	  *ef = T->edge_flag;
	int 	  ip1[3], ip2[3];
	int	  i,k,nc,list,step;
	boolean	  crx_is_physical = YES;

	for (i = 0; i < 3; ++i)
	    ip1[i] = ip[i];
	current_comp = comp[d_index3d(ip1[0],ip1[1],ip1[2],gmax)];
	step = 0;
	while (next_ip_in_dir(ip1,dir,ip2,smin,smax))
	{
	    k = seg_index3d(ip1[0],ip1[1],ip1[2],dir,gmax);
	    if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
	        break;
	    nc = T->seg_crx_count[k];
	    if (dir == EAST || dir == NORTH || dir == UPPER)
	    {
		for (i = 0; i < nc; ++i)
		{
		    list = T->seg_crx_lists[k][i];
		    crx = &(T->crx_store[list]);
		    if (crx->lcomp == current_comp)
			current_comp = crx->ucomp;
		    else
		    {
			crx_is_physical = NO;
			break;
		    }
		}
	    }
	    else
	    {
		for (i = nc-1; i >= 0; --i)
		{
		    list = T->seg_crx_lists[k][i];
		    crx = &(T->crx_store[list]);
		    if (crx->ucomp == current_comp)
			current_comp = crx->lcomp;
		    else
		    {
			crx_is_physical = NO;
			break;
		    }
		}
	    }
	    if (!crx_is_physical) 
		break;
	    ++step;
            comp[d_index3d(ip2[0],ip2[1],ip2[2],gmax)] = current_comp;
	    if (dir == LOWER || dir == UPPER)
	    {
		if (ip[0] != smin[0])
		{
	    	    k = seg_index3d(ip2[0],ip2[1],ip2[2],WEST,gmax);
		    if(!use_wall_edge()  ||  ef[k] == INSIDE_WALL)
		    {
	    	        nc = T->seg_crx_count[k];
		        if (nc == 0)
		        {
		    	    next_ip_in_dir(ip2,WEST,ip1,smin,smax);
			    if ((comp[d_index3d(ip1[0],ip1[1],ip1[2],gmax)] !=
			         NO_COMP) &&
			        (comp[d_index3d(ip2[0],ip2[1],ip2[2],gmax)] !=
			         comp[d_index3d(ip1[0],ip1[1],ip1[2],gmax)]))
			    {
			        crx_is_physical = NO;
			    }
		        }
		        else
		        {
			    list = T->seg_crx_lists[k][nc-1];
			    crx = T->crx_store+list;
			    if (crx->ucomp != current_comp)
			    {
			        crx_is_physical = NO;
			    }
		        }
		    }
		}
		if (ip[1] != smin[1])
		{
	    	    k = seg_index3d(ip2[0],ip2[1],ip2[2],SOUTH,gmax);
		    if(!use_wall_edge()  ||  ef[k] == INSIDE_WALL)
		    {
	    	        nc = T->seg_crx_count[k];
		        if (nc == 0)
		        {
		    	    next_ip_in_dir(ip2,SOUTH,ip1,smin,smax);
			    if ((comp[d_index3d(ip1[0],ip1[1],ip1[2],gmax)] !=
			         NO_COMP) && 
			        (comp[d_index3d(ip2[0],ip2[1],ip2[2],gmax)] !=
			        comp[d_index3d(ip1[0],ip1[1],ip1[2],gmax)]))
			    {
			        crx_is_physical = NO;
			    }
		        }
		        else
		        {
			    list = T->seg_crx_lists[k][nc-1];
			    crx = T->crx_store+list;
			    if (crx->ucomp != current_comp)
			    {
			        crx_is_physical = NO;
			    }
		        }
		    }
		}
	        if (!crx_is_physical)
		{
		    break;
		}
	    }
	    for (i = 0; i < 3; ++i)
		ip1[i] = ip2[i];
	}
	return step;
}	/* end walk_comp_along_grid_line */

EXPORT	void fill_comps_in_box(
        int      *smin_in,
        int      *smax_in,
        int      *gmax,
        INTERFACE *intfc)
{
        int             k,l,nc,list,step,i;
        int             ip[3], smin[3], smax[3];
	Table		*T = table_of_interface(intfc);
        COMPONENT       *comp = T->components;
	COMPONENT	current_comp;
        COMPONENT       cn;
        CRXING          *crx;
        boolean            end_fill_comps = NO, crx_is_physical = YES;
        boolean            end_min[3],end_max[3];

	ft_assign(smin, smin_in, 3*INT);
	ft_assign(smax, smax_in, 3*INT);
	
	for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
            for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
                for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
                    comp[d_index3d(ip[0],ip[1],ip[2],gmax)] = NO_COMP;

	/*if one face of the box is on the boundary of the domain, we can not approach from that direction. */
	for (i = 0; i < 3; i++)
	{
	    end_min[i] = smin[i] == 0 ? YES : NO;
	    end_max[i] = smax[i] == gmax[i] ? YES : NO;
	}

	/*This part is not necessary because outside the box, components must be consistent, see  */
	/*track_comp_and_repair3d  (The only function calls this function) */
	/*    fill_physical_comps(smin,smax,gmax,intfc); */
	/*    fill_comp_with_component3d(smin,smax,gmax,intfc); */
	/*    record_unphysical_ips   record all the unphysical ips inside the box, so the outside comps are consistent. */
	/*    if the box is on the face of the domain, the following is redundent, because while (!end_fill_comps) will */
	/*    do the same thing.  */

        while (!end_fill_comps)
        {
            for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
                for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
                {
                    crx_is_physical = YES; 
                    if (smin[2] != 0 &&
                        comp[d_index3d(ip[0],ip[1],smin[2]-1,gmax)] != NO_COMP)
                    {
                        k = seg_index3d(ip[0],ip[1],smin[2],LOWER,gmax);
                        nc = T->seg_crx_count[k];
                        if (nc == 0)
                            comp[d_index3d(ip[0],ip[1],smin[2],gmax)] =
                              comp[d_index3d(ip[0],ip[1],smin[2]-1,gmax)];
                        else if (nc == 1)
                        {
                            list = T->seg_crx_lists[k][0];
                            crx = T->crx_store+list;
                            if (comp[d_index3d(ip[0],ip[1],smin[2]-1,gmax)]
                                == crx->lcomp)
                                comp[d_index3d(ip[0],ip[1],smin[2],gmax)] =
                                                crx->ucomp;
                            else
                                end_min[2] = YES;
                        }
                        else
                            end_min[2] = YES;
                    }
                    crx_is_physical = YES;
		    if (smax[2] != gmax[2] &&
                        comp[d_index3d(ip[0],ip[1],smax[2]+1,gmax)] != NO_COMP)
                    {
                         k = seg_index3d(ip[0],ip[1],smax[2],UPPER,gmax);
                         nc = T->seg_crx_count[k];
                         if (nc == 0)
                             comp[d_index3d(ip[0],ip[1],smax[2],gmax)] =
                               comp[d_index3d(ip[0],ip[1],smax[2]+1,gmax)];
                        else if (nc ==1)
                        {
                            list = T->seg_crx_lists[k][0];
                            crx = T->crx_store+list;
                            if (comp[d_index3d(ip[0],ip[1],smax[2]+1,gmax)]
                                == crx->ucomp)
                                comp[d_index3d(ip[0],ip[1],smax[2],gmax)] =
                                                crx->lcomp;
                            else
                                end_max[2] = YES;
                        }
                        else
                            end_max[2] = YES;
                    }
                }

            for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
                for (ip[1] = smin[1]; ip[1] <= smax[1]; ++ip[1])
                {
                    crx_is_physical = YES;
		    if (smin[0] != 0 &&
                        comp[d_index3d(smin[0]-1,ip[1],ip[2],gmax)] != NO_COMP)
                    {
                        k = seg_index3d(smin[0],ip[1],ip[2],WEST,gmax);
                        nc = T->seg_crx_count[k];
                        if (nc == 0)
                            comp[d_index3d(smin[0],ip[1],ip[2],gmax)] =
                              comp[d_index3d(smin[0]-1,ip[1],ip[2],gmax)];
                        else if (nc == 1)
                        {
                            list = T->seg_crx_lists[k][0];
                            crx = T->crx_store+list;
                            if (comp[d_index3d(smin[0]-1,ip[1],ip[2],gmax)]
                                == crx->lcomp)
                                comp[d_index3d(smin[0],ip[1],ip[2],gmax)] =
                                                crx->ucomp;
                            else
                                end_min[0] = YES;
                        }
                        else
                            end_min[0] = YES;
                    }
                    crx_is_physical = YES;
		    if (smax[0] != gmax[0] &&
                        comp[d_index3d(smax[0]+1,ip[1],ip[2],gmax)] != NO_COMP)
                    {
                        k = seg_index3d(smax[0],ip[1],ip[2],EAST,gmax);
                        nc = T->seg_crx_count[k];
                        if (nc == 0)
                            comp[d_index3d(smax[0],ip[1],ip[2],gmax)] =
                              comp[d_index3d(smax[0]+1,ip[1],ip[2],gmax)];
                        else if (nc ==1)
                        {
                            list = T->seg_crx_lists[k][0];
                            crx = T->crx_store+list;
                            if (comp[d_index3d(smax[0]+1,ip[1],ip[2],gmax)]
                                == crx->ucomp)
                                comp[d_index3d(smax[0],ip[1],ip[2],gmax)] =
                                                crx->lcomp;
                            else
                                end_max[0] = YES;
                        }
                        else
                            end_max[0] = YES;
                    }
                }

            for (ip[0] = smin[0]; ip[0] <= smax[0]; ++ip[0])
                for (ip[2] = smin[2]; ip[2] <= smax[2]; ++ip[2])
                {
                    crx_is_physical = YES;
		    if (smin[1] != 0 &&
                        comp[d_index3d(ip[0],smin[1]-1,ip[2],gmax)] != NO_COMP)
                    {
                        k = seg_index3d(ip[0],smin[1],ip[2],SOUTH,gmax);
                        nc = T->seg_crx_count[k];
                        if (nc == 0)
                            comp[d_index3d(ip[0],smin[1],ip[2],gmax)] =
                              comp[d_index3d(ip[0],smin[1]-1,ip[2],gmax)];
                        else if (nc ==1) 
                        {
                            list = T->seg_crx_lists[k][0];
                            crx = T->crx_store+list;
                            if (comp[d_index3d(ip[0],smin[1]-1,ip[2],gmax)]
                                == crx->lcomp)
                                comp[d_index3d(ip[0],smin[1],ip[2],gmax)] =
                                                crx->ucomp;
                            else
                                end_min[1] = YES;
                        }
                        else
                            end_min[1] = YES;
                    }
                    crx_is_physical = YES;
		    if (smax[1] != gmax[1] &&
                        comp[d_index3d(ip[0],smax[1]+1,ip[2],gmax)] != NO_COMP)
                    {
                        k = seg_index3d(ip[0],smax[1],ip[2],NORTH,gmax);
                        nc = T->seg_crx_count[k];
                        if (nc == 0)
                            comp[d_index3d(ip[0],smax[1],ip[2],gmax)] =
                              comp[d_index3d(ip[0],smax[1]+1,ip[2],gmax)];
                        else if (nc == 1)
                        {
                            list = T->seg_crx_lists[k][0];
                            crx = T->crx_store+list;
                            if (comp[d_index3d(ip[0],smax[1]+1,ip[2],gmax)]
                                == crx->ucomp)
                                comp[d_index3d(ip[0],smax[1],ip[2],gmax)] =
                                                crx->lcomp;
                            else
                                end_max[1] = YES;
                        }
                        else
                            end_max[1] = YES;
                    }
                }
            if (!end_min[0]) smin[0]++;
            if (!end_max[0]) smax[0]--;
            if (smin[0] > smax[0]) break;
            if (!end_min[1]) smin[1]++;
            if (!end_max[1]) smax[1]--;
            if (smin[1] > smax[1]) break;
            if (!end_min[2]) smin[2]++;
            if (!end_max[2]) smax[2]--;
            if (smin[2] > smax[2]) break;
	    if (end_min[0] && end_max[0] && end_min[1] && 
                end_max[1] && end_min[2] && end_max[2])
	        break;
        }
}   /*end fill_comps_in_box*/



LOCAL	int	rm_unphy_crx_along_grid_line(
	INTERFACE      *intfc,
	int            *smin,
	int            *smax,
	int            *gmax,
	int            *ip,
	GRID_DIRECTION dir,
	CRX_TYPE       crx_type)
{
	Table	  *T = table_of_interface(intfc);
	COMPONENT *comp = T->components;
	int	  *ef = T->edge_flag;
	COMPONENT current_comp,cn,cl,cu;
	CRXING	  *crx,*crx1;
	int 	  ip1[3],ip2[3];
	int	  i,k,nc,list,l,step;
	int	  unphysical_encountered = NO;

	for (i = 0; i < 3; ++i)
	    ip1[i] = ip[i];
	current_comp = comp[d_index3d(ip1[0],ip1[1],ip1[2],gmax)];
	step = 0;
	while (next_ip_in_dir(ip1,dir,ip2,smin,smax))
	{
	    cn = comp[d_index3d(ip2[0],ip2[1],ip2[2],gmax)];
	    k = seg_index3d(ip1[0],ip1[1],ip1[2],dir,gmax);
	    
	    if(use_wall_edge()  &&  ef[k] != INSIDE_WALL)
	        break;
	    
	    nc = T->seg_crx_count[k];
	    if (cn != NO_COMP)
	    {
	    	if (dir == EAST || dir == NORTH || dir == UPPER)
		{
		    cl = current_comp;
		    cu = cn;
		}
		else
		{
		    cu = current_comp;
		    cl = cn;
		}
		if (nc == 0)
		{
		    if (cl == cu) break;
		    else
		    {
		    	(void) printf("WARNING: no crxing between adjacent ");
			(void) printf("points with different components!\n");
			(void) printf("ip1 = %d %d %d  comp = %d\n",
					ip1[0],ip1[1],ip1[2],current_comp);
			(void) printf("ip2 = %d %d %d  comp = %d\n",
					ip2[0],ip2[1],ip2[2],cn);
		    	break;
		    }
		}
		else
		{
		    if (crx_type == SINGLE)
		    {
			if (cl == cu)
			{
			    T->seg_crx_count[k] = 0;
			    break;
			}
			for (i = 0; i < nc; ++i)
			{
		    	    list = T->seg_crx_lists[k][i];
		    	    crx = &(T->crx_store[list]);
			    if (crx->lcomp == cl && crx->ucomp == cu)
			    {
			    	T->seg_crx_lists[k][0] =
					T->seg_crx_lists[k][i];
			    	T->seg_crx_count[k] = 1;
				break;
			    }
			}
			if (i < nc) break;
			if (i == nc)
			{
			    (void) printf("WARNING: No suitable crxing!\n");
			    T->seg_crx_count[k] = 0;
			    break;
			}
		    }
		}
	    }

	    /*case 1. cn = NO_COMP, */
	    /*case 2. cn!= NO_COMP, crx_type = MULTIPLE */
	    /*use current_comp, try to remove tangled crxs. */
	    if (dir == EAST || dir == NORTH || dir == UPPER)
	    {
		for (i = 0; i < nc; ++i)
		{
		    list = T->seg_crx_lists[k][i];
		    crx = &(T->crx_store[list]);
		    if (crx->lcomp == current_comp)
		    {
			current_comp = crx->ucomp;
		    }
		    else
		    {
			if (i < nc-1)
			{
		    	    list = T->seg_crx_lists[k][i+1];
			    crx1 = &(T->crx_store[list]);
			    if (crx->ucomp == crx1->lcomp)
			    {
				/* Find unphysical doublet */
				for (l = i; l < nc-2; ++l)
			    	    T->seg_crx_lists[k][l] =
					T->seg_crx_lists[k][l+2];
				nc -= 2;
				T->seg_crx_count[k] -= 2;
				i--;
				/*if (i == 0) */
			        /*    i --; */
				/*else */
				/*    i -= 2; */
			    }
			    else
			    {
				for (l = i; l < nc-1; ++l)
			    	    T->seg_crx_lists[k][l] =
					T->seg_crx_lists[k][l+1];
				--i;
				--nc;
				--T->seg_crx_count[k];
				unphysical_encountered = YES;
				break;
			    }
			}
			else
			{
			    --T->seg_crx_count[k];
			    unphysical_encountered = YES;
			    break;
			}
		    }
		}
	    }
	    else  /*WEST SOUTH LOWER */
	    {
		for (i = nc-1; i >= 0; --i)
		{
		    list = T->seg_crx_lists[k][i];
		    crx = &(T->crx_store[list]);
		    if (crx->ucomp == current_comp)
			current_comp = crx->lcomp;
		    else
		    {
			if (i > 0)
			{
		    	    list = T->seg_crx_lists[k][i-1];
			    crx1 = &(T->crx_store[list]);
			    if (crx->lcomp == crx1->ucomp)
			    {
				/* Find unphysical doublet */
				for (l = i-1; l < nc-2; ++l)
			    	    T->seg_crx_lists[k][l] =
					T->seg_crx_lists[k][l+2];
				nc -= 2;
				T->seg_crx_count[k] -= 2;
				i--;
			    }
			    else
			    {
			    	for (l = i; l < nc-1; ++l)
			            T->seg_crx_lists[k][l] =
				        T->seg_crx_lists[k][l+1];
			        --nc;
			        --T->seg_crx_count[k];
			        unphysical_encountered = YES;
			        break;
			    }
			}
			else
			{
			    for (l = i; l < nc-1; ++l)
			        T->seg_crx_lists[k][l] =
				    T->seg_crx_lists[k][l+1];
			    --nc;
			    --T->seg_crx_count[k];
			    unphysical_encountered = YES;
			    break;
			}
		    }
		}
	    }
	    if (unphysical_encountered) break;
	    if (comp[d_index3d(ip2[0],ip2[1],ip2[2],gmax)] == NO_COMP) 
	    	comp[d_index3d(ip2[0],ip2[1],ip2[2],gmax)] = current_comp;
	    else if (comp[d_index3d(ip2[0],ip2[1],ip2[2],gmax)] != current_comp)
	    	break;
	    if (crx_type == SINGLE && nc != 1) 
	    {
		if (nc%2 == 0) 
		    T->seg_crx_count[k] = 0;
		else
		    T->seg_crx_count[k] = 1;
	    }
	    for (i = 0; i < 3; ++i)
		ip1[i] = ip2[i];
	    ++step;
	}
	return step;
}	/* end rm_unphy_crx_along_grid_line */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

LOCAL	int 	crx_ascend(
	const void 	*c1,
	const void 	*c2)
{
  	if (*(((CRX_SORT*)c1)->compare_coord) >
	    *(((CRX_SORT*)c2)->compare_coord)) return 1;
	else return -1;
}		/*end crx_ascend*/

LOCAL	int 	crx_descend(
	const void 	*c1,
	const void 	*c2)
{
  	if (*(((CRX_SORT*)c1)->compare_coord) <
	    *(((CRX_SORT*)c2)->compare_coord)) return 1;
  	else return -1;
}		/*end crx_descend*/

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

EXPORT	void show_component_along_line(
	int i1,
	int i2,
	int *smin,
	int *smax,
	int idir,
	INTERFACE *intfc)
{
	RECT_GRID	gr = topological_grid(intfc);
	int 		ix,iy,iz,k,nc;
	Table		*T = table_of_interface(intfc);
	COMPONENT       *comp = T->components;
	int 		*gmax = gr.gmax;

	if (idir == 0)
	{
	    for (ix = smin[0]; ix <= smax[0]; ++ix)
	    {
	        k = seg_index3d(ix,i1,i2,EAST,gmax);
		nc = T->seg_crx_count[k];
		if (nc == 0)
		    printf("%2d ",comp[d_index3d(ix,i1,i2,gmax)]);
		else if (nc == 1)
		    printf("%2d|",comp[d_index3d(ix,i1,i2,gmax)]);
		else
		    printf("%2d*",comp[d_index3d(ix,i1,i2,gmax)]);
	    	if ((ix+1)%20 == 0) printf("\n");
	    }
	}
	else if (idir == 1)
	{
	    for (iy = smin[1]; iy <= smax[1]; ++iy)
	    {
	        k = seg_index3d(i2,iy,i1,NORTH,gmax);
		nc = T->seg_crx_count[k];
		if (nc == 0)
		    printf("%2d ",comp[d_index3d(i2,iy,i1,gmax)]);
		else if (nc == 1)
		    printf("%2d|",comp[d_index3d(i2,iy,i1,gmax)]);
		else
		    printf("%2d*",comp[d_index3d(i2,iy,i1,gmax)]);
	    	if ((iy+1)%20 == 0) printf("\n");
	    }
	}
	else
	{
	    for (iz = smin[2]; iz <= smax[2]; ++iz)
	    {
	        k = seg_index3d(i1,i2,iz,UPPER,gmax);
		nc = T->seg_crx_count[k];
		if (nc == 0)
		    printf("%2d ",comp[d_index3d(i1,i2,iz,gmax)]);
		else if (nc == 1)
		    printf("%2d|",comp[d_index3d(i1,i2,iz,gmax)]);
		else
		    printf("%2d*",comp[d_index3d(i1,i2,iz,gmax)]);
	    	if ((iz+1)%20 == 0) printf("\n");
	    }
	}
	printf("\n");
}	/* end show_component_along_line */

EXPORT	boolean adjacent_cell(
	int *icrds1,
	int *icrds2)
{
	int i,num_idn,num_nb;
	num_idn = num_nb = 0;
	for (i = 0; i < 3; ++i)
	{
	    if (icrds1[i] == icrds2[i]) num_idn++;
	    if (icrds1[i] - icrds2[i] == 1 ||
	        icrds1[i] - icrds2[i] == -1)
		num_nb++;
	}
	if ((num_idn == 2 && num_nb == 1) || num_idn == 3) return YES;
	else return NO;
}	/* end adjacent_cell */


EXPORT	int count_grid_intfc_crossings3d(
	INTERFACE	*grid_intfc)
{
	RECT_GRID	*rgr = &topological_grid(grid_intfc);
	struct Table	*T = table_of_interface(grid_intfc);
	TRI		*****tz, ****tzy, ***tzyx;
	int		i,j,k;
	int		***ntz, **ntzy, *ntzyx;
	int		icrds[MAXD];
	int		n_crx = 0;
	int             xmax, ymax, zmax;
	int		*seg_crx_count = T->seg_crx_count;
	DEBUG_ENTER(count_grid_intfc_crossings3d)

	if (grid_intfc->surfaces == NULL || 
	    is_outside_surfaces(grid_intfc, rgr)) 
		return 0;

	xmax = rgr->gmax[0];
	ymax = rgr->gmax[1];
	zmax = rgr->gmax[2];

	for (k = 0, ntz = T->num_of_tris, tz = T->tris;
						k < zmax; ++k, ++ntz, ++tz)
	{
	    for (j = 0, ntzy = *ntz, tzy = *tz;
						j < ymax; ++j, ++ntzy, ++tzy)
	    {
		for (i = 0, ntzyx = *ntzy, tzyx = *tzy;
						i < xmax; ++i, ++ntzyx, ++tzyx)
		{
		    if (*ntzyx == 0)
			continue;
		    icrds[0] = i; icrds[1] = j; icrds[2] = k;
		    n_crx += count_block_crossings(rgr,seg_crx_count,*tzyx,
				    		*ntzyx,icrds);
		}
	    }
	}
	DEBUG_LEAVE(count_grid_crossings3d)
	return n_crx;
}		/* end count_grid_intfc_crossings3d */

enum { MAX_EDGE_CRX = 20 }; /*TOLERANCE*/

LOCAL int count_block_crossings(
	RECT_GRID	*rgr,
	int		*seg_crx_count,
	TRI		**tris,
	int		num_tris,
	int		*ic)
{
	int		n_blk_crx;
	double		coords[MAXD],crds_crx[MAXD];
	double           *L, *h;
	int		i,n_ecrx;	/* number of crossings on edge */
	int		index;
	int		iv,ie;
	int             xmax, ymax, zmax;
	static CRX_STORE	*crx_list;
	
	/*DEBUG_ENTER(count_block_crossings) */
	int 		*gmax = rgr->gmax;

	xmax = rgr->gmax[0];
	ymax = rgr->gmax[1];
	zmax = rgr->gmax[2];
	L = rgr->L;
	h = rgr->h;
	if (crx_list == NULL)
	    uni_array(&crx_list,MAX_EDGE_CRX,sizeof(CRX_STORE));
	for (i = 0; i < 3; ++i)
	    coords[i] = L[i] + ic[i]*h[i];
	n_blk_crx = 0;

		/* Count EAST face crossings */

	n_ecrx = 0;
	for (i = 0; i < num_tris; ++i)
	{
	    if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
	    {
		n_blk_crx += add_to_edge_list(tris[i],0,crx_list,
				              &n_ecrx,crds_crx,iv,ie);
	    }
	}
	index = seg_index3d(ic[0],ic[1],ic[2],EAST,gmax);
	seg_crx_count[index] = n_ecrx;

		/* Count NORTH face crossings */

	n_ecrx = 0;
	for (i = 0; i < num_tris; ++i)
	{
	    if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
	    {
		n_blk_crx += add_to_edge_list(tris[i],1,crx_list,
				              &n_ecrx,crds_crx,iv,ie);
	    }
	}
	index = seg_index3d(ic[0],ic[1],ic[2],NORTH,gmax);
	seg_crx_count[index] = n_ecrx;

		/* Count UPPER face crossings */

	n_ecrx = 0;
	for (i = 0; i < num_tris; ++i)
	{
	    if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
	    {
		n_blk_crx += add_to_edge_list(tris[i],2,crx_list,
				              &n_ecrx,crds_crx,iv,ie);
	    }
	}
	index = seg_index3d(ic[0],ic[1],ic[2],UPPER,gmax);
	seg_crx_count[index] = n_ecrx;

	if (ic[0] != xmax-1 && ic[1] != ymax-1 && ic[2] != zmax-1)
	{
	    /*DEBUG_LEAVE(count_block_crossings) */
	    return n_blk_crx;
	}

	if (ic[0] == xmax - 1)
	{
	    ic[0] += 1;
	    coords[0] += h[0];
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],1,crx_list,
				                  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],NORTH,gmax);
	    seg_crx_count[index] = n_ecrx;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],2,crx_list,
				                  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],UPPER,gmax);
	    seg_crx_count[index] = n_ecrx;
	    ic[0] -= 1;
	    coords[0] -= h[0];
	}
	if (ic[1] == ymax - 1)
	{
	    ic[1] += 1;
	    coords[1] += h[1];
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],0,crx_list,
				                  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],EAST,gmax);
	    seg_crx_count[index] = n_ecrx;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],2,crx_list,
				                  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],UPPER,gmax);
	    seg_crx_count[index] = n_ecrx;
	    coords[1] -= h[1];
	    ic[1] -= 1;
	}
	if (ic[2] == zmax - 1)
	{
	    ic[2] += 1;
	    coords[2] += h[2];
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],0,crx_list,
						  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],EAST,gmax);
	    seg_crx_count[index] = n_ecrx;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],1,crx_list,&n_ecrx,
						  crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],NORTH,gmax);
	    seg_crx_count[index] = n_ecrx;
	    coords[2] -= h[2];
	    ic[2] -= 1;
	}
	if (ic[0] == xmax-1 && ic[1] == ymax-1)
	{
	    ic[0] += 1;
	    ic[1] += 1;
	    coords[0] += h[0];
	    coords[1] += h[1];
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],2,crx_list,
				                  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],UPPER,gmax);
	    seg_crx_count[index] = n_ecrx;
	    coords[0] -= h[0];
	    coords[1] -= h[1];
	    ic[0] -= 1;
	    ic[1] -= 1;
	}
	if (ic[0] == xmax-1 && ic[2] == zmax-1)
	{
	    ic[0] += 1;
	    ic[2] += 1;
	    coords[0] += h[0];
	    coords[2] += h[2];
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],1,crx_list,
				                  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],NORTH,gmax);
	    seg_crx_count[index] = n_ecrx;
	    coords[0] -= h[0];
	    coords[2] -= h[2];
	    ic[0] -= 1;
	    ic[2] -= 1;
	}
	if (ic[1] == ymax-1 && ic[2] == zmax-1)
	{
	    ic[1] += 1;
	    ic[2] += 1;
	    coords[1] += h[1];
	    coords[2] += h[2];
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
		{
		    n_blk_crx += add_to_edge_list(tris[i],0,crx_list,
				                  &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    index = seg_index3d(ic[0],ic[1],ic[2],EAST,gmax);
	    seg_crx_count[index] = n_ecrx;
	    coords[1] -= h[1];
	    coords[2] -= h[2];
	    ic[1] -= 1;
	    ic[2] -= 1;
	}
	/*DEBUG_LEAVE(count_block_crossings) */
	return n_blk_crx;
}		/*end count_block_crossings*/

LOCAL int add_to_edge_list(
	TRI		*tri,
	int		ic,
	CRX_STORE	*crx_list,
	int		*nc,
	double		*crds_crx,
	int		iv,
	int		ie)
{
	int		i;

	if (*nc != 0)
	{
	    if (iv != ERROR)
	    {
	    	for (i = 0; i < *nc; ++i)
	    	{
		    if (crx_list[i].vertex == Point_of_tri(tri)[iv])
		    	return 0;
	    	}
	    }
	    else if (ie != ERROR)
	    {
		for (i = 0; i < *nc; ++i)
		{
		    if ((crx_list[i].edge[0] == Point_of_tri(tri)[ie] &&
			 crx_list[i].edge[1] == Point_of_tri(tri)[(ie+1)%3]) ||
		        (crx_list[i].edge[1] == Point_of_tri(tri)[ie] &&
			 crx_list[i].edge[0] == Point_of_tri(tri)[(ie+1)%3]))
		    {
		 	return 0;
		    }
		}
	    }
	    else
	    {
	    	for (i = 0; i < *nc; ++i)
	    	{
	    	    if ((crx_list[i].coords[ic] == crds_crx[ic]) &&
		        same_sign(Tri_normal(tri)[ic],crx_list[i].coords[3]))
		    	return 0;
	    	}
	    }
	}
	if (iv != ERROR)
	{
	    crx_list[*nc].vertex = Point_of_tri(tri)[iv];
	    crx_list[*nc].edge[0] = NULL;
	    crx_list[*nc].edge[1] = NULL;
	}
	else if (ie != ERROR)
	{
	    if (! is_side_bdry(tri,ie))
	    {
		TRI *nbtri = Tri_on_side(tri,ie);
		
		if (nbtri != NULL && 
 		    Tri_normal(tri)[ic] >= 0.0 && Tri_normal(nbtri)[ic] <= 0.0)
		    return 0;
	    }
	    crx_list[*nc].edge[0] = Point_of_tri(tri)[ie];
	    crx_list[*nc].edge[1] = Point_of_tri(tri)[(ie+1)%3];
	    crx_list[*nc].vertex = NULL;
	}
	else
	{
	    for (i = 0; i < 3; ++i)
	    {
	    	crx_list[*nc].coords[i] = crds_crx[i];
	    }
	    crx_list[*nc].coords[3] = Tri_normal(tri)[ic];
	    crx_list[*nc].vertex = NULL;
	    crx_list[*nc].edge[0] = NULL;
	    crx_list[*nc].edge[1] = NULL;
	}
	++(*nc);
	return 1;
}		/*end add_to_edge_list*/

LOCAL void insert_block_crossings(
	INTERFACE	*intfc,
	RECT_GRID	*rgr,
	CRXING		*crx_store,
	int		**seg_crx_lists,
	int		*seg_crx_count,
	TRI		**tris,             /* triangles         */
	SURFACE		**surfs,            /* surfs of tris     */
	int		num_tris,           /* num tris in block */
	int		*icrds,             /* block icoords     */
	int		*index )            /* crossing index    */
{
	double		coords[MAXD],crds_crx[MAXD];
	double           *L, *h;
	int		i,k,iv,ie,n_ecrx; /* number of crossings on edge */
	CRXING		*crx_list;
	int		*edge_list;
	int             xmax, ymax, zmax;
	static CRX_STORE	*crx_tmp_store;
	int		*gmax = rgr->gmax;

	xmax = rgr->gmax[0];
	ymax = rgr->gmax[1];
	zmax = rgr->gmax[2];
	L = rgr->L;
	h = rgr->h;
	if (crx_tmp_store == NULL)
	    uni_array(&crx_tmp_store,MAX_EDGE_CRX,sizeof(CRX_STORE));
	for (i = 0; i < 3; ++i)
	    coords[i] = L[i] + icrds[i]*h[i];

		/* Count EAST face crossings */

	k = seg_index3d(icrds[0],icrds[1],icrds[2],EAST,gmax);
	edge_list = seg_crx_lists[k];
	crx_list = crx_store + *index;
	n_ecrx = 0;
	for (i = 0; i < num_tris; ++i)
	{

	    if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
	    {
		add_to_crx_list(index,0,intfc,tris[i],surfs[i],crx_list,
				crx_tmp_store,edge_list,&n_ecrx,crds_crx,iv,ie);
	    }
	    

	}
	seg_crx_count[k] = n_ecrx;

		/* Count NORTH face crossings */

	k = seg_index3d(icrds[0],icrds[1],icrds[2],NORTH,gmax);
	edge_list = seg_crx_lists[k];
	crx_list = crx_store + *index;
	n_ecrx = 0;
	for (i = 0; i < num_tris; ++i)
	{
	    if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
	    {
		add_to_crx_list(index,1,intfc,tris[i],surfs[i],crx_list,
				crx_tmp_store,edge_list,&n_ecrx,crds_crx,iv,ie);
	
	    }
	    
	}
	seg_crx_count[k] = n_ecrx;

		/* Count UPPER face crossings */

	k = seg_index3d(icrds[0],icrds[1],icrds[2],UPPER,gmax);
	edge_list = seg_crx_lists[k];
	crx_list = crx_store + *index;
	n_ecrx = 0;
	for (i = 0; i < num_tris; ++i)
	{
	    if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
	    {
		add_to_crx_list(index,2,intfc,tris[i],surfs[i],crx_list,
				crx_tmp_store,edge_list,&n_ecrx,crds_crx,iv,ie);
	    }
	}

	seg_crx_count[k] = n_ecrx;

	if (icrds[0] != xmax-1 && icrds[1] != ymax-1 && icrds[2] != zmax-1)
	    return;

	if (icrds[0] == xmax - 1)
	{
	    icrds[0] += 1;
	    coords[0] += h[0];
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],NORTH,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
		{
		    add_to_crx_list(index,1,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,&n_ecrx,
				    crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],UPPER,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
		{
		    add_to_crx_list(index,2,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,&n_ecrx,
				    crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    icrds[0] -= 1;
	    coords[0] -= h[0];
	}
	if (icrds[1] == ymax - 1)
	{
	    icrds[1] += 1;
	    coords[1] += h[1];
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],EAST,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
		{
		    add_to_crx_list(index,0,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,
				    &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],UPPER,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
		{
		    add_to_crx_list(index,2,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,
				    &n_ecrx,crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    coords[1] -= h[1];
	    icrds[1] -= 1;
	}
	if (icrds[2] == zmax - 1)
	{
	    icrds[2] += 1;
	    coords[2] += h[2];
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],EAST,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
		{
		    add_to_crx_list(index,0,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,&n_ecrx,
				    crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],NORTH,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
		{
		    add_to_crx_list(index,1,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,&n_ecrx,
				    crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    coords[2] -= h[2];
	    icrds[2] -= 1;
	}
	if (icrds[0] == xmax-1 && icrds[1] == ymax-1)
	{
	    icrds[0] += 1;
	    icrds[1] += 1;
	    coords[0] += h[0];
	    coords[1] += h[1];
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],UPPER,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,2,&iv,&ie,h))
		{
		    add_to_crx_list(index,2,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,&n_ecrx,
				    crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    coords[0] -= h[0];
	    coords[1] -= h[1];
	    icrds[0] -= 1;
	    icrds[1] -= 1;
	}
	if (icrds[0] == xmax-1 && icrds[2] == zmax-1)
	{
	    icrds[0] += 1;
	    icrds[2] += 1;
	    coords[0] += h[0];
	    coords[2] += h[2];
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],NORTH,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,1,&iv,&ie,h))
		{
		    add_to_crx_list(index,1,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,&n_ecrx,
				    crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    coords[0] -= h[0];
	    coords[2] -= h[2];
	    icrds[0] -= 1;
	    icrds[2] -= 1;
	}
	if (icrds[1] == ymax-1 && icrds[2] == zmax-1)
	{
	    icrds[1] += 1;
	    icrds[2] += 1;
	    coords[1] += h[1];
	    coords[2] += h[2];
	    k = seg_index3d(icrds[0],icrds[1],icrds[2],EAST,gmax);
	    edge_list = seg_crx_lists[k];
	    crx_list = crx_store + *index;
	    n_ecrx = 0;
	    for (i = 0; i < num_tris; ++i)
	    {
	    	if (tri_edge_crossing(tris[i],coords,crds_crx,0,&iv,&ie,h))
		{
		    add_to_crx_list(index,0,intfc,tris[i],surfs[i],crx_list,
				    crx_tmp_store,edge_list,&n_ecrx,
				    crds_crx,iv,ie);
		}
	    }
	    seg_crx_count[k] = n_ecrx;
	    coords[1] -= h[1];
	    coords[2] -= h[2];
	    icrds[1] -= 1;
	    icrds[2] -= 1;
	}
}		/*end insert_block_crossings*/

LOCAL	void add_to_crx_list(
	int *index,
	int ic,
	INTERFACE *intfc,
	TRI *tri,
	SURFACE *surf,
	CRXING *crx_list,
	CRX_STORE *crx_tmp_store,
	int *edge_list,
	int *nc,
	double *crds_crx,
	int iv,
	int ie)
{
	FT_POINT	**p;
	int	i;
	
	p = Point_of_tri(tri);
	if (iv != ERROR)
	{
	    if (*nc != 0)
	    {
	    	for (i = 0; i < *nc; ++i)
		{
		    FT_POINT *ptmp = crx_list[i].pt;
		    if (crx_tmp_store[i].vertex == p[iv])
			return;
		    if (crds_crx[(ic+1)%3] == Coords(ptmp)[(ic+1)%3] &&
			crds_crx[(ic+2)%3] == Coords(ptmp)[(ic+2)%3] &&
			Coords(p[iv])[ic] == Coords(ptmp)[ic])
			return;
		}
	    }
	    if (!set_comp_at_vertex(&crx_list[*nc],p[iv],tri,surf,ic))
	    	return;

	    crx_tmp_store[*nc].vertex = p[iv];
	    crx_tmp_store[*nc].edge[0] = NULL;
	    crx_tmp_store[*nc].edge[1] = NULL;
	    
	    if (crds_crx[(ic+1)%3] == Coords(p[iv])[(ic+1)%3] &&
		crds_crx[(ic+2)%3] == Coords(p[iv])[(ic+2)%3])
	    {
	    	crx_list[*nc].pt = Point_of_tri(tri)[iv];
	    }
	    else
	    {
	    	crx_list[*nc].pt = copy_point(Point_of_tri(tri)[iv]);
		
		Coords(crx_list[*nc].pt)[(ic+1)%3] = crds_crx[(ic+1)%3];
		Coords(crx_list[*nc].pt)[(ic+2)%3] = crds_crx[(ic+2)%3];
	    }
	}
	else if (ie != ERROR)
	{
	    if (*nc != 0)
	    {
	    	for (i = 0; i < *nc; ++i)
		{
		    if ((crx_tmp_store[i].edge[0] == p[ie] &&
			 crx_tmp_store[i].edge[1] == p[(ie+1)%3])
			 ||
		        (crx_tmp_store[i].edge[1] == p[ie] &&
			 crx_tmp_store[i].edge[0] == p[(ie+1)%3]))
			return;
		}
	    }
	    if (!is_side_bdry(tri,ie))
	    {
		TRI 	*nbtri = Tri_on_side(tri,ie);
		double	n1, n2;

		if (nbtri != NULL)
		{
		    n1 = Tri_normal(tri)[ic];
		    n2 = Tri_normal(nbtri)[ic];
		    if(n1*n2 <= 0.0)
			return;
		}
	    }
	    crx_list[*nc].pt = Point(crds_crx);
	    interpolate_crx_pt_states_on_edge(intfc,crx_list[*nc].pt,tri,
					surf,ie);
	    
	    crx_tmp_store[*nc].edge[0] = p[ie];
	    crx_tmp_store[*nc].edge[1] = p[(ie+1)%3];
	    crx_tmp_store[*nc].vertex = NULL;
	    
	    if (Tri_normal(tri)[ic] > 0.0)
	    {
		crx_list[*nc].lcomp = negative_component(surf);
		crx_list[*nc].ucomp = positive_component(surf);
	    }
	    else
	    {
		crx_list[*nc].lcomp = positive_component(surf);
		crx_list[*nc].ucomp = negative_component(surf);
	    }
	}
	else
	{
	    if (*nc != 0)
	    {
	    	for (i = 0; i < *nc; ++i)
		{
		    if ((Coords(crx_list[i].pt)[ic] == crds_crx[ic]) &&
			((Tri_normal(tri)[ic] > 0 && 
			  crx_list[i].lcomp == negative_component(surf)) ||
			 (Tri_normal(tri)[ic] < 0 &&
			 crx_list[i].lcomp == positive_component(surf))))
			return;
		}
	    }
	    crx_list[*nc].pt = Point(crds_crx);
	    interpolate_crx_pt_states_on_tri(intfc,crx_list[*nc].pt,tri,surf);
	    
	    crx_tmp_store[*nc].edge[0] = NULL;
	    crx_tmp_store[*nc].edge[1] = NULL;
	    crx_tmp_store[*nc].vertex = NULL;
	    
	    if (Tri_normal(tri)[ic] > 0.0)
	    {
		crx_list[*nc].lcomp = negative_component(surf);
		crx_list[*nc].ucomp = positive_component(surf);
	    }
	    else
	    {
		crx_list[*nc].lcomp = positive_component(surf);
		crx_list[*nc].ucomp = negative_component(surf);
	    }
	}

	crx_list[*nc].hs = Hyper_surf(surf);
	crx_list[*nc].tri = tri;
	crx_list[*nc].crx_num = 0;

	for (i = 0; i < *nc; ++i)
	{
	    if (Coords(crx_list[i].pt)[ic] > Coords(crx_list[*nc].pt)[ic])
	    {
		CRXING crx_tmp = crx_list[i];
		crx_list[i] = crx_list[*nc];
		crx_list[*nc] = crx_tmp;
	    }
	}
	edge_list[*nc] = *index;
	++(*nc);
	++(*index);
}		/* end add_to_crx */

EXPORT  void interpolate_crx_pt_states_on_tri(
	INTERFACE	*intfc,
	FT_POINT		*pt,
	TRI		*tri,
	SURFACE		*surf)
{
	double		f[MAXD];
	double		*coords = Coords(pt);
	FT_POINT		*p;
	int		i;
	Locstate	lstate[3], rstate[3];

	linear_interp_coefs_3d_tri(f,coords,tri);

	for (i=0; i< 3; ++i)
	{   
	    p = Point_of_tri(tri)[i];
	    slsr(p,Hyper_surf_element(tri),Hyper_surf(surf),lstate+i,rstate+i);
	}

	if ((tri_interpolate_intfc_states(intfc,f[0],f[1],f[2],
		                          Coords(Point_of_tri(tri)[0]),
					  lstate[0],
					  Coords(Point_of_tri(tri)[1]),
					  lstate[1],
		                          Coords(Point_of_tri(tri)[2]),
					  lstate[2],
					  left_state(pt))
					      != FUNCTION_SUCCEEDED)
	    ||
	    (tri_interpolate_intfc_states(intfc,f[0],f[1],f[2],
		                            Coords(Point_of_tri(tri)[0]),
					    rstate[0],
					    Coords(Point_of_tri(tri)[1]),
					    rstate[1],
		                            Coords(Point_of_tri(tri)[2]),
					    rstate[2],
					    right_state(pt))
						!= FUNCTION_SUCCEEDED))
	{
	    screen("ERROR in interpolate_crx_pt_states_on_tri(), "
		   "tri_interpolate_intfc_states() failed\n");
	    clean_up(ERROR);
	}
}		/*end interpolate_crx_pt_states_on_tri*/

LOCAL void linear_interp_coefs_3d_tri(
	double		*f,
	double		*coords,
	TRI		*t)
{
	double		*p0,*p1,*p2;

	p0 = Coords(Point_of_tri(t)[0]);
	p1 = Coords(Point_of_tri(t)[1]);
	p2 = Coords(Point_of_tri(t)[2]);
	linear_interp_coefs_three_pts(f,coords,p0,p1,p2);

}		/*end linear_interp_coefs_3d_tri*/

EXPORT void linear_interp_coefs_three_pts(
	double		*f,
	double		*coords,
	double		*p0,
	double		*p1,
	double		*p2)
{
	double		v1[MAXD],v2[MAXD],v[MAXD];
	double		qd[MAXD],q1[MAXD],q2[MAXD];
	double		D,D2;
	int		i;

	for (i = 0; i < 3; ++i)
	{
	    v1[i] = p1[i] - p0[i];
	    v2[i] = p2[i] - p0[i];
	    v[i] = coords[i] - p0[i];
	}

	D = vector_product(v1,v2,qd,3);
	Cross3d(v,v2,q1);
	Cross3d(v,v1,q2);
	D2 = D*D;

	f[1] =  Dot3d(qd,q1)/D2;
	f[2] = -Dot3d(qd,q2)/D2;
	f[0] = 1.0 - f[1] - f[2];
}		/*end linear_interp_coefs_three_pts*/

EXPORT void interpolate_crx_pt_states_on_edge(
	INTERFACE	*intfc,
	FT_POINT		*pt,
	TRI		*tri,
	SURFACE		*surf,
	int 		ie)
{
	double		alpha;
	double		*coords = Coords(pt);
	FT_POINT		*p1,*p2;
	int		i;
	Locstate	lstate[2], rstate[2];
	static  double	crx_tol;
	static	boolean	first = YES;
	if (first)
	{
	    double *h = topological_grid(intfc).h;
	    first = NO;
	    crx_tol = 0.000001*h[0];
	}

	p1 = Point_of_tri(tri)[ie];
	slsr(p1,Hyper_surf_element(tri),Hyper_surf(surf),lstate,rstate);
	p2 = Point_of_tri(tri)[(ie+1)%3];
	slsr(p2,Hyper_surf_element(tri),Hyper_surf(surf),lstate+1,rstate+1);
	alpha = 0.5;
	for (i = 0; i < 3; ++i)
	{
	    if (within_interval(Coords(p1)[i],Coords(p2)[i],coords[i]))
	    {
		if (fabs(Coords(p1)[i] - Coords(p2)[i]) < crx_tol)
		    continue;
		alpha = fabs(coords[i]-Coords(p2)[i])/
			fabs(Coords(p1)[i]-Coords(p2)[i]);
		break;
	    }
	}

	bi_interpolate_intfc_states(intfc,alpha,1.0-alpha,
		Coords(p1),lstate[0],Coords(p2),lstate[1],left_state(pt));
	bi_interpolate_intfc_states(intfc,alpha,1.0-alpha,
		Coords(p1),rstate[0],Coords(p2),rstate[1],right_state(pt));
}		/*end interpolate_crx_pt_states_on_tri*/

LOCAL boolean set_comp_at_vertex(
	CRXING  *crx,
	FT_POINT   *p,
	TRI     *tri,
	SURFACE *surf,
	int     dir)
{
	int i,v,num_tris;
	TRI **tri_list;
	double *p0,*p1,*p2,dp1[2],dp2[2];
	double dp1_len,dp2_len,sin_arg,cos_arg;
	double angle,accum_angle;
	double pi_2 = PI/2.0;

	num_tris = set_tri_list_around_point(p,tri,&tri_list,surf->interface);
	accum_angle = 0.0;
	for (i = 0; i < num_tris; ++i)
	{
	    v = Vertex_of_point(tri_list[i],p);
	    p0 = Coords(Point_of_tri(tri_list[i])[v]);
	    p1 = Coords(Point_of_tri(tri_list[i])[Next_m3(v)]);
	    p2 = Coords(Point_of_tri(tri_list[i])[Prev_m3(v)]);
	    dp1[0] = p1[Next_m3(dir)] - p0[Next_m3(dir)];
	    dp2[0] = p2[Next_m3(dir)] - p0[Next_m3(dir)];
	    dp1[1] = p1[Prev_m3(dir)] - p0[Prev_m3(dir)];
	    dp2[1] = p2[Prev_m3(dir)] - p0[Prev_m3(dir)];
	    dp1_len = sqrt(dp1[0]*dp1[0] + dp1[1]*dp1[1]);
	    dp2_len = sqrt(dp2[0]*dp2[0] + dp2[1]*dp2[1]);
	    if (dp1_len == 0.0 || dp2_len == 0.0) continue;
	    sin_arg = (dp1[0]*dp2[1] - dp1[1]*dp2[0])/dp1_len/dp2_len;
	    cos_arg = (dp1[0]*dp2[0] + dp1[1]*dp2[1]);
	    if (sin_arg >  1.0) sin_arg =  1.0;
	    if (sin_arg < -1.0) sin_arg = -1.0;
	    angle = asin(sin_arg);
	    if (cos_arg < 0.0) 
	    {
	        if (sin_arg > 0.0)
	    	angle =  PI - angle;
	        else
	    	angle = -PI - angle;
	    }
	    accum_angle += angle;
	}
	if (accum_angle > pi_2 - 0.01)
	{
	    crx->lcomp = negative_component(surf);
	    crx->ucomp = positive_component(surf);
	    return YES;
	}
	else if (accum_angle < -pi_2 + 0.01)
	{
	    crx->lcomp = positive_component(surf);
	    crx->ucomp = negative_component(surf);
	    return YES;
	}
	else
	{
	    crx->lcomp = crx->ucomp = NO_COMP;
	    return NO;
	}
}		/*end set_comp_at_vertex*/

EXPORT 	int insert_grid_intfc_crossings3d(
        INTERFACE *grid_intfc)
{
        struct Table    *T = table_of_interface(grid_intfc);
	RECT_GRID       *rgr = &topological_grid(grid_intfc);
	TRI 		**t;
	SURFACE		**s;
	int		nt;
        int             icrds[MAXD];
        int             i,j,k;
        int             crx_index = 0;
        int             xmax, ymax, zmax;
        CRXING          *crx_store = T->crx_store;
        int             **seg_crx_lists = T->seg_crx_lists;
        int             *seg_crx_count = T->seg_crx_count;

        DEBUG_ENTER(insert_grid_intfc_crossings)

	if (grid_intfc->surfaces == NULL ||
	    is_outside_surfaces(grid_intfc, rgr)) 
		return GOOD_STEP;
      
	xmax = rgr->gmax[0];
        ymax = rgr->gmax[1];
        zmax = rgr->gmax[2];

	for (k = 0; k < zmax; ++k)
	for (j = 0; j < ymax; ++j)
	for (i = 0; i < xmax; ++i)
	{
	    nt = T->num_of_tris[k][j][i];
	    if (nt == 0) continue;
	    t = T->tris[k][j][i];
	    s = T->surfaces[k][j][i];
            icrds[0] = i; icrds[1] = j; icrds[2] = k;
	    insert_block_crossings(grid_intfc,rgr,crx_store,seg_crx_lists,
			seg_crx_count,t,s,nt,icrds,&crx_index);
	}
	
	if(debugging("tst_param"))
	{
	    int	smin[3] = {0,0,0};
	    printf("#af ins\n");
	    print_edge_crossings(smin,rgr->gmax,grid_intfc);
	}

	DEBUG_LEAVE(insert_grid_intfc_crossings)
	return GOOD_STEP;
}               /*end insert_grid_intfc_crossings*/

EXPORT  void free_crx_storage(
        INTERFACE *intfc)
{
        Table *T = table_of_interface(intfc);
        free_these(5,T->seg_crx_count,T->seg_crx_lists,T->seg_crx_lists_store,
                                T->crx_store,T->components);
	if (T->area) free_these(2,T->area,T->vol_frac);
}       /* end free_crx_storage */


/* if it fails average_btris will give inconsistent results. */
LOCAL   void check_curve_connect(
	CURVE	   *c,
	SURFACE    *surf)
{
	BOND      *b, *bn;
	BOND_TRI  **btris, *btri, *btrin;

	for(b = c->first; b != NULL; b = b->next)
	{
	    bn = b->next;
	    if(bn == NULL)    /*b==c->last */
	        if(is_closed_curve(c))
		    bn = c->first;
		else
		    break;
	    
	    for(btris = Btris(b); btris && *btris; btris++)
	        if((*btris)->surface == surf)
		{    
		    btri = *btris;
		    break;
	        }
	    for(btris = Btris(bn); btris && *btris; btris++)
	        if((*btris)->surface == surf)
		{
		    btrin = *btris;
		    break;
		}

	    if(left_end_btri_state(btri) != left_start_btri_state(btrin))
	    {
	         printf("ERROR check_curve_connect, left btri state is"
		 		"inconsistent.\n");
		 print_bond(b);
		 print_bond(bn);
		 clean_up(ERROR);
	    }
	    if(right_end_btri_state(btri) != right_start_btri_state(btrin))
	    {
	         printf("ERROR check_curve_connect, right btri state is inconsistent.\n");
		 clean_up(ERROR);
	    }
	}
}

/*should pass this check, because a reorder_curve_link_list is at the end of  */
/*reconstruct_intfc3d_in_box */
EXPORT  void  check_surface_curve(
	SURFACE    *s)
{
CURVE   **c;

	for(c=s->pos_curves; c && *c; c++)
	    check_curve_connect(*c, s);
	for(c=s->neg_curves; c && *c; c++)
	    check_curve_connect(*c, s);
}

EXPORT  void check_intfc_curve_connect(INTERFACE *intfc)
{
	SURFACE    **s;

	printf("#check_intfc_curve_connect\n");
	for(s=intfc->surfaces; s && *s; s++)
	{
	    if(positive_component(*s) == 3 && negative_component(*s) == 2)
	        check_surface_curve(*s);
	}
}

#endif /* defined(THREED) */


