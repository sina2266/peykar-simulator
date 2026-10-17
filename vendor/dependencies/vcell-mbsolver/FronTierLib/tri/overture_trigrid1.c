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
 *                               overture_trigrid1.c:
 *
 *       Copyright 1999 by The University at Stony Brook, All rights reserved.
 *
 *       A triangulation of the computational region is constructed
 *       from a rectangular grid and an interface.  The corners of
 *       the triangles and quadrangles in the triangulation are either
 *       intersections between grid lines of the rectangular grid or
 *       intersections of grid lines with the interface.
 *
 *       TODO:
 *               1. Rig up a mechanism for triangulating only selected
 *                       components.
 *               2. Speed up area computations.
 */

#if defined(DEBUG_TRI_GRID)
#define DEBUG_STRING    "tri_grid"
#endif /* defined(DEBUG_TRI_GRID) */

#include <tri/trilocaldecs.h>

#if defined(USE_OVERTURE)

EXPORT  void use_overture_init_triangulation_storage(
        TRI_GRID        *ntg,
        int             **ic_of_node,
        size_t          sizest)
{
        RECT_GRID       *expanded_dual_grid = &ntg->rect_grid;
        int             j,n_reg_nodes;
        int             i,dim = expanded_dual_grid->dim;
        int             *gmax = expanded_dual_grid->gmax;
        register Locstate *state;
        register byte     *storage;
        DEBUG_ENTER(use_overture_init_triangulation_storage)

        print_storage("before init_triangulation_storage","TRI_storage");
#if defined(DEBUG_TRI_GRID)
        if (debugging("TRI_storage"))
        {
            (void) printf("\n\tBefore set triangulation storage\n");
            print_trigrid_alloc_status("",ntg);
        }
#endif /* defined(DEBUG_TRI_GRID) */

        n_reg_nodes = gmax[0] + 1;
        for (i = 1; i < dim; ++i)
            n_reg_nodes *= (gmax[i] + 1);
        ntg->n_node_points = ntg->n_reg_nodes = n_reg_nodes;

        switch (dim)
        {
#if defined(ONED)
        case 1:
            set_crx_structure_storage1d(ntg);
            break;
#endif /* defined(ONED) */
#if defined(TWOD)
        case 2:
            set_crx_structure_storage2d(ntg,ic_of_node);
            break;
#endif /* defined(TWOD) */
#if defined(THREED)
        case 3:
            set_crx_structure_storage3d(ntg);
            break;
#endif /* defined(THREED) */
        }

                        /* allocate comp/state storage */
        alloc_components_array(ntg,n_reg_nodes);
        alloc_states_array(ntg,n_reg_nodes);

        CompositeGrid   *cg1;
        int             num_float;
        doubleCompositeGridFunction *u_over;
        Index           I1,I2,I3;
        int             grid_over;
        Range           all;
        int base1,base2,base3,bound1,bound2,bound3;

        if (not ntg->use_overture_state)
        {
            VECTOR(ntg,rect_state_storage,n_reg_nodes,sizest);
            storage = ntg->rect_state_storage;
        }
        else
        {
            grid_over = ntg->patch_number;
            if(grid_over == 0 && ntg->overture_init_step)
            {
                num_float = sizest/sizeof(double)+1;
                cg1 = (CompositeGrid*)ntg->cg_over;
                u_over = new doubleCompositeGridFunction();
                u_over->updateToMatchGrid((*cg1),num_float,all, all, all);
                ntg->cg_over_function = (POINTER)u_over;
            }
            else
            {
                cg1 = (CompositeGrid*)ntg->cg_over;
                u_over = (doubleCompositeGridFunction*)ntg->cg_over_function;
            }
            getIndex((*cg1)[grid_over].indexRange(),I1,I2,I3);
            base1 = I1.getBase();
            base2 = I2.getBase();
            base3 = I3.getBase();
            bound1 = I1.getBound();
            bound2 = I2.getBound();
            bound3 = I3.getBound();
        }

                    /* storage for regular points */

        state = ntg->states;
        j = 0;
        switch (dim)
        {
#if defined(ONED)
        case 1:
        {
            register double      *xx_grid = expanded_dual_grid->edges[0];
            int                 xmax = gmax[0];
            int                 ix;

            for (ix = 0;  ix <= xmax;  ++ix)
            {
                Coords(ntg->node_points+j)[0] = xx_grid[ix];
                ++j;

                *state++ = (Locstate) storage;
                storage += sizest;
            }
        }
                break;
#endif /* defined(ONED) */
#if defined(TWOD)
        case 2:
        {
            register double y;
            register double *xx_grid = expanded_dual_grid->edges[0];
            register double *yy_grid = expanded_dual_grid->edges[1];
            int            xmax = gmax[0],ymax = gmax[1];
            int            ix,iy;

            if(ntg->use_overture_state)
            {
                int                 BXL,BXU,BYL,BYU;
                int                 ii = 0; 

                BXL = ntg->tg_grid.lbuf[0];
                BYL = ntg->tg_grid.lbuf[1];
                BXU = ntg->tg_grid.ubuf[0];
                BYU = ntg->tg_grid.ubuf[1];
                
                for (iy = 0;  iy <= ymax;  iy++)
                {
                    y = yy_grid[iy];

                    for (ix = 0;  ix <= xmax;  ix++)
                    {
                        Coords(ntg->node_points+j)[0] = xx_grid[ix];
                        Coords(ntg->node_points+j)[1] = y;

                        j++;
                        if ( grid_over == 0)
                        {
                            state[ii++] = (Locstate)
                                (&(*(u_over))[grid_over](0,ix-BXL-1,iy-BYL-1,0));
                        }
                        else
                        {
                            state[ii++] = (Locstate)
                                (&(*(u_over))[grid_over](0,ix+base1-BXL-1,iy+base2-BYL-1,0));
                        }
                    }
                }
            } 
            else
            {
                for (iy = 0;  iy <= ymax;  ++iy)
                {
                    y = yy_grid[iy];
                    for (ix = 0;  ix <= xmax;  ++ix)
                    {
                        Coords(ntg->node_points+j)[0] = xx_grid[ix];
                        Coords(ntg->node_points+j)[1] = y;
                        ++j;

                        *state++ = (Locstate) storage;
                        storage += sizest;
                    }
                }
            } 
        }
            break;
#endif /* defined(TWOD) */
#if defined(THREED)
        case 3:
        {
            register double y,z;
            register double *xx_grid = expanded_dual_grid->edges[0];
            register double *yy_grid = expanded_dual_grid->edges[1];
            register double *zz_grid = expanded_dual_grid->edges[2];
            int            xmax = gmax[0],ymax = gmax[1],zmax = gmax[2];
            int            ix,iy,iz;


            for (iz = 0;  iz <= zmax;  ++iz)
            {
                z = zz_grid[iz];
                for (iy = 0;  iy <= ymax;  ++iy)
                {
                    y = yy_grid[iy];
                    for (ix = 0;  ix <= xmax;  ++ix)
                    {
                        Coords(ntg->node_points+j)[0] = xx_grid[ix];
                        Coords(ntg->node_points+j)[1] = y;
                        Coords(ntg->node_points+j)[2] = z;
                        ++j;

                        *state++ = (Locstate) storage;
                        storage += sizest;
                    }
                }
            }
        }
            break;
#endif /* defined(THREED) */
        }
        print_storage("after init_triangulation_storage","TRI_storage");
        DEBUG_LEAVE(use_overture_init_triangulation_storage)
}               /*end use_overture_init_triangulation_storage*/

