#include <front/fdecs.h>

enum {MAX_NUM_CURVES = 400};

enum {
	UNSET = 0x0000,
        F_VOL = 0x0002,     /* On front, can build volume */
	F_NO_GBS = 0x0004,  /* On front, BLK is NOT GRID_BASED  */
	F_NO_VOL = 0x0008,  /* On front, but does not build volume */
	OFF_FRONT = 0x0010  /* Off front BLK */
};

LOCAL 	void set_aug_comp_grid2d(INTERFACE*,RECT_GRID*);
LOCAL   void set_comp_at_crossings2d1(INTERFACE*);
LOCAL  	void alloc_blk_crx2d_on_grid(BLK_CRX2**);
LOCAL 	void rm_select_odd_crx(int,int*,int**,CRXING*);
LOCAL 	void rm_select_even_crx(int,int*,int**,CRXING*);
LOCAL   void init_blk_edges(BLK_CRX2*,BLK_EDGE*,double*,double*);
LOCAL   void eliminate_same_crossings2d(INTERFACE*,int*,int*);
LOCAL   void set_aug_comp_grid(RECT_GRID*,RECT_GRID*,int);
LOCAL  	void set_interface_topology_on_grid(INTERFACE*,RECT_GRID*);
LOCAL	void init_seg_crx_lists_on_grid(INTERFACE*,int,int);
LOCAL 	void adjust_for_min_spacing2d(CRXING*,double*,int*,double*,int,int,int,
        		int**,CRXING*);
LOCAL   void rm_unphy_crx_along_grid_line2d(INTERFACE*,int*,int*,int*,int*,
			GRID_DIRECTION,CRX_TYPE);
LOCAL	void count_crossings_on_grid(INTERFACE*,double,double,double,double,int,int,
			int,int,double,double,double,double,CURVE*,int*,RECT_GRID*);
LOCAL   void set_crx_storage_for_reconstruction_on_grid2d(INTERFACE*,
			RECT_GRID*);
LOCAL	int grid_crossings_on_segment_on_grid(INTERFACE*,double,double,double, 
			double,int,int,int,int,double,double,double,double,CURVE*,
			int*,int*,RECT_GRID*);
LOCAL   int point_on_bond2d1(FT_POINT*,BOND*);
LOCAL   int count_component2d(COMPONENT***,int,int,int[][2]);
LOCAL 	int shift_from_cell_edge_on_comp(double*,int,RECT_GRID*,double);
LOCAL	int find_index_of_node(NODE*,INTERFACE*);
LOCAL  	int **set_node_index_list_on_grid(INTERFACE*,RECT_GRID*);
LOCAL	int insert_grid_crossings2d_on_grid(INTERFACE*,RECT_GRID*);
LOCAL 	int count_intfc_crossings_of_grid_lattice2d(INTERFACE*,RECT_GRID*);
LOCAL	int add_comp_crx_to_list2d(INTERFACE*,int*,int,CROSSING_DIRECTION,
			FT_POINT*,CURVE*,int*);
LOCAL   int walk_comp_along_grid_line2d(INTERFACE*,int*,int*,int*,int*,
        		GRID_DIRECTION);
LOCAL   boolean reconstruct_crx_intfc2d(INTERFACE*,int,int*,int*);
LOCAL   boolean remove_unphysical_crossings2d_on_grid(INTERFACE*,INTERFACE*,
        		int*,int*,int*);
LOCAL  	boolean track_comp_through_crxings2d_on_grid(int*,int*,int*,INTERFACE*,
        		CRX_TYPE);

#define 	_TOL_ 		1.0e-6
#define 	_NTOL_ 		1.0e-5
LOCAL double TOL = _TOL_;
LOCAL double NTOL = _NTOL_;
LOCAL double ONEMTOL = 1.0 - _TOL_;
LOCAL double ONEMNTOL = 1.0 - _NTOL_;

LOCAL int   n_new_intfc_points = 0; 
LOCAL FT_POINT *new_intfc_points[50];	/*POTENTIAL BUG, static array size*/

/*
*               make_emb_grid_intfc():
*/

EXPORT INTERFACE *make_emb_grid_intfc(
        INTERFACE	*intfc)
{
        INTERFACE	*grid_intfc;
	RECT_GRID       dual_grid, *comp_grid = computational_grid(intfc);
	boolean            sav_copy, sav_interp;
        int             i,smin[MAXD],smax[MAXD];
        CURVE           **c; 
        BOND            *b; 
        int             dim; 
        int             n_fr_blk; 
        size_t          sizest;  
	struct	Table	*T;

        dim = intfc->dim;

        set_size_of_intfc_state(size_of_state(intfc));
        sav_copy = copy_intfc_states(); 
        sav_interp = interpolate_intfc_states(intfc);         
        set_copy_intfc_states(NO);
        interpolate_intfc_states(intfc) = YES;         

        set_add_to_correspond_list(YES);
	
	grid_intfc = copy_interface(intfc);
	T = table_of_interface(grid_intfc);

	set_dual_grid(&dual_grid,comp_grid);
	set_topological_grid(grid_intfc,&dual_grid);
  
        for(c = intfc->curves; c && *c; c++)
        {
            b = (*c)->first;
            b->start->crx = NO; 
            while(b != NULL)
            {
                b->end->crx = NO; 
                b = b->next;
            } 
        }

        set_aug_comp_grid2d(grid_intfc,comp_grid);
        
        set_crx_storage_for_reconstruction_on_grid2d(grid_intfc,
                    	&T->aug_comp_grid);
        insert_grid_crossings2d_on_grid(grid_intfc,&T->aug_comp_grid);
        set_interface_topology_on_grid(grid_intfc,&T->aug_comp_grid); 


	/*next time begin from here */
	set_comp_at_crossings2d1(grid_intfc);
	for (i = 0; i < dim; i++)
        {
            smin[i] = 0;
            smax[i] = T->aug_comp_grid.gmax[i]; /*expanded_comp_grid*/
        }
 
        if (remove_unphysical_crossings2d_on_grid(intfc,grid_intfc,
               &n_fr_blk,smin,smax) == FUNCTION_FAILED)
        {
            free_grid_lines(&T->aug_comp_grid);
            set_copy_intfc_states(sav_copy);
            interpolate_intfc_states(intfc) = sav_interp;         
            clean_up(ERROR);
            DEBUG_LEAVE(make_emb_grid_intfc)
            return NULL;
        }        

        if (reconstruct_crx_intfc2d(grid_intfc,n_fr_blk,smin,smax)
            ==FUNCTION_FAILED)
        {
            free_grid_lines(&T->aug_comp_grid);
            printf("reconstruct_crx_intfc2d() failed.\n");
            set_copy_intfc_states(sav_copy);
            interpolate_intfc_states(intfc) = sav_interp;         
            clean_up(ERROR);
            DEBUG_LEAVE(make_emb_grid_intfc)
            return NULL;
        }

        free_grid_lines(&T->aug_comp_grid); 

        set_copy_intfc_states(sav_copy);
        interpolate_intfc_states(intfc) = sav_interp;     

        DEBUG_LEAVE(make_emb_grid_intfc)
	return grid_intfc; 
}

LOCAL void set_aug_comp_grid2d(
        INTERFACE	*grid_intfc,
        RECT_GRID       *comp_grid)
{
	struct Table *T = table_of_interface(grid_intfc);
        int       gmax[MAXD]; 
        int       i, dim = comp_grid->dim;

        set_aug_comp_grid(&T->aug_comp_grid,comp_grid,1);
        if (! set_grid_lines(&T->aug_comp_grid))
        {
            screen("ERROR in set_tri_grid_aug_comp_grids(), ");
            screen("set_grid_lines() failed\n");
            clean_up(ERROR);
        }        
        for (i = 0; i < dim; i++)
        gmax[i] = T->aug_comp_grid.gmax[i];
        T->c_nnx = gmax[0] + 1;
        T->c_nny = gmax[1] + 1;
        T->c_node_offset = comp_grid->lbuf[0] + 1 +
              (gmax[0]+1) * (comp_grid->lbuf[1] + 1);
        T->c_cell_offset =
            (comp_grid->lbuf[0] + 1) + gmax[0] * (comp_grid->lbuf[1] + 1);
}	/* end set_aug_comp_grid2d */

LOCAL   void set_crx_storage_for_reconstruction_on_grid2d(
        INTERFACE   *grid_intfc,
        RECT_GRID   *expanded_gr)
{
        int         *gmax = expanded_gr->gmax;
        int         *gmax1 = computational_grid(grid_intfc)->gmax;
        int         dim = expanded_gr->dim;
        int         n_segs,n_crx,i,n_reg_nodes;
        int         n_intfc_nodes = 0;
        int         xmax,ymax;        
	NODE        **node; 
	struct	Table *T = table_of_interface(grid_intfc);

	xmax = gmax[0]; ymax = gmax[1];
        
	for (node = grid_intfc->nodes; node && *node;  ++node)
            ++n_intfc_nodes;
        n_segs = 0;
        n_reg_nodes = 1;
        for (i = 0; i < 2; i++)
        {
            n_segs += gmax[i]*(gmax[(i+1)%2] + 1);
            n_reg_nodes *= gmax[i] + 1;
        }
        T->n_c_node_points = T->n_c_reg_nodes = n_reg_nodes;
        T->n_c_segs = n_segs;
        uni_array(&T->c_seg_crx_count,n_segs,INT);
        for (i = 0; i < n_segs; i++)
            T->c_seg_crx_count[i] = 0;
        T->n_c_crx = n_crx = count_intfc_crossings_of_grid_lattice2d(grid_intfc,
				expanded_gr);
        T->n_c_node_points += n_crx+n_intfc_nodes; 

        uni_array(&T->c_node_points, T->n_c_node_points, sizeof(TG_PT)); 
        T->c_cg_npts = T->c_node_points + T->c_node_offset;
        init_seg_crx_lists_on_grid(grid_intfc,n_crx,n_segs);
        uni_array(&T->c_components,T->n_c_reg_nodes,sizeof(COMPONENT));
        T->c_cg_comps = T->c_components + T->c_node_offset;

        bi_array(&T->blk_edge,gmax1[0],gmax1[1],sizeof(BLK_EDGE));

        uni_array(&T->blk_type,xmax*ymax,sizeof(int));
	for (i = 0; i < xmax*ymax; i++)
	    T->blk_type[i] = UNSET;

        {
            int                 j = 0; 
            register double      y;
            register double      *xx_grid = expanded_gr->edges[0];
            register double      *yy_grid = expanded_gr->edges[1];
            int                 xmax1 = expanded_gr->gmax[0],
                                ymax1 = expanded_gr->gmax[1];
            int                 ix,iy;

            for (iy = 0;  iy <= ymax1;  iy++)
            {
                y = yy_grid[iy];
                for (ix = 0;  ix <= xmax1;  ix++)
                {
                    Coords(T->c_node_points+j)[0] = xx_grid[ix];
                    Coords(T->c_node_points+j)[1] = y;
                    j++;
                }
            }
        }
}	/* end set_crx_storage_for_reconstruction_on_grid2d */


LOCAL	void init_seg_crx_lists_on_grid(
	INTERFACE	*grid_intfc,
	int		n_crx,
	int		size)
{
	struct	Table	*T = table_of_interface(grid_intfc);
	int		i;
	int		*scls;

	n_crx += MAX_CRX_FILL;/*Storage for missing crosses*/
	uni_array(&T->c_seg_crx_lists,size,sizeof(int *));
	uni_array(&T->c_seg_crx_lists_store,n_crx,sizeof(int));
	uni_array(&T->c_crx_store,n_crx,sizeof(CRXING));
	scls = T->c_seg_crx_lists_store;
        for (i = 0;  i < size;  ++i)
        {
            if (T->c_seg_crx_count[i] == 0)
                T->c_seg_crx_lists[i] = NULL;
            else
            {
                T->c_seg_crx_lists[i] = scls;
                scls += T->c_seg_crx_count[i];
            }
        }
        for (i = 0;  i < n_crx;  ++i)
            T->c_seg_crx_lists_store[i] = -1;
        return; 
}	/* end init_seg_crx_lists_on_grid */


LOCAL int count_intfc_crossings_of_grid_lattice2d(
	INTERFACE	*grid_intfc,
	RECT_GRID	*rgr)
{
	register CURVE	**c;
	register BOND	*b;
	double           hx = rgr->h[0];
	double           hy = rgr->h[1];
	double           max_lenx,max_leny;
	double		tolx, toly;
	double           *xx_grid,*yy_grid;
	int		n_index;
	int		n_crx = 0;
        int             ix1,ix2,iy1,iy2;
	
	max_lenx = ONEMTOL*hx;		max_leny = ONEMTOL*hy;
	xx_grid = rgr->edges[0];	yy_grid = rgr->edges[1];
	tolx = TOL * hx;		toly = TOL * hy;

			/* count intfc crossings of dual lattice */

	for (c = grid_intfc->curves; c && *c;  ++c)
	{
	    b = (*c)->first;
	    n_index = find_index_of_node((*c)->start,grid_intfc);
	    ix2 = shift_from_cell_edge_on_comp(Coords(b->start),0,rgr,tolx);
	    iy2 = shift_from_cell_edge_on_comp(Coords(b->start),1,rgr,toly);
	    while (b != NULL)
	    {
	    	ix1 = ix2;		iy1 = iy2;

	    	ix2 = shift_from_cell_edge_on_comp(Coords(b->end),0,rgr,tolx);
		iy2 = shift_from_cell_edge_on_comp(Coords(b->end),1,rgr,toly);

		count_crossings_on_grid(grid_intfc,Coords(b->start)[0],
				Coords(b->start)[1],Coords(b->end)[0],
				Coords(b->end)[1],ix1,iy1,ix2,iy2,
				fabs(Coords(b->end)[0] - Coords(b->start)[0]),
				fabs(Coords(b->end)[1] - Coords(b->start)[1]),
				max_lenx,max_leny,*c,&n_crx,rgr);
		b = b->next;
	    }
	}
	return n_crx;
}		/*end count_intfc_crossings_of_grid_lattice2d*/

#define north_cross_on_grid(iy,ymax) (0 <= (iy) && (iy) <  (ymax))
#define south_cross_on_grid(iy,ymax) (0 <  (iy) && (iy) <= (ymax))
#define east_cross_on_grid(ix,xmax)  (0 <= (ix) && (ix) <  (xmax))
#define west_cross_on_grid(ix,xmax)  (0 <  (ix) && (ix) <= (xmax))

LOCAL	void count_crossings_on_grid(
	INTERFACE	*grid_intfc,
	double		x1,
	double		y1,
	double		x2,
	double		y2,
	int             ix1,
	int             iy1,
	int             ix2,
	int             iy2,
	double		lenx,
	double		leny,
	double           max_lenx,
	double           max_leny,
	CURVE		*curve,
	int		*n_crx,
	RECT_GRID	*rgr)
{
	register int	ix, iy;
	double		p3[MAXD], x_h, y_h, x_v, y_v;
	double		tolx, toly, wtolx, wtoly;
	double           hx,hy;
	double           *xx_grid,*yy_grid;
	int		msk, signf, signx;
        int             xmax,ymax; 
        int             *seg_crx_count; 
	struct	Table	*T = table_of_interface(grid_intfc);
         
	hx = rgr->h[0]; hy = rgr->h[1];
     	xmax = rgr->gmax[0]; ymax = rgr->gmax[1];

	seg_crx_count = T->c_seg_crx_count;

		/* if bond is too long, recursively cut off */
		/* hunks with lenx > max_lenx or leny > max_leny */

	tolx = TOL * hx;	toly = TOL * hy;
	if ((lenx > max_lenx) || (leny > max_leny))
	{
	    int	save_ix2, save_iy2;
	    double	s0, s1;

	    s0 = 1.0;
	    if (lenx > max_lenx)
		s0 = max_lenx/lenx;
	    if (leny > max_leny)
		s0 = min(s0,max_leny/leny);
	    s1 = 1.0 - s0;

	    p3[0] = x2 - s0 * (x2-x1);	p3[1] = y2 - s0 * (y2-y1);
	    save_ix2 = ix2;			save_iy2 = iy2;

            ix2 = shift_from_cell_edge_on_comp(p3,0,rgr,tolx);
            iy2 = shift_from_cell_edge_on_comp(p3,1,rgr,toly);

	    count_crossings_on_grid(grid_intfc,x1,y1,p3[0],p3[1],ix1,iy1,ix2,
	    		iy2,s1*lenx,s1*leny,max_lenx,max_leny,curve,n_crx,rgr);
	    ix1 = ix2; 		iy1 = iy2;
	    ix2 = save_ix2; 	iy2 = save_iy2;
	    x1  = p3[0];		y1  = p3[1];
        }
	if ((iy1 != iy2) && (ix1 != ix2))    /* crossings of both vertical */
	{				      /* and horizontal grid bonds */

	    xx_grid = rgr->edges[0];
	    yy_grid = rgr->edges[1];

	    iy = max(iy1,iy2);	ix = max(ix1,ix2);
	    y_h = yy_grid[iy];
	    x_h = x1 + (x2 - x1) * (y_h - y1) / (y2 - y1);
	    x_v = xx_grid[ix];
	    y_v = y1 + (y2 - y1) * (x_v - x1) / (x2 - x1);

	    	/* test if too close to grid crossings */

	    wtolx = x_h - x_v;		wtoly = y_v - y_h;

	    signf = ((ix1-ix2)*(iy1-iy2) > 0) ? 1 : -1;
	    signx = 0;
	    
	    if (fabs(wtolx) < tolx)
	    {
	    	if (wtolx >= 0)
	    	{
	    	    x_h += tolx;
	    	    signx =  1;
	    	}
	    	else
	    	{
	    	    x_h -= tolx;
	    	    signx = -1;
	    	}
	    }
	    if (fabs(wtoly) < toly)
	    {
	    	if (signx != 0)
	    	    y_v -= signx * signf * toly;
		else
		{
		    if (wtoly >= 0)
		        y_v += toly;
		    else
			y_v -= toly;
		}	
	    }
	    msk = 2*iy*xmax + iy + ix;
	    if (x_h <= x_v)
	    {
	    	if (west_cross_on_grid(ix,xmax))
	    	{
	    	    ++(*n_crx);
	    	    ++seg_crx_count[msk-1];
	    	}
	    	if ((ix2 - ix1)*(iy2 - iy1) > 0)
	    	{
	    	    if (north_cross_on_grid(iy,ymax))
	    	    {
	    	    	++(*n_crx);
	    	    	++seg_crx_count[msk+xmax];
	    	    }
	    	}
	    	else
	    	{
	    	    if (south_cross_on_grid(iy,ymax))
	    	    {
	    	    	++(*n_crx);
	    	    	++seg_crx_count[msk-xmax-1];
	    	    }
	    	}
	    }
	    else
	    {
	    	if (east_cross_on_grid(ix,xmax))
	    	{
	    	    ++(*n_crx);
	    	    ++seg_crx_count[msk];
	    	}
	    	if ((ix2 - ix1)*(iy2 - iy1) > 0)
	    	{
	    	    if (south_cross_on_grid(iy,ymax))
	    	    {
	    	    	++(*n_crx);
	    	    	++seg_crx_count[msk-xmax-1];
	    	    }
	    	}
	    	else
	    	{
	    	    if (north_cross_on_grid(iy,ymax))
	    	    {
	    	    	++(*n_crx);
	    	    	++seg_crx_count[msk+xmax];
	    	    }
	    	}
	    }
	}
	else if (iy1 != iy2 && east_cross_on_grid(ix1,xmax))
	{
		/* crossing of horizontal grid bond */

	    ++(*n_crx);
	    iy = max(iy1,iy2);
	    msk = 2*iy*xmax + iy + ix1;
	    ++seg_crx_count[msk];
	}
	else if (ix1 != ix2 && north_cross_on_grid(iy1,ymax))
	{
	    /* crossing of vertical grid bond */

	    ++(*n_crx);
	    ix = max(ix1,ix2);
	    msk = 2*iy1*xmax+iy1+ix+xmax;
	    ++seg_crx_count[msk];
	}
}		/*end count_crossings*/