EXPORT  void set_AMR_triangulation_storage(
        TRI_GRID        *ntg,
        size_t          sizest)
{
        register RECT_GRID      *r_gr = &ntg->rect_grid;
        register Locstate       *state;
        register byte           *storage;

        int             n_reg_nodes;
        int             i, dim = r_gr->dim;
        int             *gmax = r_gr->gmax;
#if defined(TWOD) || defined(THREED)
        BLK_EL0         *blk_el0;
        static BLK_EL0  CLEAR_BLK_EL0;
#endif /* defined(TWOD) || defined(THREED) */
#if defined(TWOD)
        BLK_EL1         *blk_el1;
        static BLK_EL1  CLEAR_BLK_EL1;
#endif /* defined(TWOD) */

        CompositeGrid   *cg1;
        int             num_float;
        doubleCompositeGridFunction *u_over;
        Index           I1,I2,I3;
        int             grid_over;
        Range           all;
        int             base1,base2,base3,bound1,bound2,bound3;

        print_storage("before set_AMR_triangulation_storage","TRI_storage");
#if defined(DEBUG_TRI_GRID)
        if (debugging("TRI_storage"))
        {
            (void) printf("\n\tBefore set AMR triangulation storage\n");
            print_trigrid_alloc_status("",ntg);
        }
#endif /* defined(DEBUG_TRI_GRID) */

        n_reg_nodes = gmax[0] + 1;
        for (i = 1; i < dim; i++) n_reg_nodes *= (gmax[i] + 1);
#if defined(DEBUG_TRI_GRID)
        if (debugging("TRI_storage"))
        {
            print_int_vector("gmax = ",gmax,dim,"\n");
            (void) printf("n_reg_nodes = %d\n",n_reg_nodes);
        }
#endif /* defined(DEBUG_TRI_GRID) */

                        /* allocate comp/state storage */

        alloc_states_array(ntg,n_reg_nodes);

        if (not ntg->use_overture_state)
        {
            VECTOR(ntg,rect_state_storage,n_reg_nodes,sizest);
        }
        else 
        {
            grid_over = ntg->patch_number;
            cg1 = (CompositeGrid*)ntg->cg_over;
            u_over = (doubleCompositeGridFunction*)ntg->cg_over_function;
            getIndex((*cg1)[grid_over].indexRange(),I1,I2,I3);
            base1 = I1.getBase();
            base2 = I2.getBase();
            base3 = I3.getBase();
            bound1 = I1.getBound();
            bound2 = I2.getBound();
            bound3 = I3.getBound();
        }  

        switch (dim)
        {
#if defined(ONED)
        case 1:
        {
                int xmax = gmax[0];
                int ix;

                    /* storage for regular points */
                /* associate storage with states at regular grid nodes */

                state = ntg->states;
                storage = ntg->rect_state_storage;
                for (ix = 0;  ix <= xmax;  ix++)
                {
                        *state++ = (Locstate) storage;
                        storage += sizest;
                }
        }
                break;
#endif /* defined(ONED) */
#if defined(TWOD)
        case 2:
        {
            int xmax = gmax[0], ymax = gmax[1];
            int ii, ix, iy;
            int BXL,BXU,BYL,BYU;

            ii = 0;
            BXL = ntg->tg_grid.lbuf[0];
            BYL = ntg->tg_grid.lbuf[1];
            BXU = ntg->tg_grid.ubuf[0];
            BYU = ntg->tg_grid.ubuf[1];
            if (debugging("set_AMR_triangulation_storage"))
            {
                printf("In set_AMR_triangulation_storage use_overture = %d\n",
                                    ntg->use_overture_state);
                printf("overture_init_step = %d\n", ntg->overture_init_step);
                printf("Do patch[%d], cg_over_function = %d\n",
                                grid_over, u_over);
                printf("base1=%d  bound1=%d  BXL=%d  BXU=%d\n",
                                base1,bound1,BXL,BXU);
                printf("base2=%d  bound2=%d  BYL=%d  BYU=%d\n",
                                base2,bound2,BYL,BYU);
                printf("number of grids of expanded dual[x,y]<%d,%d>\n",
                                xmax, ymax);
            }

            VECTOR(ntg,lin_els,ntg->n_lin_els,sizeof(LINEAR_ELEMENT));
            if (interpolation_method(ntg) == ELEMENT_ON_THE_FLY)
                VECTOR(ntg,pcs,ntg->n_pcs,sizeof(POINT_COMP_ST));
            alloc_blk_els0(ntg,ymax*xmax);
            alloc_blk_els1(ntg,ymax*xmax);
#if defined(DEBUG_TRI_GRID)
            if (debugging("TRI_storage"))
            {
                (void) printf("n_crx %d, ",ntg->n_crx);
                (void) printf("storage set for %d triangles\n",ntg->n_lin_els);
            }
#endif /* defined(DEBUG_TRI_GRID) */
                    /* storage for regular points */
        /* associate storage with states at regular grid nodes */

            state = ntg->states;

            if (not ntg->use_overture_state)
                storage = ntg->rect_state_storage;

            blk_el0 = ntg->blk_els0;    blk_el1 = ntg->blk_els1;
            for (iy = 0;  iy <= ymax;  iy++)
            {
                for (ix = 0;  ix <= xmax;  ix++)
                {
                    if (ntg->use_overture_state)
                    {
                        if (grid_over == 0)
                        {
                            state[ii++] = (Locstate)
                               (&(*(u_over))[grid_over](0,ix-BXL-1,iy-BYL-1,0));
                        }
                        else
                        {
                            state[ii++] = (Locstate)
                              (&(*(u_over))[grid_over](0,ix+base1-BXL-1,iy+base2-BYL-1,0));
                        }
                    }
                    else
                    {
                        *state++ = (Locstate) storage;
                        storage += sizest;
                    }
                    if ((ix == xmax) or (iy == ymax))
                        continue;
                    *blk_el1++ = CLEAR_BLK_EL1;
                    *blk_el0++ = CLEAR_BLK_EL0;
                }
            }
        }
            break;
#endif /* defined(TWOD) */
#if defined(THREED)
        case 3:
        {
                int xmax = gmax[0], ymax = gmax[1], zmax = gmax[2];
                int ix, iy, iz;

                if (interpolation_method(ntg) == COMPLETE_TRIANGULATION)
                    VECTOR(ntg,lin_els,ntg->n_lin_els,sizeof(LINEAR_ELEMENT));
                if (interpolation_method(ntg) == ELEMENT_ON_THE_FLY)
                    VECTOR(ntg,pcs,ntg->n_pcs,sizeof(POINT_COMP_ST));
                alloc_blk_els0(ntg,zmax*ymax*xmax);
#if defined(DEBUG_TRI_GRID)
                if (debugging("TRI_storage"))
                {
                        (void) printf("n_crx %d, ",ntg->n_crx);
                        (void) printf("storage set for %d tetras\n",
                                      ntg->n_lin_els);
                }
#endif /* defined(DEBUG_TRI_GRID) */
                    /* storage for regular points */
                /* associate storage with states at regular grid nodes */

                state = ntg->states;
                storage = ntg->rect_state_storage;
                blk_el0 = ntg->blk_els0;
                for (iz = 0;  iz <= zmax;  iz++)
                {
                        for (iy = 0;  iy <= ymax;  iy++)
                        {
                                for (ix = 0;  ix <= xmax;  ix++)
                                {
                                        *state++ = (Locstate) storage;
                                        storage += sizest;
                                        if ((ix == xmax) or (iy == ymax)
                                                         or (iz == zmax))
                                                continue;
                                        *blk_el0++ = CLEAR_BLK_EL0;
                                }
                        }
                }
        }
                break;
#endif /* defined(THREED) */
        }
        print_storage("after set_AMR_triangulation_storage","TRI_storage");
}               /*end set_AMR_triangulation_storage*/