LOCAL	int insert_grid_crossings2d_on_grid(
        INTERFACE       *grid_intfc, 
        RECT_GRID       *rgr)
{
	register CURVE	        **c;
	register BOND	        *b;
	NODE		**node;
	double           hx = rgr->h[0];
	double           hy = rgr->h[1];
	double           max_lenx,max_leny;
	int		n_crx;
	int		crx_num;
	int		i, n_index;
	int             ix1,ix2,iy1,iy2;
	int		First_cross_on_curve;
	int		status;
	struct	Table	*T = table_of_interface(grid_intfc);

        n_crx = T->n_c_crx;
			/* record crossings */
	crx_num = -1;
	max_lenx = ONEMTOL*hx; max_leny = ONEMTOL*hy;


	for (c = grid_intfc->curves; c && *c;  ++c)
	{
	    First_cross_on_curve = YES;
	    b = (*c)->first;
	    n_index = find_index_of_node((*c)->start,grid_intfc);
	    ix2 = cell_index(Coords(b->start)[0],0,rgr);
            iy2 = cell_index(Coords(b->start)[1],1,rgr);
	    while (b != NULL)
	    {
	    	n_new_intfc_points = 0;
	    	ix1 = ix2;		iy1 = iy2;
	    	ix2 = cell_index(Coords(b->end)[0],0,rgr);
	    	iy2 = cell_index(Coords(b->end)[1],1,rgr);

	    	status = grid_crossings_on_segment_on_grid(grid_intfc,
				Coords(b->start)[0],Coords(b->start)[1],
				Coords(b->end)[0],Coords(b->end)[1],
				ix1,iy1,ix2,iy2,max_lenx,max_leny,
				fabs(Coords(b->end)[0] - Coords(b->start)[0]),
				fabs(Coords(b->end)[1] - Coords(b->start)[1]),
				*c,&crx_num,&First_cross_on_curve,rgr);

	    	if (status != GOOD_STEP) 
	    	{
	    	    return status;
	    	}

                if((! is_passive_boundary(Hyper_surf(*c))) &&
                   (! is_subdomain_boundary(Hyper_surf(*c))) )
                {
	    	    for (i = 0;  i < n_new_intfc_points;  ++i)
                    {
                        if(b == (*c)->first)
                            new_intfc_points[i]->indx = (*c)->sindx; 
                        else if(b == (*c)->last)
                            new_intfc_points[i]->indx = (*c)->eindx;
                        else 
                            new_intfc_points[i]->indx = b->start->indx; 
                    }
                }
	    	for (i = 0;  i < n_new_intfc_points;  ++i)
	    	{
	    	    if (insert_point_in_bond(new_intfc_points[i],b,*c) !=
			FUNCTION_SUCCEEDED)
	            {
	                screen("ERROR in insert_grid_crossings2d(), "
		               "insert_point_in_bond() failed\n");
	                clean_up(ERROR);
	            }
	    	    b = b->next;
	    	}
	    	b = b->next;
	    }
	}

	if (crx_num > 0)
	    T->c_crx_store[crx_num].end = YES;

	return GOOD_STEP;
}		/*end insert_grid_crossings2d*/

/*
*	Recursively inserts the grid crossings for each bond of the intfc
*/

LOCAL	int grid_crossings_on_segment_on_grid(
	INTERFACE	*grid_intfc,
	double		x1,
	double		y1,
	double		x2,
	double		y2,
	int             ix1,
	int             iy1,
	int             ix2,
	int             iy2,
	double           max_lenx,
	double           max_leny,
	double		lenx,
	double		leny,
	CURVE		*curve,
	int		*crx_num,
	int		*fcoc,
	RECT_GRID	*rgr)
{
	register int	ix, iy;
	register FT_POINT	*p_h, *p_v;

	double              hx = rgr->h[0];
	double              hy = rgr->h[1];
	double	           p3[MAXD], x_h, y_h, x_v, y_v;
	double	           coords_h[MAXD], coords_v[MAXD];
	double	           tolx, toly, wtolx, wtoly;
	double              *xx_grid,*yy_grid;
	boolean            weight;
	CROSSING_DIRECTION dir_v, dir_h;
	int 	           signf, signx, msk, status;
        int                xmax,ymax;

	xx_grid = rgr->edges[0]; yy_grid = rgr->edges[1];

	xmax=rgr->gmax[0]; ymax=rgr->gmax[1];

	tolx = TOL * hx;	toly = TOL * hy;
	if ((lenx > max_lenx) || (leny > max_leny))
	{
	    int	save_ix2, save_iy2;
	    double	s0, s1;

	    s0 = 1.0;
	    if (lenx > max_lenx)
		s0 = max_lenx/lenx;
	    if (leny > max_leny)
		s0 = min(s0,max_leny/leny);
	    s1 = 1.0 - s0;

	    p3[0] = x2 - s0 * (x2-x1);	p3[1] = y2 - s0 * (y2-y1);
	    save_ix2 = ix2;			save_iy2 = iy2;

	    ix2 = shift_from_cell_edge_on_comp(p3,0,rgr,tolx);
	    iy2 = shift_from_cell_edge_on_comp(p3,1,rgr,toly);

	    status = grid_crossings_on_segment_on_grid(grid_intfc,x1,y1,p3[0],
	    		p3[1],ix1,iy1,ix2,iy2,max_lenx,max_leny,s1*lenx,
			s1*leny,curve,crx_num,fcoc,rgr);

	    if (status != GOOD_STEP)
		return status;

	    ix1 = ix2;		iy1 = iy2;
	    ix2 = save_ix2;	iy2 = save_iy2;
	    x1  = p3[0];	y1  = p3[1];
	}

	if ((iy1 != iy2) && (ix1 != ix2))    /* crossings of both vertical */
	{				       /* and horizontal grid bonds */
	    iy = max(iy1,iy2);
	    y_h = yy_grid[iy];
	    x_h = x1 + (x2 - x1) * (y_h - y1) / (y2 - y1);

	    dir_h = (iy1 < iy2) ? BELOW_TO_ABOVE : ABOVE_TO_BELOW;

	    ix = max(ix1,ix2);
	    x_v = xx_grid[ix];
	    y_v = y1 + (y2 - y1) * (x_v - x1) / (x2 - x1);

	    dir_v = (ix1 < ix2) ? LEFT_TO_RIGHT : RIGHT_TO_LEFT;

	    	/* test if too close to grid crossings */

	    wtolx = x_h - x_v;		wtoly = y_v - y_h;

	    signf = ((ix1-ix2)*(iy1-iy2) > 0) ? 1 : -1;
	    signx =  0;

	    if (fabs(wtolx) < tolx)
	    {
	    	if (wtolx >= 0)
	    	{
	    	    x_h += tolx;
	    	    signx =  1;
	    	}
	    	else
	    	{
	    	    x_h -= tolx;
	    	    signx = -1;
	    	}
	    }
	    if (fabs(wtoly) < toly)
	    {
	    	if (signx != 0)
	    	    y_v -= signx * signf * toly;
	    	else
	    	{
	    	    if (wtoly >= 0)
			y_v += toly;
	    	    else
			y_v -= toly;
	    	}	
	    }
		/* NOTE: weight is a boolean    	   */
		/* NOTE: for the rest of this else{} loop  */
		/*	 x_h,y_h and x_v,y_v must be equiv */
		/*	 expressions to p_h and p_v	   */

	    weight = ((sqr(x_h-x1)+sqr(y_h-y1)) > (sqr(x_v-x1)+sqr(y_v-y1))) ?
		     YES : NO;
	    coords_h[0] = x_h;	coords_h[1] = y_h;
	    coords_v[0] = x_v;	coords_v[1] = y_v;

	    msk = 2*iy*xmax + iy + ix;
	    if (x_h <= x_v)
	    {
	        if ((ix2 - ix1)*(iy2 - iy1) > 0)
	        {
	            if (weight == YES)
	            {
	    	        if (north_cross_on_grid(iy,ymax))
	    	        {
	    		    p_v = Point(coords_v);
	    		    new_intfc_points[n_new_intfc_points++] = p_v;
	    		    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    			msk+xmax,dir_v,p_v,curve,fcoc);
	    		    if (status != GOOD_STEP)
				return status;
	    	        }

	    	        if (west_cross_on_grid(ix,xmax))
	    	        {
	    		    p_h = Point(coords_h);
	    		    new_intfc_points[n_new_intfc_points++] = p_h;
	    		    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    			msk-1,dir_h,p_h,curve,fcoc);
			    if (status != GOOD_STEP)	
			       return status;
	    	        }
		    }
		    else
		    {
		        if (west_cross_on_grid(ix,xmax))
		        {
		    	    p_h = Point(coords_h);
		    	    new_intfc_points[n_new_intfc_points++] = p_h;
		    	    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    			msk-1,dir_h,p_h,curve,fcoc);
		            if (status != GOOD_STEP)
				return status;
		        }
		        if (north_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[n_new_intfc_points++] = p_v;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    			msk+xmax,dir_v,p_v,curve,fcoc);
			    if (status != GOOD_STEP) return status;
			}
		    }
		}
		else
	        {
		    if (weight == YES)
		    {
		        if (south_cross_on_grid(iy,ymax))
		        {
		    	    p_v = Point(coords_v);
		    	    new_intfc_points[n_new_intfc_points++] = p_v;
		    	    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk-xmax-1,dir_v,p_v,curve,fcoc);
		    	    if (status != GOOD_STEP)
				return status;
		        }

			if (west_cross_on_grid(ix,xmax))
			{
			    p_h = Point(coords_h);
			    new_intfc_points[n_new_intfc_points++] = p_h;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk-1,dir_h,p_h,curve,fcoc);
			    if (status != GOOD_STEP) return status;
			}
		    }
		    else
		    {
		        if (west_cross_on_grid(ix,xmax))
		        {
			    p_h = Point(coords_h);
			    new_intfc_points[n_new_intfc_points++] = p_h;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk-1,dir_h,p_h,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
			}
			if (south_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[n_new_intfc_points++] = p_v;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk-xmax-1,dir_v,p_v,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
			}
		    }
		}
	    }
	    else
	    {
	        if ((ix2 - ix1)*(iy2 - iy1) > 0)
	        {
	    	    if (weight == YES)
	    	    {
	    	        if (south_cross_on_grid(iy,ymax))
	    	        {
	    		    p_v = Point(coords_v);
	    		    new_intfc_points[n_new_intfc_points++] = p_v;
	    		    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk-xmax-1,dir_v,p_v,curve,fcoc);
	    		    if (status != GOOD_STEP)
				return status;
	    	        }

	    	        if (east_cross_on_grid(ix,xmax))
	    	        {
	    		    p_h = Point(coords_h);
	    		    new_intfc_points[n_new_intfc_points++] = p_h;
	    		    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
			}
		    }
		    else
		    {
		        if (east_cross_on_grid(ix,xmax))
		        {
			    p_h = Point(coords_h);
			    new_intfc_points[n_new_intfc_points++] = p_h;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
			}
			if (south_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[n_new_intfc_points++] = p_v;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk-xmax-1,dir_v,p_v,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
			}
		    }
		}
		else
		{
		    if (weight == YES)
		    {
		        if (north_cross_on_grid(iy,ymax))
		        {
			    p_v = Point(coords_v);
			    new_intfc_points[n_new_intfc_points++] = p_v;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk+xmax,dir_v,p_v,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
		        }
			if (east_cross_on_grid(ix,xmax))
			{
			    p_h = Point(coords_h);
			    new_intfc_points[n_new_intfc_points++] = p_h;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
			}
		    }
		    else
		    {
		        if (east_cross_on_grid(ix,xmax))
		        {
		    	    p_h = Point(coords_h);
			    new_intfc_points[n_new_intfc_points++] = p_h;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk,dir_h,p_h,curve,fcoc);
			    if (status != GOOD_STEP)
				return status;
			}

			if (north_cross_on_grid(iy,ymax))
			{
			    p_v = Point(coords_v);
			    new_intfc_points[n_new_intfc_points++] = p_v;
			    status = add_comp_crx_to_list2d(grid_intfc,crx_num,
			    		msk+xmax,dir_v,p_v,curve,fcoc);

			    if (status != GOOD_STEP)
				return status;
			}
		    }
	        }
	    }
	}
	else if (iy1 != iy2) 
	{
	    /* crossing of horizontal grid bond */

	    iy = max(iy1,iy2);
	    y_h = yy_grid[iy];	x_v = xx_grid[ix1];
	    x_h = x1 + (x2 - x1) * (y_h - y1) / (y2 - y1);

	    wtolx = x_h - x_v;
	    if (fabs(wtolx) < tolx)
	    {
	    	if (wtolx > 0.0)
		    x_h += tolx;
	    	else
		    x_h -= tolx;
	    }

	    if (east_cross_on_grid(ix1,xmax))
	    {
	    	dir_h = (iy1 < iy2) ? BELOW_TO_ABOVE : ABOVE_TO_BELOW;
	    	coords_h[0] = x_h;
		coords_h[1] = y_h;
	    	p_h = Point(coords_h);
	    	new_intfc_points[n_new_intfc_points++] = p_h;
	    	status = add_comp_crx_to_list2d(grid_intfc,crx_num,
				2*iy*xmax+iy+ix1,dir_h,p_h,curve,fcoc);
	    	if (status != GOOD_STEP)
		    return status;
	    }
	}
	else if (ix1 != ix2)
	{
	    /* crossing of vertical bond grid */

	    ix = max(ix1,ix2);
	    x_v = xx_grid[ix];	y_h = yy_grid[iy1];
	    y_v = y1 + (y2 - y1) * (x_v - x1) / (x2 - x1);

	    wtoly = y_v - y_h;
	    if (fabs(wtoly) < toly)
	    {
	    	if (wtoly < 0.0)
		    y_v -= toly;
	    	else
		    y_v += toly;
	    }

	    if (north_cross_on_grid(iy1,ymax))
	    {
	    	dir_v = (ix1 < ix2) ? LEFT_TO_RIGHT : RIGHT_TO_LEFT;

	    	coords_v[0] = x_v;	coords_v[1] = y_v;
	    	p_v = Point(coords_v);
	    	new_intfc_points[n_new_intfc_points++] = p_v;
	    	status = add_comp_crx_to_list2d(grid_intfc,crx_num,
				2*iy1*xmax+iy1+ix+xmax,dir_v,p_v,curve,fcoc);
	    	if (status != GOOD_STEP)
		    return status;
	    }
	}
	return GOOD_STEP;
}		/*end grid_crossings_on_segment*/


LOCAL	int add_comp_crx_to_list2d(
	INTERFACE	   *grid_intfc,
	int		   *n_crx,
	int		   msk,
	CROSSING_DIRECTION dir,
	FT_POINT		   *p,
	CURVE		   *cur,
	int		   *First_cross_on_curve)
{
	struct Table	*T = table_of_interface(grid_intfc);
	register CRXING *cross;
	CRXING		*crx_store = T->c_crx_store;

	int		nx, i, j, k, hold, input;
	int		*start;
        int             xmax,ymax;
	
	
	xmax = T->aug_comp_grid.gmax[0]; ymax = T->aug_comp_grid.gmax[1];
	++(*n_crx);

	cross = &(crx_store[*n_crx]);
	cross->hs = Hyper_surf(cur);
	cross->pt = p;
	cross->crossing_direction = dir;
	cross->crx_num = *n_crx;
        cross->lcomp = NO_COMP;
        cross->ucomp = NO_COMP;
        p->crx = YES; 

	if (First_cross_on_curve != NULL)
	{
	    j = (ymax+1)*(xmax+1)+(*n_crx);
	    
	    Coords(T->c_node_points+j)[0] = Coords(p)[0];
	    Coords(T->c_node_points+j)[1] = Coords(p)[1];
	    cross->nd = &T->c_node_points[j];
	}

	if ((First_cross_on_curve != NULL) && (*First_cross_on_curve == YES))
	{
	    cross->end = YES;
	    if (*n_crx > 0)
	    {
	    	crx_store[*n_crx - 1].end = YES;
	    }
	    *First_cross_on_curve = NO;
	}
	else
	    cross->end = NO;

	nx = T->c_seg_crx_count[msk];
	if (nx == 0)
	{
	    (void) printf("WARNING in add_crx_to_list2d(), nx = 0");
	    return MODIFY_TIME_STEP;
	}

	start = T->c_seg_crx_lists[msk];

	switch (dir)
	{
	case BELOW_TO_ABOVE:
	case ABOVE_TO_BELOW:
	    for (j = 0;  j < nx;  ++j)
	    {
	    	if (*(start+j) == -1) break;
	    }

	    if (j == 0)
	    	*start = *n_crx;
	    else
	    {
	    	for (i = 0;  i < j;  ++i)
	    	{
	    	    if (Coords((crx_store[*(start+i)]).pt)[0] >
						 Coords(cross->pt)[0])
			break;
		}

		if (i < j)
		{
		    input = *n_crx;
		    for (k = i;  k < j;  ++k)
		    {
		    	hold = *(start+k);
		    	*(start+k) = input;
		    	input = hold;
		    }
		    *(start+j) = input;
		}
		else
		    *(start+j) = *n_crx;
	    }
	    break;
	case LEFT_TO_RIGHT:
	case RIGHT_TO_LEFT:
	    for (j = 0;  j < nx;  ++j)
	    {
	    	if (*(start+j) == -1) break;
	    }

	    if (j == 0)
	    	*start = *n_crx;
	    else
	    {
	    	for (i = 0;  i < j;  ++i)
	    	{
	    	    if (Coords((crx_store[*(start+i)]).pt)[1] >
	    				 Coords(cross->pt)[1])
	    		    break;
	    	}

	    	if (i < j)
	    	{
	    	    input = *n_crx;
	    	    for (k = i;  k < j;  ++k)
	    	    {
	    	    	hold = *(start+k);
	    	    	*(start+k) = input;
	    	    	input = hold;
	    	    }
	    	    *(start+j) = input;
	    	}
	    	else
	    	    *(start+j) = *n_crx;
	    }
	    break;
	}
	return GOOD_STEP;
}		/*end add_comp_crx_to_list2d*/

LOCAL   boolean remove_unphysical_crossings2d_on_grid(
        INTERFACE  *oldintfc,
        INTERFACE  *grid_intfc,
        int        *n_fr_blk,
        int        *smin,
        int        *smax)
{
	struct Table	*T = table_of_interface(grid_intfc);
        int        *gmax;
        COMPONENT  *comp;
        COMPONENT  **ocom, oc;
        COMPONENT  **ncom, nc;
        int        ixm,iym;
        int        i, ix, iy, n_reg_node, ip[2];
        CURVE      **cc;

        gmax = T->aug_comp_grid.gmax;
        comp = T->c_components;
        n_reg_node = (gmax[0]+1)*(gmax[1]+1);
        ocom = (table_of_interface(oldintfc))->compon2d;
        ncom = (table_of_interface(grid_intfc))->compon2d;       

        for (i = 0; i < n_reg_node; i++)
            comp[i] = NO_COMP;

        for (iy = smin[1]+1; iy < smax[1]-1; iy++)
        {
            for (ix = smin[0]+1; ix < smax[0]-1; ix++)
            {
                oc = ocom[iy-1][ix-1];
                nc = ncom[iy][ix];

                if (oc != ONFRONT && nc != ONFRONT)
                {
                    ixm = ix + 1;
                    iym = iy + 1;
                    for (ip[1] = iy; ip[1] <= iym; (ip[1])++)
                    {
                        for (ip[0] = ix; ip[0] <= ixm; (ip[0])++)
                        {
                            comp[d_index2d(ip[0],ip[1],gmax)] = oc;
                        }
                    }
                }
            }
        }

        if (!track_comp_through_crxings2d_on_grid(smin,smax,gmax,grid_intfc,
					SINGLE))
        {
            DEBUG_LEAVE(remove_unphysical_crossings2d)
            return FUNCTION_FAILED;
        }

        *n_fr_blk = 0;
        for (iy = smin[1]; iy < smax[1]; iy++)
        {
            for (ix = smin[0]; ix < smax[0]; ix++)
            {
                nc = ncom[iy][ix];
                if (nc == ONFRONT) (*n_fr_blk)++;
            }
        }

        return FUNCTION_SUCCEEDED;
        DEBUG_LEAVE(remove_unphysical_crossings2d)
}