EXPORT  void    copy_AMR_tri_storage2d(
        TRI_GRID        *ogrid,
        TRI_GRID        *ngrid,
        size_t          sizest)
{
        BLK_EL0         *nblk,     *oblk;
        LINEAR_ELEMENT  *nlin,     *olin;
        byte            *nstorage, *ostorage, *end_ostr;
        int             *gmax = ogrid->rect_grid.gmax;
        int             xmax = gmax[0], ymax = gmax[1];
        int             j, ix, iy;
        int             i, dim = ogrid->rect_grid.dim;
        int             use_overture_state = ogrid->use_overture_state;

        if ( not use_overture_state )
        {
            nstorage = ngrid->rect_state_storage;
            ostorage = ogrid->rect_state_storage;
            end_ostr = ogrid->rect_state_storage + (xmax+1)*(ymax+1)*sizest;
        }
        if(use_overture_state)
        {
            int status;

            if(interpolation_method(ogrid) != ELEMENT_ON_THE_FLY)
            {
                printf("ERROR in copy_AMR_tri_storage2d()\n");
                printf("other triangulation method not implement\n");
                clean_up(ERROR);
            }
            start_clock("triangulate_mesh");
            status = triangulate_mesh(ngrid);
            stop_clock("triangulate_mesh");

            if (status != GOOD_STEP)
            {
                printf("ERROR in copy_AMR_tri_storage2d()\n");
                printf("retriangulate mesh failed\n");
                clean_up(ERROR);
            }
            return;
        }

        if (interpolation_method(ogrid) == COMPLETE_TRIANGULATION)
        {
            for (j = 0, olin = ogrid->lin_els, nlin = ngrid->lin_els;
                                j < ogrid->n_lin_els;  j++, olin++, nlin++)
            {
                nlin->comp   = olin->comp;
                for (i = 0; i < dim+1; i++)
                {
                    nlin->side[i] = olin->side[i];
                    nlin->p[i] = olin->p[i];

                    if ( use_overture_state )
                    {
                        nlin->s[i] = olin->s[i];
                    }
                    else
                    {
                        if ((olin->s[i] >= ((Locstate) ostorage)) and
                            (olin->s[i] < ((Locstate) end_ostr)))
                        {
                            nlin->s[i] = (Locstate)(nstorage +
                                        (((byte *) olin->s[i]) - ostorage));
                        }
                        else
                        {
                            nlin->s[i] = olin->s[i];
                        }
                    }
                }
            }

            nlin   = ngrid->lin_els;    olin   = ogrid->lin_els;

            nblk = ngrid->blk_els0;
            oblk = ogrid->blk_els0;
            for (iy = 0;  iy < ymax;  iy++)
            {
                for (ix = 0;  ix < xmax;  ix++, nblk++, oblk++)
                {
                    if (blk_el0_is_bilinear(oblk))
                    {
                        set_bilinear_blk_el0(nblk);
                        blk_el0_bilinear_el(nblk) = blk_el0_bilinear_el(oblk);
                    }
                    else
                    {
                        num_lin_els_in_blk(nblk) = num_lin_els_in_blk(oblk);
                        blk_el0_linear_els(nblk) = nlin
                                        + (blk_el0_linear_els(oblk) - olin);
                    }
                }
            }
        }
        if (interpolation_method(ogrid) == ELEMENT_ON_THE_FLY)
        {
            POINT_COMP_ST    *npcs, *opcs;
            for (j = 0, opcs = ogrid->pcs, npcs = ngrid->pcs;
                                j < ogrid->n_pcs;  j++, opcs++, npcs++)
            {
                npcs->p = opcs->p;
                for (i = 0; i < 2; i++)
                {
                    npcs->comp[i] = opcs->comp[i];

                    if ( use_overture_state )
                    {
                        npcs->s[i] = opcs->s[i];
                    }
                    else
                    {
                        if ((opcs->s[i] >= ((Locstate) ostorage)) and
                            (opcs->s[i] < ((Locstate) end_ostr)))
                        {
                            npcs->s[i] = (Locstate)(nstorage +
                                (((byte *) opcs->s[i]) - ostorage));
                        }
                        else
                        {
                            npcs->s[i] = opcs->s[i];
                        }
                    }
                }
            }
            npcs   = ngrid->pcs; opcs   = ogrid->pcs;
            nblk = ngrid->blk_els0;
            oblk = ogrid->blk_els0;
            for (iy = 0; iy < ymax; iy++)
            for (ix = 0; ix < xmax; ix++, nblk++, oblk++)
            {
                num_lin_els_in_blk(nblk) = num_lin_els_in_blk(oblk);
                blk_el0_pcs_els(nblk) = npcs + (blk_el0_pcs_els(oblk) - opcs);
            }
        }

#if defined(DEBUG_TRI_GRID)
        if (debugging("print_copy_tri"))
                debug_copy_2d_tri_grid(ogrid,ngrid,sizest);
#endif /* defined(DEBUG_TRI_GRID) */
}               /*end copy_AMR_tri_storage2d*/
         
#endif /* if defined(USE_OVERTURE) */  