LOCAL  boolean track_comp_through_crxings2d_on_grid(
        int             *smin,
        int             *smax,
        int             *gmax,
        INTERFACE	*grid_intfc,
        CRX_TYPE        crx_type)
{
	struct Table	*T = table_of_interface(grid_intfc);
        int             ix, iy, i, step;
        int             ip[2],ipn[2];
        COMPONENT       *comp;
        COMPONENT       c,cn;
        CURVE           **cc;

        DEBUG_ENTER(track_comp_through_crxings2d)

        comp = T->c_components;

        eliminate_same_crossings2d(grid_intfc,smin,smax);

        for (ix = smin[0]; ix <= smax[0]; ix++)
        {
            ip[0] = ipn[0] = ix;
            for (iy = smin[1]; iy <= smax[1]; iy++)
            {
                ip[1] = iy;
                c = comp[d_index2d(ip[0],ip[1],gmax)];
                if (c == NO_COMP) continue;

                if (iy != 0)
                {
                    ipn[1] = iy - 1;
                    cn = comp[d_index2d(ipn[0],ipn[1],gmax)];
                    if (cn == NO_COMP)
                    {
                         step = walk_comp_along_grid_line2d(grid_intfc,smin,smax,
                                        gmax,ip,SOUTH);
                    }
                }
                if (iy != smax[1])
                {
                    ipn[1] = iy + 1;
                    cn = comp[d_index2d(ipn[0],ipn[1],gmax)];
                    if (cn == NO_COMP)
                    {
                         step = walk_comp_along_grid_line2d(grid_intfc,smin,smax,
                                        gmax,ip,NORTH);
                         iy +=step;
                    }
                }
            }
        }
        for (iy = smin[1]; iy <= smax[1]; iy++)
        {
            ip[1] = ipn[1] = iy;
            for (ix = smin[0]; ix <= smax[0]; ix++)
            {
                ip[0] = ix;
                c = comp[d_index2d(ip[0],ip[1],gmax)];
                if (c == NO_COMP) continue;

                if (ix != 0)
                {
                    ipn[0] = ix - 1;
                    cn = comp[d_index2d(ipn[0],ipn[1],gmax)];
                    if (cn == NO_COMP)
                    {
                         step = walk_comp_along_grid_line2d(grid_intfc,smin,smax,
                                        gmax,ip,WEST);
                    }
                }
                if (ix != smax[0])
                {
                    ipn[0] = ix + 1;
                    cn = comp[d_index2d(ipn[0],ipn[1],gmax)];
                    if (cn == NO_COMP)
                    {
                         step = walk_comp_along_grid_line2d(grid_intfc,smin,smax,
                                        gmax,ip,EAST);
                         ix +=step;
                    }
                }
            }
        }

        /* remove unphysical crossings */
        for (ix = smin[0]; ix <= smax[0]; ix++)
        {
            ip[0] = ipn[0] = ix;
            for (iy = smin[1]; iy <= smax[1]; iy++)
            {
                ip[1] = ipn[1] = iy;
                c = comp[d_index2d(ip[0],ip[1],gmax)];
                if (c == NO_COMP) continue;

                if (iy != 0)
                {
                    rm_unphy_crx_along_grid_line2d(grid_intfc,smin,smax,
                                 gmax,ip,SOUTH,crx_type);
                }
                if (iy != smax[1])
                {
                    rm_unphy_crx_along_grid_line2d(grid_intfc,smin,smax,
                                 gmax,ip,NORTH,crx_type);
                }
            }
        }
        for (iy = smin[1]; iy <= smax[1]; iy++)
        {
            ip[1] = ipn[1] = iy;
            for (ix = smin[0]; ix <= smax[0]; ix++)
            {
                ip[0] = ix;
                c = comp[d_index2d(ip[0],ip[1],gmax)];
                if (c == NO_COMP) continue;
                if (ix != 0)
                {
                    rm_unphy_crx_along_grid_line2d(grid_intfc,smin,smax,
                                 gmax,ip,WEST,crx_type);
                }
                if (ix != smax[0])
                {
                    rm_unphy_crx_along_grid_line2d(grid_intfc,smin,smax,
                                 gmax,ip,EAST,crx_type);
                }
            }
        }

	/*
        if (! check_grid_comp_and_crx2d(grid_intfc,smin,smax,which_grid))
        {
            DEBUG_LEAVE(track_comp_through_crxings3d)
            return FUNCTION_FAILED;
        }*/

        DEBUG_LEAVE(track_comp_through_crxings2d)
        return FUNCTION_SUCCEEDED;
}

LOCAL   void eliminate_same_crossings2d(
        INTERFACE	*grid_intfc,
        int             *smin,
        int             *smax)
{
	struct Table	*T = table_of_interface(grid_intfc);
        int             ix,iy;
        GRID_DIRECTION  dir[2] = {EAST,NORTH};
        int             ip[2],i,k,l,m,nc,list1,list2;
        CRXING          *crx1,*crx2;
        double           grid_crds;
        double           *L;
        double           *h;
        int             *gmax = T->aug_comp_grid.gmax;
	int             error_comp_cnt = 0, same_crxing_cnt = 0;
        int             *seg_crx_count;
        int             **seg_crx_lists;
        CRXING          *crx_store;
 
        DEBUG_ENTER(eliminate_same_crossings2d)

        L = T->aug_comp_grid.L;
        h = T->aug_comp_grid.h;
        seg_crx_count = T->c_seg_crx_count;
        seg_crx_lists = T->c_seg_crx_lists;
        crx_store = T->c_crx_store;

        for (iy = smin[1]; iy <= smax[1]; iy++)
        {
            ip[1] = iy;
            for (ix = smin[0]; ix <= smax[0]; ix++)
            {
                ip[0] = ix;
                for (i = 0; i < 2; i++)
                {
                    if (ix == smax[0] && dir[i] == EAST)
                        continue;
                    if (iy == smax[1] && dir[i] == NORTH)
                        continue;
                    k = seg_index2d(ip[0],ip[1],dir[i],gmax);
                    nc = seg_crx_count[k];

                    if (nc == 0) continue;
                    else
                    {
                        for (l = 0; l < nc; l++)
                        {
                            list1 = seg_crx_lists[k][l];
                            crx1 = &(crx_store[list1]);
                            if (crx1->lcomp == NO_COMP)
                            {
                                for (m = l; m < nc - 1; m++)
                                    seg_crx_lists[k][m] =
                                            seg_crx_lists[k][m+1];
                                l--;
                                nc--;
                                seg_crx_count[k]--;
                                error_comp_cnt++;
                            }
                        }
                        if (nc == 0) continue;
                        list1 = seg_crx_lists[k][0];
                        crx1 = &(crx_store[list1]);
                        adjust_for_min_spacing2d(crx1,L,ip,h,nc,i,k,
                           seg_crx_lists,crx_store);
                    }
                }
            }
        }

        DEBUG_LEAVE(eliminate_same_crossings2d)
}

LOCAL   int     walk_comp_along_grid_line2d(
        INTERFACE      *grid_intfc,
        int            *smin,
        int            *smax,
        int            *gmax,
        int            *ip,
        GRID_DIRECTION dir)
{
	struct Table	*T = table_of_interface(grid_intfc);
        COMPONENT       *comp;
        int             *seg_crx_count;
        int             **seg_crx_lists;
        CRXING          *crx_store;
        COMPONENT       current_comp;
        CRXING          *crx;
        int             ip1[3],ip2[3];
        int             i,k,nc,list,step;
        boolean            crx_is_physical = YES;

        comp = T->c_components;
        seg_crx_count = T->c_seg_crx_count;
        seg_crx_lists = T->c_seg_crx_lists;
        crx_store = T->c_crx_store;

        for (i = 0; i < 2; i++) ip1[i] = ip[i];
        current_comp = comp[d_index2d(ip1[0],ip1[1],gmax)];
        step = 0;
        while (next_ip_in_dir(ip1,dir,ip2,smin,smax))
        {
            if (comp[d_index2d(ip2[0],ip2[1],gmax)] != NO_COMP)
                break;
            k = seg_index2d(ip1[0],ip1[1],dir,gmax);
            nc = seg_crx_count[k];
            if (dir == EAST || dir == NORTH)
            {
                for (i = 0; i < nc; i++)
                {
                    list = seg_crx_lists[k][i];
                    crx = &(crx_store[list]);
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
                for (i = nc-1; i >= 0; i--)
                {
                    list = seg_crx_lists[k][i];
                    crx = &(crx_store[list]);
                    if (crx->ucomp == current_comp)
                        current_comp = crx->lcomp;
                    else
                    {
                        crx_is_physical = NO;
                        break;
                    }
                }
            }
            if (! crx_is_physical)
                break;
            step++;
            comp[d_index2d(ip2[0],ip2[1],gmax)] = current_comp;
            for (i = 0; i < 3; i++) ip1[i] = ip2[i];
        }

        DEBUG_LEAVE(walk_comp_along_grid_line2d)
        return step; 
}

LOCAL   void    rm_unphy_crx_along_grid_line2d(
        INTERFACE	*grid_intfc,
        int             *smin,
        int             *smax,
        int             *gmax,
        int             *ip,
        GRID_DIRECTION  dir,
        CRX_TYPE        crx_type)
{
	struct Table	*T = table_of_interface(grid_intfc);
        COMPONENT       *comp;
        int             *seg_crx_count;
        int             **seg_crx_lists;
        CRXING          *crx_store;
        COMPONENT       current_comp;
        CRXING          *crx;
        int             ip1[3],ip2[3];
        int             i,k,nc,list,l;
        double           *L;
        double           *h;
            
	comp = T->c_components;
        seg_crx_count = T->c_seg_crx_count;
        seg_crx_lists = T->c_seg_crx_lists;
        crx_store = T->c_crx_store;
        L = T->aug_comp_grid.L;
        h = T->aug_comp_grid.h;

        for (i = 0; i < 2; i++) ip1[i] = ip[i];
        current_comp = comp[d_index2d(ip1[0],ip1[1],gmax)];

        while (next_ip_in_dir(ip1,dir,ip2,smin,smax))
        {
            k = seg_index2d(ip1[0],ip1[1],dir,gmax);
            nc = seg_crx_count[k];
            if (nc == 0) return;

            if (dir == EAST || dir == NORTH)
            {
                for (i = 0; i < nc; i++)
                {
                    list = seg_crx_lists[k][i];
                    crx = &(crx_store[list]);
                    if (crx->lcomp == current_comp)
                        current_comp = crx->ucomp;
                    else
                    {
                        for (l = i; l < nc-1; l++)
                            seg_crx_lists[k][l] =
                                seg_crx_lists[k][l+1];
                        i--;       /*why using this ?? */
                        nc--;
                        seg_crx_count[k]--;
                    }
                }
            }
            else
            {
                for (i = nc-1; i >= 0; i--)
                {
                    list = seg_crx_lists[k][i];
                    crx = &(crx_store[list]);
                    if (crx->ucomp == current_comp)
                        current_comp = crx->lcomp;
                    else
                    {
                        for (l = i; l < nc-1; l++)
                            seg_crx_lists[k][l] =
                                seg_crx_lists[k][l+1];
                        nc--;
                        seg_crx_count[k]--;
                    }
                }
            }

            /* The current code can only handle one tracked curve.
             * The appearing of the multi-component is purely caused
             * by the boundary curves. The following reducing crxing
             * rules should only be applied to the tracked physical wave.
             */
            if (crx_type == SINGLE && nc != 1 &&
                (dir == WEST || dir == SOUTH))
            {
                if (nc%2 == 0)
                {
                    rm_select_even_crx(k, seg_crx_count, seg_crx_lists, crx_store);
                    /*
                    seg_crx_count[k] = 0;
                    */
                }
                else if(seg_crx_count[k] != 1)
                {
                    /*
                    if(seg_crx_count[k] != 1)
                        find_mid_crx_left(grid_intfc,k,ip1,ip2,dir,which_grid);
                    */
                    rm_select_odd_crx(k, seg_crx_count, seg_crx_lists, crx_store);
                    /*
                    seg_crx_count[k] = 1;
                    */
                }
            }
            if (comp[d_index2d(ip2[0],ip2[1],gmax)] != NO_COMP)
                break;
            comp[d_index2d(ip2[0],ip2[1],gmax)] = current_comp;
            for (i = 0; i < 2; i++) ip1[i] = ip2[i];
        }

        DEBUG_LEAVE(rm_unphy_crx_along_grid_line2d)
        return; 
}



LOCAL   boolean reconstruct_crx_intfc2d(
        INTERFACE    *grid_intfc,
        int          n_fr_blk,
        int          *smin,
        int          *smax)
{
        int             *gmax;
        int             *seg_crx_count;
        int             **seg_crx_lists;
        int             xmax,ymax;
        CRXING          *crx_store;
        COMPONENT       *comp;
        BLK_BOND        ***blk_mem,*blk_mem_store;
        COMPONENT       **coy, *coyx;
        int             ix, iy, ixx, iyy,ix_r,iy_r;       
				/*ix_r and iy_r is the real icrds  */
        int             i, j, k, nbc, ip[3], num_curves;
        CURVE           **c;  
        CURVE           **curves = NULL; 
        int             alloc_cn = 0;

        static BLK_CRX2 *blk_crx = NULL;
        struct Table    *T;
        COMPONENT       blk_comp[2][2];
        int             blk_ip[2];
        int             l, nc, nn, list;
        double           blk_start[2];   /*start position of the blk */
        CRXING          *crx;
	RECT_GRID	*comp_grid = computational_grid(grid_intfc);
	COMPONENT	elliptic_comp = grid_intfc->elliptic_comp;

        T = table_of_interface(grid_intfc);

        xmax = comp_grid->gmax[0];
        ymax = comp_grid->gmax[1];

        gmax = T->aug_comp_grid.gmax;
        comp = T->c_components;
        seg_crx_count = T->c_seg_crx_count;
        seg_crx_lists = T->c_seg_crx_lists;
        crx_store = T->c_crx_store;

        num_curves = 0;
        for(c = grid_intfc->curves; c && *c; c++)
            num_curves++;
        if(curves == NULL)
        {
            alloc_cn = num_curves + MAX_NUM_CURVES; 
            uni_array(&curves, alloc_cn, sizeof(CURVE*)); 
        }

        nbc = 0;
        for(i = 0, c = grid_intfc->curves; c && *c; i++, c++)
        {
            curves[i] = *c;
        }

        if (blk_crx == NULL)
            alloc_blk_crx2d_on_grid(&blk_crx);
        
	bi_array(&blk_mem, smax[1]-smin[1],smax[0]-smin[0], sizeof(BLK_BOND*));
        uni_array(&blk_mem_store, n_fr_blk, sizeof(BLK_BOND));

        coy = (table_of_interface(grid_intfc))->compon2d; 

        for (iy = smin[1]; iy < smax[1]; iy++)
        {
            for(coyx = coy[iy], ix = smin[0]; ix < smax[0]; ix++)
            {
                ixx = ix - smin[0]; ix_r = ix - 1 - comp_grid->lbuf[0];
                iyy = iy - smin[1]; iy_r = iy - 1 - comp_grid->lbuf[1];

                if (coyx[ix] == ONFRONT)
                {
                    boolean   multi_crxing = NO;

                    for (i = 0; i < 2; i++)
                    {
                        ip[0] = ix + i;
                        for (j = 0; j < 2; j++)
                        {
                            ip[1] = iy + j;
                            blk_crx->comp[i][j][0] =
                                     comp[d_index2d(ip[0],ip[1],gmax)];
                            blk_crx->ix[i][j][0] = i; 
                            blk_crx->iy[i][j][0] = j;
                            blk_comp[i][j] = blk_crx->comp[i][j][0];
                        }
                    }
                    for (i = 0; i < 2; i++)
                    {
                        blk_ip[0] = ix;
                        blk_ip[1] = iy + i;
                        l = seg_index2d(blk_ip[0],blk_ip[1],EAST,gmax);
                        nc = seg_crx_count[l];

                        if(nc > 1)
                        {
                            multi_crxing = YES;
                            screen("\n WARNING!, %d crxing occuring ",nc);
			    screen("for blk [%d %d]",ix_r,iy_r);
                        }

                        if(nc == 0 && blk_comp[0][i] != blk_comp[1][i])
                        {
                             screen("ERROR in reconstruct_crx_intfc2d() \n");
                             screen("nc = 0\n");
                             clean_up(ERROR); 
                        }
                        if(nc != 0)
                        {
                            for(nn = 0; nn < nc; nn++)
                            {
                                list = seg_crx_lists[l][nn];
                                crx = &crx_store[list];
                                blk_crx->crx[0][i][0][nn].hs = crx->hs;
                                blk_crx->crx[0][i][0][nn].p = crx->pt;
                                blk_crx->crx[0][i][0][nn].lcomp = crx->lcomp;
                                blk_crx->crx[0][i][0][nn].ucomp = crx->ucomp;
                            }
                             blk_crx->n_crx[0][i][0] = nc;
                        }
                        else
                        {
                             blk_crx->crx[0][i][0][0].p = NULL;
                             blk_crx->n_crx[0][i][0] = 0;
                        }

                        blk_ip[0] = ix + i;
                        blk_ip[1] = iy;
                        l = seg_index2d(blk_ip[0],blk_ip[1],NORTH,gmax);
                        nc = seg_crx_count[l];

                        if(nc > 1)
                        {
                            multi_crxing = YES;
                            screen("\n WARNING!, %d crxing occuring ",nc);
			    screen("for blk [%d %d]",ix_r,iy_r);
                        }
                        if(nc == 0 && blk_comp[i][0] != blk_comp[i][1])
                        {
                             screen("ERROR in reconstruct_crx_intfc2d() \n");
                             screen("nc = 0, north\n");
                             clean_up(ERROR); 
                        }
                        if(nc != 0)
                        {
                            for(nn = 0; nn < nc; nn++)
                            {
                                list = seg_crx_lists[l][nn];
                                crx = &crx_store[list];
                                blk_crx->crx[1][0][i][nn].hs = crx->hs;
                                blk_crx->crx[1][0][i][nn].p = crx->pt;
                                blk_crx->crx[1][0][i][nn].lcomp = crx->lcomp;
                                blk_crx->crx[1][0][i][nn].ucomp = crx->ucomp;
                            }
                            blk_crx->n_crx[1][0][i] = nc;
                        }
                        else
                        {
                             blk_crx->crx[1][0][i][0].p = NULL;
                             blk_crx->n_crx[1][0][i] = 0;
                        }
                    }
                    
                    if(ix_r >= 0&&ix_r < xmax&&iy_r >= 0&&iy_r < ymax)
                    {
		        if(multi_crxing == YES)
                            T->blk_edge[ix_r][iy_r].ctype = EXTERNAL;
                        else
                        { 
                           blk_start[0] = comp_grid->L[0] + 
			   		ix_r*comp_grid->h[0];
                           blk_start[1] = comp_grid->L[1] + 
			   		iy_r*comp_grid->h[1];
                           init_blk_edges(blk_crx,&(T->blk_edge[ix_r][iy_r]),
					   blk_start,comp_grid->h); 
                        }      
                    } 
                }
                else
                {
                    blk_mem[iyy][ixx] = NULL;
                    for (i = 0; i < 2; i++)
                    {
                        ip[0] = ix + i;
                        for (j = 0; j < 2; j++)
                        {
                            ip[1] = iy + j;
                            blk_comp[i][j] = comp[d_index2d(ip[0],ip[1],gmax)];
                        }
                    }
                    /* since blk is off_front, so type can only be 
		     * EXTERNAL or INTERNAL*/
		    
		    if(ix_r >= 0&&ix_r < xmax&&iy_r >= 0&&iy_r < ymax)
                    {
                        if(blk_comp[0][0] == elliptic_comp)
                            T->blk_edge[ix_r][iy_r].ctype = INTERNAL;
                        else
                            T->blk_edge[ix_r][iy_r].ctype = EXTERNAL;
		    }
                }
            }
        }

	free(blk_mem);
        free(blk_mem_store);
        free(curves); 
        DEBUG_LEAVE(reconstruct_crx_intfc2d)
        return FUNCTION_SUCCEEDED;
}	/* end reconstruct_crx_intfc2d */

/* NOTE: the rgr used here should be
 * tri_grid->aug_comp_grid, The 
 * aug_comp_grid->G[L,U] is set to be the
 * the buffered computational grid subdomain.
 */ 

LOCAL int shift_from_cell_edge_on_comp(
        double           *p,
        int             idir,
        RECT_GRID       *rgr,
        double           tol)
{
        int             ic;
        double           sfrac;
        double           *L = rgr->GL;
        double           *U = rgr->GU;

        sfrac = fabs(p[idir] - L[idir]) / cell_width(ic,idir,rgr);
        if(sfrac < tol) /* close to L bdry */
        {
            p[idir] += tol;
            ic = cell_index(p[idir],idir,rgr);
            return ic; 
        }

        sfrac = fabs(p[idir] - U[idir]) / cell_width(ic,idir,rgr);
        if(sfrac < tol) /* close to U bdry */
        {
            p[idir] -= tol;
            ic = cell_index(p[idir],idir,rgr);
            return ic; 
        }

        ic = cell_index(p[idir],idir,rgr);
        sfrac = (p[idir] - rgr->edges[idir][ic]) / cell_width(ic,idir,rgr);
        if (sfrac < tol)                p[idir] -= tol;
        else if (sfrac > 1.0-tol)       p[idir] += tol;
        return ic;
}	/* end shift_from_cell_edge_on_comp */

LOCAL   void init_blk_edges(
        BLK_CRX2        *blk_crx,
	BLK_EDGE        *blk_edge,
        double           *blk_start,
        double           *h)
{
        int       i,j,k;
        int       num_comp;
        int       total = 0;

        for(i = 0; i < 2; ++i)
        {
            for(j = 0; j < 2; ++j)
            {
                int       comp_crd[2][2];
                double     **coords;
                FT_POINT     *p;

		bi_array(&coords,2,2,sizeof(double));

                num_comp = count_component2d(blk_crx->comp,i,j,comp_crd);
                total += num_comp; 

                if(num_comp == 0)         /*non surface */
                    blk_edge->edge[i][j].etype = NONE_EDGE;
                else if(num_comp == 2) 
                    blk_edge->edge[i][j].etype = FULL_EDGE;
                else       /*in this case, num_comp can only be 1 */
                {
                    blk_edge->edge[i][j].etype = PARTIAL_EDGE;

                    for(k = 0; k < 2; ++k)
                       coords[0][k] = blk_start[k] + comp_crd[0][k]*h[k]; 
                    
                    if(i == 0)
                       p = blk_crx->crx[1][0][j][0].p;
                    if(i == 1)
                       p = blk_crx->crx[0][j][0][0].p;
                    coords[1] = Coords(p);
                                      
                    blk_edge->edge[i][j].cen[0] = 
		    		0.5*(coords[0][0]+coords[1][0]);
                    blk_edge->edge[i][j].cen[1] = 
		    		0.5*(coords[0][1]+coords[1][1]); 

                    blk_edge->edge[i][j].length = 
		    		distance_between_positions(coords[0],coords[1],
				2); 
                }
		free(coords);
            }
        }
        if(total == 0)
            blk_edge->ctype = EXTERNAL;  /*in case of multiple crxing */
        else if(total == 8)              /*every point counted twice; */
            blk_edge->ctype = INTERNAL;
        else 
            blk_edge->ctype = PARTIAL;
}       /* end init_blk_edges */ 

LOCAL   int  count_component2d(
         COMPONENT    ***comp,
         int          i,
         int          j,
         int          comp_crd[][2])
{
         int    m;
         int    num_comp = 0;

         if(i == 0)
         {
             for(m = 0; m < 2; ++m)
             {
                 if(comp[j][m][0] == 2)
                 {
                     comp_crd[num_comp][0] = j;
                     comp_crd[num_comp][1] = m;
                     num_comp++;
                 }
             }
         } 
         else if(i == 1)
         {
             for(m = 0; m < 2; ++m)
             {
                 if(comp[m][j][0] == 2)
                 {
                     comp_crd[num_comp][0] = m;
                     comp_crd[num_comp][1] = j;
                     num_comp++;
                 }
             }
         }
         return num_comp;
}	/* end count_component2d */

LOCAL void adjust_for_min_spacing2d(
        CRXING          *crxing,
        double           *L,
        int             *ip,
        double           *h,
        int             n_crx,
        int             dir,
        int             k,
        int             **seg_crx_lists,
        CRXING          *crx_store)
{

        int             i;
        double           crds_start[2], crds_end[2];
        double           mgs = 0.001*h[dir];/*TOLERANCE old = 0.004, 0.008*/
        int             list1;
        CRXING          *crxings;

        for(i = 0; i < 2; i++)
        {
            crds_start[i] = L[i] + ip[i]*h[i];
            crds_end[i] = crds_start[i] + h[i];
        }
        for (i = 0; i < n_crx; i++)
        {
            list1 = seg_crx_lists[k][i];
            crxings = &(crx_store[list1]);
            if(wave_type(crxings->hs) < FIRST_PHYSICS_WAVE_TYPE)
                continue;  
            if(crxings->crossing_direction == BELOW_TO_ABOVE ||
               crxings->crossing_direction == ABOVE_TO_BELOW)
            {
                if(fabs(Coords(crxings->pt)[0] - crds_start[0]) < mgs &&
                   Coords(crxings->pt)[0] > crds_start[0])
                {
                    Coords(crxings->pt)[0] = crds_start[0] + mgs;
                }
                if(fabs(Coords(crxings->pt)[0] - crds_start[0]) < mgs &&
                   Coords(crxings->pt)[0] < crds_start[0])
                {
                    Coords(crxings->pt)[0] = crds_start[0] - mgs;
                }
                if(fabs(crds_end[0] - Coords(crxings->pt)[0]) < mgs &&
                   Coords(crxings->pt)[0] > crds_end[0])
                {
                    Coords(crxings->pt)[0] = crds_end[0] + mgs;
                }
                if(fabs(crds_end[0] - Coords(crxings->pt)[0]) < mgs &&
                   Coords(crxings->pt)[0] < crds_end[0])
                {
                    Coords(crxings->pt)[0] = crds_end[0] - mgs;
                }
            }
            else if(crxings->crossing_direction == RIGHT_TO_LEFT ||
                    crxings->crossing_direction == LEFT_TO_RIGHT)
            {
                if(fabs(Coords(crxings->pt)[1] - crds_start[1]) < mgs &&
                   Coords(crxings->pt)[1] > crds_start[1])
                {
                    Coords(crxings->pt)[1] = crds_start[1] + mgs;
                }
                if(fabs(Coords(crxings->pt)[1] - crds_start[1]) < mgs &&
                   Coords(crxings->pt)[1] < crds_start[1])
                {
                    Coords(crxings->pt)[1] = crds_start[1] - mgs;
                }
                if(fabs(crds_end[1] - Coords(crxings->pt)[1]) < mgs &&
                   Coords(crxings->pt)[1] > crds_end[1])
                {
                    Coords(crxings->pt)[1] = crds_end[1] + mgs;
                }
                if(fabs(crds_end[1] - Coords(crxings->pt)[1]) < mgs &&
                   Coords(crxings->pt)[1] < crds_end[1])
                {
                    Coords(crxings->pt)[1] = crds_end[1] - mgs;
                }
            }
            else
            {
                screen("ERROR in adjust_for_min_spacing2d()\n");
                screen("CRXING %d 's direction is not set\n", crxings);
                screen("%f, %f\n", Coords(crxings->pt)[0],
				Coords(crxings->pt)[1]);
                print_crxings(crxings, NO);
                print_curve(Curve_of_hs(crxings->hs));
                clean_up(ERROR);
            }
        }
}	/* end adjust_for_min_spacing2d */

LOCAL void rm_select_even_crx(
	int        k, 
        int        *seg_crx_count, 
        int        **seg_crx_lists, 
        CRXING     *crx_store)
{
        int        nc = seg_crx_count[k];
        int        list, i, j;
        CRXING     *crx;
        HYPER_SURF *hs[20]; /* Enough ? */
        int        diff, n_trk_w = 0, n_trk_c;
        int        n_un_trk = 0; 

        for(i = 0; i < nc; i++)
        {
            list = seg_crx_lists[k][i];
            crx = &(crx_store[list]);
            if(wave_type(crx->hs) >= FIRST_PHYSICS_WAVE_TYPE)
            {
                hs[0] = crx->hs;
                n_trk_w = 1;
                break;
            }
        }
        if(n_trk_w == 0)
            return;

        n_trk_c = n_trk_w;
        for(i = 0; i < nc; i++)
        {
            list = seg_crx_lists[k][i];
            crx = &(crx_store[list]);
            if(wave_type(crx->hs) < FIRST_PHYSICS_WAVE_TYPE)
            {
                n_un_trk++; 
                continue;
            }
            diff = YES;
            for(j = 0; j < n_trk_c; j++)
            {
                if(crx->hs == hs[j])
                {
                    diff = NO;
                    break;
                }
            }
            if(diff == YES)
            {
                hs[n_trk_c] = crx->hs;
                n_trk_c++;
            }
        }
 
        if(n_un_trk != 0)
        {
            screen("ERROR: in rm_unphy_crx_along_grid_line2d\n");
            screen("seg_crx_count[%d] = %d is even\n",k, seg_crx_count[k]); 
            screen("Contains tracked and untracked crxs\n");
	    screen("n_un_trk = %d\n",n_un_trk); 
            screen("Need to implement rm_select_even_crx\n");
            for(i = 0; i < nc; i++)
            {
                list = seg_crx_lists[k][i];
                crx = &(crx_store[list]);
                print_crxings(crx,YES); 
            }
	    clean_up(ERROR);
        }

        if(n_trk_c == 1)
            seg_crx_count[k] = 0; 
        else
        {
            screen("ERROR: in rm_unphy_crx_along_grid_line2d\n");
            screen("seg_crx_count[%d] = %d is even\n",k, seg_crx_count[k]); 
            screen("tracked crxings belong to diff curves  n_trk_c[%d]\n",
	    				n_trk_c); 
            screen("Need to implement rm_select_even_crx\n");
            for(i = 0; i < nc; i++)
            {
                list = seg_crx_lists[k][i];
                crx = &(crx_store[list]);
                print_crxings(crx,YES); 
            }
	    clean_up(ERROR);
        }
}

LOCAL void rm_select_odd_crx(
	int        k, 
        int        *seg_crx_count, 
        int        **seg_crx_lists, 
        CRXING     *crx_store)
{
        int        nc = seg_crx_count[k];
        int        list, i, j; 
        CRXING     *crx; 
        HYPER_SURF *hs[20]; /* Enough ? */
        int        diff, n_tracked_w = 0, n_tracked_c; 

        for(i = 0; i < nc; i++)
        {
            list = seg_crx_lists[k][i];
            crx = &(crx_store[list]);
            if(wave_type(crx->hs) >= FIRST_PHYSICS_WAVE_TYPE)
            {
                hs[0] = crx->hs;
                n_tracked_w = 1;
                break; 
            }
        }
        if(n_tracked_w == 0) 
            return; 

        n_tracked_c = n_tracked_w; 
        for(i = 0; i < nc; i++)
        {
            list = seg_crx_lists[k][i];
            crx = &(crx_store[list]);
            if(wave_type(crx->hs) < FIRST_PHYSICS_WAVE_TYPE)
                continue; 
            diff = YES; 
            for(j = 0; j < n_tracked_c; j++)
            {
                if(crx->hs == hs[j])
                {
                    diff = NO;
                    break;
                }
            }
            if(diff == YES)
            {
                hs[n_tracked_c] = crx->hs;
                n_tracked_c++;                    
            }
        }

        if(n_tracked_c != 1)
        {
            screen("ERROR: in rm_unphy_crx_along_grid_line2d\n");
            screen("seg_crx_count[%d] = %d is even\n",k, seg_crx_count[k]); 
            screen("Need to implement rm_select_odd_crx\n");
	    screen("n_tracked curve = %d\n",n_tracked_c);
	    clean_up(ERROR);
        }
}	/* rm_select_odd_crx */

LOCAL  void alloc_blk_crx2d_on_grid(
        BLK_CRX2      **blk_crx2)
{
        int           i, j, k; 
        stat_scalar(blk_crx2,sizeof(BLK_CRX2));
        stat_tri_array(&(*blk_crx2)->comp,2,2,2,sizeof(COMPONENT));
        stat_tri_array(&(*blk_crx2)->ix,2,2,2,sizeof(int));
        stat_tri_array(&(*blk_crx2)->iy,2,2,2,sizeof(int));
        stat_tri_array(&(*blk_crx2)->iz,2,2,2,sizeof(int));
        stat_tri_array(&(*blk_crx2)->crx,3,2,2,sizeof(BBI_POINT2*));
        stat_tri_array(&(*blk_crx2)->n_crx,3,2,2,sizeof(int));
        uni_array(&(*blk_crx2)->crx_store,20,sizeof(BBI_POINT2));

        k = 0; 
        for(i = 0; i < 2; i++)
        {
            if(i != 0)
                k += 5; 
            (*blk_crx2)->crx[0][i][0] = (*blk_crx2)->crx_store + k;
        }

        for(i = 0; i < 2; i++)
        {
            k += 5; 
            (*blk_crx2)->crx[1][0][i] = (*blk_crx2)->crx_store + k;
        }
}               /*end alloc_blk_crx2d*/


LOCAL   void set_comp_at_crossings2d1(
        INTERFACE        *grid_intfc)
{
        int             minx, miny, maxx, maxy;
        int             i, j, ix, iy;
        struct Table    *T;
        BOND            ****bonds, **byx;
        CURVE           ****curves, **cyx;
        COMPONENT       **coy, *coyx, c;
        int             num_of_bonds;
        int             smin[MAXD], smax[MAXD], ip[MAXD], ipn[MAXD];
        int             *seg_crx_count;
        int             **seg_crx_lists;
        CRXING          *crx_store, *crx;
        int             m,k,nc,list;

        T = table_of_interface(grid_intfc);

        for (i = 0; i < 3; i++)
        {
            smin[i] = 0;
            smax[i] = T->aug_comp_grid.gmax[i]; /*expanded_comp_grid*/
        }
        seg_crx_count = T->c_seg_crx_count;
        seg_crx_lists = T->c_seg_crx_lists;
        crx_store = T->c_crx_store;
	
        coy = T->compon2d;
        for (iy = smin[1]; iy < smax[1]; iy++)
        {
            ip[1] = iy;
            for (coyx = coy[iy], ix = smin[0]; ix < smax[0]; ix++)
            {
                ip[0] = ix; c = coyx[ix];
                if(c == ONFRONT)
                {
                    byx = T->bonds[iy][ix];
                    cyx = T->curves[iy][ix];
                    num_of_bonds = T->num_of_bonds[iy][ix];

                    /* first EAST  */
                    ipn[0] = ip[0]+1;  ipn[1] = ip[1];
                    k = seg_index2d(ip[0],ip[1],EAST,smax);
                    nc = seg_crx_count[k];
                    for (i = 0; i < nc; i++)
                    {
                        list = seg_crx_lists[k][i];
                        crx = &(crx_store[list]);
                        for(j = 0; j <num_of_bonds; j++)
                        {
                            if(point_on_bond2d1(crx->pt, byx[j]))
                            {
                                if(crx->crossing_direction ==  BELOW_TO_ABOVE)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if( crx->crossing_direction == ABOVE_TO_BELOW)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == LEFT_TO_RIGHT)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == RIGHT_TO_LEFT)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                            }
                        }
                    }
                    /* first NORTH  */
                    ipn[0] = ip[0]+1;  ipn[1] = ip[1];
                    k = seg_index2d(ip[0],ip[1],NORTH,smax);
                    nc = seg_crx_count[k];
                    for (i = 0; i < nc; i++)
                    {
                        list = seg_crx_lists[k][i];
                        crx = &(crx_store[list]);
                        for(j = 0; j <num_of_bonds; j++)
                        {
                            if(point_on_bond2d1(crx->pt, byx[j]))
                            {
                                if(crx->crossing_direction ==  BELOW_TO_ABOVE)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if( crx->crossing_direction == ABOVE_TO_BELOW)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == LEFT_TO_RIGHT)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == RIGHT_TO_LEFT)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        /*  the top  */
        iy = smax[1];
        for (coyx = coy[smax[1]-1],
                 ix = smin[0]; ix < smax[0]; ix++)
        {
             ip[1] = smax[1]; ip[0] = ix; c = coyx[ix];
             if(c == ONFRONT)
             {
                    byx = T->bonds[iy-1][ix];
                    cyx = T->curves[iy-1][ix];
                    num_of_bonds = T->num_of_bonds[iy-1][ix];
                    /* first EAST  */
                    ipn[0] = ip[0]+1;  ipn[1] = ip[1];
                    k = seg_index2d(ip[0],ip[1],EAST,smax);
                    nc = seg_crx_count[k];
                    for (i = 0; i < nc; i++)
                    {
                        list = seg_crx_lists[k][i];
                        crx = &(crx_store[list]);
                        for(j = 0; j <num_of_bonds; j++)
                        {
                            if(point_on_bond2d1(crx->pt, byx[j]))
                            {
                                if(crx->crossing_direction ==  BELOW_TO_ABOVE)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if( crx->crossing_direction == ABOVE_TO_BELOW)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == LEFT_TO_RIGHT)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == RIGHT_TO_LEFT)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                            }
                        }
                    }
             }
        }

        /* The right   */
        ix = smax[0];
        for (iy = smin[1]; iy < smax[1]; iy++)
        {
            coyx = coy[iy];
            ip[1] = iy; ip[0] = ix; c = coyx[ix-1];
            if(c == ONFRONT)
            {
                    byx = T->bonds[iy][ix-1];
                    cyx = T->curves[iy][ix-1];
                    num_of_bonds = T->num_of_bonds[iy][ix-1];
                    /* NORTH  */
                    ipn[0] = ip[0]+1;  ipn[1] = ip[1];
                    k = seg_index2d(ip[0],ip[1],NORTH,smax);
                    nc = seg_crx_count[k];
                    for (i = 0; i < nc; i++)
                    {
                        list = seg_crx_lists[k][i];
                        crx = &(crx_store[list]);
                        for(j = 0; j <num_of_bonds; j++)
                        {
                            if(point_on_bond2d1(crx->pt, byx[j]))
                            {
                                if(crx->crossing_direction ==  BELOW_TO_ABOVE)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if( crx->crossing_direction == ABOVE_TO_BELOW)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == LEFT_TO_RIGHT)
                                {
                                    crx->lcomp =  positive_component(cyx[j]);
                                    crx->ucomp =  negative_component(cyx[j]);
                                    break;
                                }
                                if(crx->crossing_direction == RIGHT_TO_LEFT)
                                {
                                    crx->ucomp =  positive_component(cyx[j]);
                                    crx->lcomp =  negative_component(cyx[j]);
                                    break;
                                }
                            }
                        }
                    }
            }
        }

        DEBUG_LEAVE(set_comp_at_crossings2d1)
}	/* end set_comp_at_crossings2d1 */

LOCAL   int     point_on_bond2d1(
        FT_POINT            *pt,
        BOND             *b)
{
        if(b->start == pt)    return YES;
        if(b->end == pt)      return YES;
        return NO;
}	/* end point_on_bond2d1 */


LOCAL	int	find_index_of_node(
	NODE		*node,
	INTERFACE	*intfc)
{
	NODE		**n;
	int		n_index = 0;
	
	for ( n = intfc->nodes; n && (*n != node); ++n)
	    ++n_index;

	return n_index;
}		/*end find_index_of_node*/


/*	Notes: GL and GU are the actual grid domain plus the buffer zone 
 *	while VL and VU is the size after extra one block extension. so 
 *	shift_... function should use GL and GU for boundary curve/surface
 */

LOCAL   void set_aug_comp_grid(
        RECT_GRID       *aug_comp_grid,
        RECT_GRID       *comp_grid,
	int		aug_size)
{
        int       gmax[MAXD];
        int       i, dim;
        double     h[MAXD], VL[MAXD], VU[MAXD];
        double     GL[MAXD], GU[MAXD];
        
	dim = comp_grid->dim;
        for (i = 0; i < dim; i++)
        {
            gmax[i] = comp_grid->gmax[i] + comp_grid->lbuf[i]
                    + comp_grid->ubuf[i] + 2*aug_size;
            h[i] = comp_grid->h[i];
            VL[i] = comp_grid->VL[i];
            VU[i] = comp_grid->VU[i];
            GL[i] = comp_grid->VL[i];
            GU[i] = comp_grid->VU[i];
            VU[i] += aug_size*h[i];
            VL[i] -= aug_size*h[i];
        }
        set_rect_grid(VL, VU, GL, GU, NOBUF, NOBUF, gmax, dim,
                      &comp_grid->Remap, aug_comp_grid);
}


LOCAL  void set_interface_topology_on_grid(
        INTERFACE  *intfc,
        RECT_GRID  *expanded_grid)
{
        RECT_GRID *top_grid = &topological_grid(intfc);

        DEBUG_ENTER(set_interface_topology_on_grid)
        if ((intfc->modified) || no_topology_lists(intfc) ||
                (memcmp((const void*)expanded_grid,
                        (const void*)top_grid,sizeof(RECT_GRID)) != 0))
        {
            set_topological_grid(intfc,expanded_grid);
            no_topology_lists(intfc) = NO;
		
	    if (!make_interface_topology_lists(intfc))
            {
                screen("ERROR in set_interface_topology_on_grid(), "
                       "make_interface_topology_lists() failed\n");
                clean_up(ERROR);
            }
        }
        DEBUG_LEAVE(set_interface_topology_on_grid)
}               /*end set_interface_topology_on_grid */
