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
*                               foverture_patch.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*
*       this file includes functions set_patch_front, set_patch_topo_grid,
*                       and set_patch_comput_grid
*/

#define DEBUG_STRING    "foverture_patch"

#include <front/fdecs.h>

#if defined(USE_OVERTURE)


/**************************************************/  
/* NOTE: definition BORROWED FROM front/fscat2d.c */ 
enum {
        CUT_NODE      = 0x0040,
        X_CUT_NODE    = 0x0080,
        CLIP_NODE     = 0x0100,
        LOCAL_NODE    = 0x0200,
        ADJ_BOND_NODE = 0x0400,
        NODE_MASK     = (CUT_NODE|X_CUT_NODE|CLIP_NODE|LOCAL_NODE|ADJ_BOND_NODE)
};
#define is_cut_node(n)                  (Boundary(n) & CUT_NODE)
#define is_x_cut_node(n)                (Boundary(n) & X_CUT_NODE)
#define is_clip_node(n)                 (Boundary(n) & CLIP_NODE)
#define is_local_node(n)                (Boundary(n) & LOCAL_NODE)
#define is_adj_bond_node(n)             (Boundary(n) & ADJ_BOND_NODE)

#define set_adj_bond_cross_node(n)      (Boundary(n) |= ADJ_BOND_NODE)
#define set_imported_node(n)            (Boundary(n) &= ~LOCAL_NODE)
#define clear_node_flags(n)             (Boundary(n) &= ~NODE_MASK)

LOCAL  void      print_node_flags(NODE*); 
/*********** End of NOTE  ***********/ 

LOCAL  int       use_delete_short_bonds = YES;  

LOCAL  boolean      pseudo_scatter_patch_fronts(Front**,Front**,
                      Front*,int,int,Wv_on_pc**,int,int);
LOCAL  int       set_copy_proc_frs_ver2(Front***,Front**,int,
                      Wv_on_pc**,int,int*);
LOCAL  void      clear_copy_frs_intfc(Front**,Wv_on_pc**,int,int);
LOCAL  boolean	 form_patch_subintfc_via_cut(Front*);
LOCAL  boolean 	 set_amr_subdomain_boundary(Front*,COMPONENT);  
LOCAL  boolean      amr_check_for_cut_nodes(const char*,INTERFACE*);
LOCAL  int       merge_fronts(Front*,Front**,int); 
LOCAL  void      remove_dup_nodes_from_assembly_intfc(INTERFACE*); 
LOCAL  void      amr_clip_to_interior_region(INTERFACE*,int*,int*,int); 
LOCAL  int       distribute_fronts_base_to_patch(Front**,int,
                     Wv_on_pc**,int);
LOCAL  int       merge_fronts_ver2(Front*,Front**,int); 
LOCAL  void      delete_sub_bdry_and_interior_curves(INTERFACE*); 
LOCAL  Front     *front_on_redistr_table(Wv_on_pc**,int,int,int,int);

LOCAL void print_node_flags(
        NODE            *n)
{
        (void) printf("Node %3llu boundary %5d  ",node_number(n),Boundary(n));
        if (is_bdry(n))
            (void) printf("BDRY ");
        else
            (void) printf("     ");
        if (is_cut_node(n))
        {
            (void) printf("CUT ");

            if (is_x_cut_node(n))
                (void) printf("X_CUT ");
            else
                (void) printf("Y_CUT ");
        }
        else
            (void) printf("    ");
        if (is_clip_node(n))
            (void) printf("CLIP ");
        else
            (void) printf("     ");
        if (is_local_node(n))
            (void) printf("LOCAL ");
        else
            (void) printf("      ");
        if (is_adj_bond_node(n))
            (void) printf("ADJ_BOND ");
        else
            (void) printf("         ");
        (void) printf("\n");
}               /*end print_boundary_flags*/

/* front_on_redistr_table()
 * find out the front hooked on the 
 * redistribute_table item specified by
 * node and patch_number.
 * in_node is used to speed up the searching.
 * if in_node is set, only this node is searched.
 */
LOCAL Front *front_on_redistr_table(
	Wv_on_pc        **redistr_table,
        int             max_n_patch,
        int             node,
        int             patch_number,
        int             in_node)
{
        int             numnodes, source;
        int             i;

        numnodes = pp_numnodes();

        if(in_node != -1)
            source = in_node; 
        else
            source = 0; 
        for(; source < numnodes; source++)
        {
            for(i = 0; i < max_n_patch; i++)
            {
                if(redistr_table[source][i].pc_id == node &&
                   redistr_table[source][i].wv_id == patch_number)
                {
                    if(redistr_table[source][i].front == NULL)
                    {
                        printf("ERROR in front_on_redistr_table()\n");
                        printf("node[%d] patch_num[%d] does not"
                          " have front on redistr_table[%d][%d]\n",
                           node,patch_number,source,i); 
                        clean_up(ERROR);
                    }
                    return redistr_table[source][i].front;
                }
            }

            if(in_node != -1) break; 
        } 

        printf("ERROR in_node[%d] in front_on_redistr_table()\n", in_node);
        printf("node[%d] patch_num[%d] does not"
               " have front on redistr_table\n",
                node,patch_number); 
        clean_up(ERROR); 
        return NULL; 
}

EXPORT void newfront_to_distri_table(
        Front           *fr,
        Front           *nfr,
        int             num_patches,
        Wv_on_pc        **redistr_table,
        int             max_n_patch)
{
        int             numnodes, source;
        int             i;
        int             myid;
        int             found = NO;

        numnodes = pp_numnodes();
        myid = pp_mynode();
        for(source = 0; source < numnodes; source++)
        {
            for(i = 0; i < max_n_patch; i++)
            {
                if(redistr_table[source][i].front == fr)
                {
                    redistr_table[source][i].front = nfr;
                    found = YES;
                    break;
                }
            }
            if(found)
                break;
        }

        if(NO == found)
        {
            for(source = 0; source < numnodes; source++)
            {
                printf("Proc[%d]: ", source);
                for(i = 0; i < max_n_patch; i++)
                {
                    printf(" %p ", redistr_table[source][i].front);
                }
                printf("\n");
            }
            printf("ERROR: newfront_to_distri_table()\n");
            printf("newfr is not instored into redistr_table\n");
            printf("fr = %p, nfr = %p\n", fr, nfr); 
            clean_up(ERROR);
        }
}

EXPORT void set_patch_front(
        Front           *mother_front,
        Front           *patch_front,
        RECT_GRID       *newrgr,
        int             patch_num)
{
        Front                   *front;
        INTERFACE               *sav_intfc;
        int                     i, dir, dim, *lbuf, *ubuf;
        boolean                    sav_copy; 
        RECT_GRID               *cgr;
        RECT_GRID               *tgr;
        Patch_bdry_flag         *pd_flag = patch_front->pd_flag;

        DEBUG_ENTER(set_patch_front)
        front = mother_front;

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        set_size_of_intfc_state(size_of_state(front->interf));
        set_copy_intfc_states(YES);
        patch_front->interf = copy_interface(front->interf);

        set_current_interface(patch_front->interf);
        /*  Do i need it ???   
        interpolate_intfc_states(patch_front->intfc) = YES 
        */  
        patch_front->interf->modified = YES;
        patch_front->rect_grid = newrgr;

        cgr = computational_grid(patch_front->interf);
        copy_rect_grid(cgr,newrgr);  
        /* 
        set_patch_comput_grid(newrgr,cgr);
        */ 
        dim = cgr->dim;
        lbuf = cgr->lbuf;
        ubuf = cgr->ubuf;

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        tgr = &topological_grid(patch_front->interf);
        tgr->Remap.remap = newrgr->Remap.remap;
        set_patch_topo_grid(newrgr,tgr);
        
        /* Useless ???? 
        if (make_interface_topology_lists(patch_front->interf) ==
                           FUNCTION_FAILED)
        {
            screen("ERROR in set_patch_front(), "
                  "make_interface_topology_lists() failed\n");
            clean_up(ERROR);
        }
        */ 
        if ( patch_num != 0 )
        {
            for ( dir = 0; dir < dim; dir++)
            {
                for ( i = 0; i < 2; i++)
                {
                    if (pd_flag->bdry_side[dir][i] == 0)
                        rect_boundary_type(patch_front->interf,dir,i)
                        = AMR_SUBDOMAIN_BOUNDARY;
                }
            }
        }

        DEBUG_LEAVE(set_patch_front)
}

EXPORT void set_patch_topo_grid(
        RECT_GRID       *pgr,
        RECT_GRID       *tgr)
{
        double           *L = pgr->L;
        double           *U = pgr->U;
        double           *GL = pgr->GL;
        double           *GU = pgr->GU;
        double           *h = pgr->h;
        int             *lbuf = pgr->lbuf;
        int             *ubuf = pgr->ubuf;
        int             gmax[MAXD];
        int             dim = pgr->dim;
        int             i, dir;
        RECT_GRID       dual_grid;
        RECT_GRID       expanded_dual_grid;

        DEBUG_ENTER(set_patch_topo_grid)

        set_dual_grid(&dual_grid, pgr);
        for (i = 0; i < dim; i++)
            gmax[i] = pgr->gmax[i]+pgr->lbuf[i]+pgr->ubuf[i];
        for (i = 0; i < dim; i++)
            gmax[i] = gmax[i]/2;

        expanded_dual_grid.Remap = pgr->Remap;  
        set_rect_grid(pgr->VL,pgr->VU,pgr->GL,pgr->GU,
                      NOBUF,NOBUF,gmax,dim,&expanded_dual_grid.Remap,
                      &expanded_dual_grid);
        for (i = 0; i < dim; i++)
        {
            tgr->gmax[i] = expanded_dual_grid.gmax[i];
            tgr->L[i] = expanded_dual_grid.L[i];
            tgr->U[i] = expanded_dual_grid.U[i];
        }
        set_rect_grid(tgr->L,tgr->U, expanded_dual_grid.GL,expanded_dual_grid.GU,
            NOBUF,NOBUF,tgr->gmax,dim,&tgr->Remap,tgr);

        DEBUG_LEAVE(set_patch_topo_grid)
}

EXPORT   void delete_patch_all_curves(
        Front           *fr)
{
        INTERFACE       *sav_intfc;
        INTERFACE       *intfc = fr->interf;
        NODE            **n;
        CURVE           **c;
        CURVE           **delete_curves = NULL;
        NODE            **delete_nodes = NULL;
        boolean            sav_copy;

        DEBUG_ENTER(delete_patch_all_curves)

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        /* remove subdomain bdry curves and physical boundary curves */
        for(c = intfc->curves; c and *c; c++)
        {
            if (not add_to_pointers((POINTER)*c,(POINTER**)&delete_curves))
            {
                screen("ERROR in delete_patch_all_curves(), "
                    "add_to_pointers() failed\n");
                clean_up(ERROR);
            }
        }
        for (c = delete_curves; c and *c; c++)
            (void) delete_curve(*c);


        delete_nodes = NULL;
        for (n = intfc->nodes; n and *n; n++)
        {
            if (((*n)->in_curves == NULL) and ((*n)->out_curves == NULL))
            {
                if (not add_to_pointers((POINTER)*n,(POINTER**)&delete_nodes))
                {
                    screen("ERROR in delete_patch_all_curves(), "
                              "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (n = delete_nodes; n and *n; n++)
            (void) delete_node(*n);


        if(debugging("delete_patch_all_curves"))
        {
            printf("In delete_remove_patch_all_curves\n");
            printf("Print left tmp interface:::\n");
            print_interface(intfc);
            show_intfc_states(intfc);
        }

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(delete_patch_all_curves)
}

/*  BE CAREFUL: ONLY subdomain curves are removed 
    032803
*/

EXPORT   void remove_patch_all_boundary_curves(
        Front           *fr)
{
        INTERFACE       *sav_intfc;
        INTERFACE       *intfc = fr->interf;
        NODE            **n;
        CURVE           **c;
        CURVE           **delete_curves = NULL;
        NODE            **delete_nodes = NULL;
        boolean            sav_copy;

        DEBUG_ENTER(remove_patch_all_boundary_curves)

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        /* remove subdomain bdry curves and physical boundary curves */
        delete_subdomain_curves(intfc);

        for(c = intfc->curves; c and *c; c++)
        {
            if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
            {
                if (not add_to_pointers((POINTER)*c,(POINTER**)&delete_curves))
                {
                    screen("ERROR in remove_patch_all_boundary_curves(), "
                        "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (c = delete_curves; c and *c; c++)
            (void) delete_curve(*c);


        delete_nodes = NULL;
        for (n = intfc->nodes; n and *n; n++)
        {
            if (((*n)->in_curves == NULL) and ((*n)->out_curves == NULL))
            {
                if (not add_to_pointers((POINTER)*n,(POINTER**)&delete_nodes))
                {
                    screen("ERROR in remove_patch_all_boundary_curves(), "
                              "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (n = delete_nodes; n and *n; n++)
            (void) delete_node(*n);

        if(debugging("remove_patch_all_boundary_curves"))
        {
            printf("In retrieve_boundary_curves_from_zero\n");
            printf("Print left tmp interface:::\n");
            print_interface(intfc);
            show_intfc_states(intfc);
        }

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(remove_patch_all_boundary_curves)
}

LOCAL   void delete_sub_bdry_and_interior_curves(
        INTERFACE       *intfc)
{
        INTERFACE       *sav_intfc;
        NODE            **n;
        CURVE           **c;
        CURVE           **delete_curves = NULL;
        NODE            **delete_nodes = NULL;
        boolean            sav_copy;

        DEBUG_ENTER(delete_sub_bdry_and_interior_curves)

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        /* remove subdomain bdry curves and physical boundary curves */
        delete_subdomain_curves(intfc);
        delete_passive_boundaries(intfc);

        for(c = intfc->curves; c && *c; c++)
        {
            if(wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE)
            {
                if (not add_to_pointers((POINTER)*c,(POINTER**)&delete_curves))
                {
                    screen("ERROR in delete_sub_bdry_and_interior_curves(), "
                        "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (c = delete_curves; c and *c; c++)
            (void) delete_curve(*c);


        delete_nodes = NULL;
        for (n = intfc->nodes; n and *n; n++)
        {
            if (((*n)->in_curves == NULL) && ((*n)->out_curves == NULL))
            {
                if (not add_to_pointers((POINTER)*n,(POINTER**)&delete_nodes))
                {
                    screen("ERROR in delete_sub_bdry_and_interior_curves(), "
                              "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (n = delete_nodes; n && *n; n++)
            (void) delete_node(*n);

        if(debugging("delete_sub_bdry_and_interior_curves"))
        {
            printf("In delete_sub_bdry_and_interior_curves\n");
            printf("Print left tmp interface:::\n");
            print_interface(intfc);
            show_intfc_states(intfc);
        }

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(delete_sub_bdry_and_interior_curves)
}

/* this function should be called after the assembling
 * process, it cuts a piece from global interface.
 * the input front contains a global interface.
 */

EXPORT boolean clip_patch_front(
        Front           *front,
        int             zoom_rect_grids)
{
        COMPONENT       max_comp;
        INTERFACE       *intfc = front->interf;
        RECT_GRID       *gr = front->rect_grid;
        boolean            status;
        boolean            sav_copy = copy_intfc_states();
        boolean            sav_intrp = interpolate_intfc_states(intfc);
        int             i, dim = gr->dim;

        DEBUG_ENTER(clip_patch_front)

        for (i = 0; i < dim; i++)
            if ((gr->lbuf[i] > 0) or (gr->ubuf[i] > 0)) break;
        if (i == dim)
        {
            status = FUNCTION_SUCCEEDED; /* No subdomains to process */
        }
        else
        {
            set_copy_intfc_states(YES);
            interpolate_intfc_states(intfc) = NO;
            status = form_patch_subintfc_via_cut(front);
            /*  
            max_comp = max_component(intfc);
            pp_global_imax(&max_comp,1L);
            max_component(intfc) = max_comp;
            */
            interpolate_intfc_states(intfc) = sav_intrp;
            set_copy_intfc_states(sav_copy);
        }
#if defined(TWOD)
        if(debugging("clip_patch_front"))
        {
            CURVE **c;
            BOND  *b;
            if(front->patch_number != 4)
                return status;
            for(c = front->interf->curves; c and *c;  c++)
            {
                if(is_bdry(*c)) continue;

                b = (*c)->first;
                while( b != NULL)
                {
                    printf("point <%g, %g> state\n",
                            Coords(b->start)[0], Coords(b->start)[1]);
                    (*front->print_state)(left_state(b->start));
                    if(b == (*c)->last)
                    {
                        printf("point <%g, %g> state\n",
                          Coords(b->end)[0], Coords(b->end)[1]);
                        (*front->print_state)(left_state(b->end));
                    }
                    b = b->next;
                }
            }
        }
#endif  /* defined(TWOD) */
/*
#if defined(THREED)
        if (debugging("consistency"))
        {
            (void) printf("Check consistency of interface ");
            (void) printf("after scatter_front()\n");
            check_consistency_of_tris_on_intfc(intfc);
        }
#endif */  /* defined(THREED) */

        DEBUG_LEAVE(clip_patch_front)
        return status;
}

/* NOTE: assembly_fine_patch_fronts_to_one()
 * only assembly the interfaces. The
 * boundary is not considered in this function.
 * To complete the boundary, 
 * set_amr_subdomain_boundary() or
 * set_subdomain_boundary() or should be called. 
 */
EXPORT void assembly_fine_patch_fronts_to_one(
        Front   **frs,
        Front   *front)
{
        if (front == NULL or front->interf == NULL)
        {
            printf("ERROR: assembly_fine_patch_fronts_to_one\n");
            printf("function failed because of front = %p\n", front);
            clean_up(ERROR);
        }
        (*f_user_interface(front->interf)._assembly_fine_patch_fronts_to_one)(frs,front);
        /* 
         *  Hooked to g_assembly_fine_patch_fronts_to_one().
         *  assembly_fine_patch_fronts_to_one2d_ver2().
         */ 
}

LOCAL  boolean form_patch_subintfc_via_cut(
        Front   *front)
{
        if (front == NULL or front->interf == NULL)
            return FUNCTION_FAILED;
        /* hooked with g_form_patch_subintfc_via_cut2d */
        return 
          (*f_user_interface(front->interf)._form_patch_subintfc_via_cut)(front);
        /* 
        return f_form_patch_subintfc_via_cut2d(front);  		
        */ 
}               /*end form_patch_subintfc_via_communication*/

/* This function is built based on the assumption that
 * the base patch interface is assembled from the fine
 * patch interface. AND the base interface performs the
 * real interface buffer communication. Then the
 * patch interface first copies the rebuilt base interface,
 * and the patch interface only needs to do the cut to
 * fit the interface to the domain.
 */

EXPORT boolean f_form_patch_subintfc_via_cut2d(
        Front           *fr)
{
        COMPONENT       i_comp = 0;
        INTERFACE       *intfc = fr->interf;
        INTERFACE       *sav_intfc;
        boolean            sav_copy;
        O_NODE          *onode_list;
        RECT_GRID       *gr = fr->rect_grid;
        double           coords[MAXD];
        boolean            status = FUNCTION_SUCCEEDED;
        int             i, dir, dim = gr->dim;

        DEBUG_ENTER(f_form_patch_subintfc_via_cut2d)

        /* Find an interior component on this processor's domain.  This
         * is needed to initialize the components on the subdomain
         * boundaries when no physical curves appear on this processor. */
        for (i = 0; i < dim; i++)
            coords[i] = grid_center_coord(i,gr);
        i_comp = long_component(coords,intfc);

        delete_subdomain_curves(intfc);
        delete_passive_boundaries(intfc);

        /* Cut off the patch interface to fit to the boundary*/

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        set_current_interface(fr->interf);

        for (dir = 0; dir < dim; dir++)
        {
            if (fr->rect_grid->lbuf[dir] > 0)
            {
                cut_interface(intfc,computational_grid(intfc)->VL[dir],
                      dir,1,NO,YES);
            }
            if (fr->rect_grid->ubuf[dir] > 0)
            {
                cut_interface(intfc,computational_grid(intfc)->VU[dir],
                            dir,0,NO,YES);
            }
        }

        if (debugging("f_form_patch_subintfc_via_cut2d")
            and fr->patch_number == 4)
        {
            printf("in f_form_patch_subintfc_via_cut2d()\n");
            printf("patch_[%d] after cut to virtual grid\n",
                    fr->patch_number);
            print_interface(fr->interf);
        }
        /* TODO:  a post-processing loop is needed here to shift the
         * subdomain nodes onto VL.  A problem can occur for periodic
         * boundaries on restart, where some accuracy is lost in VL or VU,
         * so that the subdomains have different sizes on each side of the
         * domain.  This will leave the curves hanging over the edge on the
         * shorter side, and these nodes will not be processed when
         * creating the subdomain boundary.
         * A better solution would be to guarantee the location of the
         * virtual boundaries, perhaps by printing them out as an integer
         * multiple of the mesh spacing instead of an absolute (double)
         * value. */

        status = set_amr_subdomain_boundary(fr,i_comp);
        if (status == FUNCTION_FAILED)
        {
            (void) printf("WARNING in f_form_patch_subintfc_via_cut2d(), "
                          "patch[%d] set_amr_subdomain_boundary() failed\n", 
                          fr->patch_number);
            if (DEBUG)
            {
                (void) printf("Offending interface: \n");
                print_interface(fr->interf);
            }
        }

        /* Zero length bonds can be produced, especially on an elliptic
         * interface.  If this happens AT a node, the component check gets
         * confused because it computes an angle for each  curve at a node
         * using only the node position and the adjacent point.  */

        /* Flag use_delete_short_bonds is set to be NO
         * in assembly_distribute_patch_fronts(),
         * after we glue interface, scatter the glued interface,
         * and clip_patch_front() for the fine grid patches.
         */ 
        if(use_delete_short_bonds == YES)
            intfc_delete_very_short_bonds(fr);
        else
            use_delete_short_bonds = YES;  

        /* The following code is intended to tell whether the scatter
         * succeeded by identifying problems/inconsistencies in the
         * new interface. */

        if (amr_check_for_cut_nodes("f_form_patch_subintfc_via_cut2d",intfc) == YES)
        {
            status = FUNCTION_FAILED;
            (void) printf("WARNING: in f_form_patch_subintfc_via_cut2d");
            (void) printf(" amr_check_for_cut_nodes() detected cut node.\n");
            clean_up(ERROR);  
        }
        else if (check_comps_at_nodes(fr->interf,&onode_list) != 0)
        {
            status = FUNCTION_FAILED;
            (void) printf("WARNING in f_form_patch_subintfc_via_cut2d");
            (void) printf(" check_comps_at_nodes() detected inconsistency\n");
            print_interface(fr->interf);
            if (DEBUG)
            {
                print_onode_list(&onode_list);
                (void) printf("Offending interface\n");
                print_interface(fr->interf);
            }
        }

        /*
        status = pp_min_status(status);
        */  
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(f_form_patch_subintfc_via_cut2d)
        return status;
}  

EXPORT boolean f_form_patch_subintfc_2d(
        Front           *fr,
        COMPONENT       i_comp)
{
        INTERFACE       *intfc = fr->interf;
        INTERFACE       *sav_intfc;
        boolean            sav_copy;
        O_NODE          *onode_list;
        boolean            status = FUNCTION_SUCCEEDED;

        DEBUG_ENTER(f_form_patch_subintfc_2d)

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        set_current_interface(fr->interf);

        if (debugging("f_form_patch_subintfc_2d"))
        {
            printf("in f_form_patch_subintfc_2d()\n");
            printf("patch [%d] after cut to virtual grid\n",
                    fr->patch_number);
            print_interface(fr->interf);
        }
        /* TODO:  a post-processing loop is needed here to shift the
         * subdomain nodes onto VL.  A problem can occur for periodic
         * boundaries on restart, where some accuracy is lost in VL or VU,
         * so that the subdomains have different sizes on each side of the
         * domain.  This will leave the curves hanging over the edge on the
         * shorter side, and these nodes will not be processed when
         * creating the subdomain boundary.
         * A better solution would be to guarantee the location of the
         * virtual boundaries, perhaps by printing them out as an integer
         * multiple of the mesh spacing instead of an absolute (double)
         * value. */

        status = set_amr_subdomain_boundary(fr,i_comp);
        if (status == FUNCTION_FAILED)
        {
            (void) printf("WARNING in f_form_patch_subintfc_2d(), "
                          "patch[%d] set_amr_subdomain_boundary() failed\n", 
                          fr->patch_number);
            if (DEBUG)
            {
                (void) printf("Offending interface: \n");
                print_interface(fr->interf);
            }
        }

        /* Zero length bonds can be produced, especially on an elliptic
         * interface.  If this happens AT a node, the component check gets
         * confused because it computes an angle for each  curve at a node
         * using only the node position and the adjacent point.  */
        
        if(use_delete_short_bonds == YES)
            intfc_delete_very_short_bonds(fr);
        else
            use_delete_short_bonds = NO;  

        /* The following code is intended to tell whether the scatter
         * succeeded by identifying problems/inconsistencies in the
         * new interface. */

        if (amr_check_for_cut_nodes("f_form_patch_subintfc_2d",intfc) == YES)
        {
            status = FUNCTION_FAILED;
            (void) printf("\n WARNING in f_form_patch_subintfc_2d");
            (void) printf(" amr_check_for_cut_nodes() detected cut node\n");
            (void) printf(" for patch front[%d] at level[%d]\n", 
                      fr->patch_number, fr->patch_level);
            print_rectangular_grid(fr->rect_grid);  
            print_interface(intfc);  
            clean_up(ERROR);  
        }
        else if (check_comps_at_nodes(fr->interf,&onode_list) != 0)
        {
            status = FUNCTION_FAILED;
            (void) printf("WARNING in f_form_patch_subintfc_via_cut2d");
            (void) printf(" check_comps_at_nodes() detected inconsistency\n");
            print_interface(fr->interf);
            if (DEBUG)
            {
                print_onode_list(&onode_list);
                (void) printf("Offending interface\n");
                print_interface(fr->interf);
            }
        }

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(f_form_patch_subintfc_2d)
        return status;
}  

/*
*                       set_amr_subdomain_boundary():
*
*/

LOCAL   boolean set_amr_subdomain_boundary(
        Front           *fr,
        COMPONENT       i_comp)
{
        INTERFACE       *intfc = fr->interf;
        CURVE           **c;
        NODE            **n;
        RECT_GRID       *gr = computational_grid(intfc);
        double           tol[MAXD];
        int             i, j, dim = gr->dim;
        ORIENTATION     orient;
        double           grid_tol;

        DEBUG_ENTER(set_amr_subdomain_boundary)

        /*
        grid_tol = grid_tolerance(gr)*pow(2.0,-(fr->NumberOfLevels-1.0-fr->patch_level));
	*/
	grid_tol = 100.0*MACH_EPS;

        /* This can fail if a tangle occurs across the virtual boundary. */

        if (set_boundary(intfc,gr,i_comp,grid_tol) == NO)
        {
            (void) printf("WARNING in set_amr_subdomain_boundary(), "
                          "set_boundary() failed\n");
            return FUNCTION_FAILED;
        }

        for (i = 0; i < dim; i++)
            tol[i] = MIN_SC_SEP(intfc) * gr->h[i];
        orient = (fr->step % 2) ? POSITIVE_ORIENTATION : NEGATIVE_ORIENTATION;

        /* 042503 closed 
        for (c = intfc->curves; c and *c; c++)
        {
            if (is_bdry(*c) and (wave_type(*c) == ERROR))
            {
                wave_type(*c) = SUBDOMAIN_BOUNDARY;
                rect_bdry_curve_redist(*c,orient,gr,tol);
            }
        }
        for (n = intfc->nodes; n and *n; n++)
        {
            if (is_bdry(*n) and (node_type(*n) == ERROR))
                node_type(*n) = FIXED_NODE;
        }
        */
/* This part of the code is copied fromt set_subdomain_boundary */
/* 042103 */
        for (c = intfc->curves; c && *c; ++c)
        {
            if (is_bdry(*c) && (wave_type(*c) == ERROR))
            {
                rect_bdry_side_for_curve(&i,&j,*c,gr);
                switch(rect_boundary_type(intfc,i,j))
                {
                case PASSIVE_BOUNDARY:
                    wave_type(*c) = PASSIVE_BOUNDARY;
                    break;
                case SUBDOMAIN_BOUNDARY:
                case AMR_SUBDOMAIN_BOUNDARY:
                case REFLECTION_BOUNDARY:
                    wave_type(*c) = SUBDOMAIN_BOUNDARY;
                    break;
                case MIXED_TYPE_BOUNDARY:
                    if (is_excluded_comp(positive_component(*c),intfc) &&
                        is_excluded_comp(negative_component(*c),intfc))
                        wave_type(*c) = PASSIVE_BOUNDARY;
                    break;
                default:
                    screen("ERROR in set_amr_subdomain_boundary(), "
                           "unexpected case for rect boundary type\n");
                    print_rectangular_grid(gr);
                    printf("wave_type = %d, is_bdry = %d front[%d]\n",
                      wave_type(*c),is_bdry(*c),fr->patch_number);
                    printf("boundary type = %d\n", rect_boundary_type(intfc,i,j));
                    print_curve(*c);
                    clean_up(ERROR);
                }
                rect_bdry_curve_redist(*c,orient,gr,tol);
                if (size_of_state(intfc) != 0)
                {
                    BOND *b;
                    size_t sizest = size_of_state(intfc);
                    obstacle_state(intfc,left_start_state(*c),sizest);
                    obstacle_state(intfc,right_start_state(*c),sizest);
                    for (b=(*c)->first; b!=NULL && b!=(*c)->last; b=b->next)
                    {
                        obstacle_state(intfc,left_state(b->end),sizest);
                        obstacle_state(intfc,right_state(b->end),sizest);
                    }
                    obstacle_state(intfc,left_end_state(*c),sizest);
                    obstacle_state(intfc,right_end_state(*c),sizest);
                }
            }
        }
        for (n = intfc->nodes; n && *n; ++n)
        {
            if (is_bdry(*n) && (node_type(*n) == ERROR))
                node_type(*n) = FIXED_NODE;
        }

        DEBUG_LEAVE(set_amr_subdomain_boundary)
        return FUNCTION_SUCCEEDED;
}

LOCAL boolean amr_check_for_cut_nodes(
        const char      *msg, 
        INTERFACE       *intfc)
{
        NODE            **n, **n2;

        for (n = intfc->nodes; n && *n; ++n)
        {
            if (is_cut_node(*n))
            {
                (void) printf("\n WARNING in amr_check_for_cut_nodes(), cut node "
                              "found at end of %s.\n",msg);
                print_node(*n);
                print_node_flags(*n);
                for (n2 = intfc->nodes; n2 && *n2; ++n2)
                {
                    print_node(*n2);
                    print_node_flags(*n2);
                }
                /* 
                print_interface(intfc);
                */
                for(CURVE **c = intfc->curves; c && *c; c++)
                {
                    if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
                        continue;
                    print_curve(*c);
                }
                return YES;
            }
        }
        return NO;
}               /*end amr_check_for_cut_nodes*/

/* nn is the number of patches which are not transfered from other procs */ 
/* Collect fronts which are originally on this proc and make             */
/* storages for the fronts which are transfer back to this proc          */  
EXPORT  int     set_copy_proc_frs(
	Front      ***tmpfrs,
        int        num_patches,
        Wv_on_pc   **redistr_table,
        int        max_n_patch,
        int        *nn)
{
        int        source, numnodes, myid;
        int        patch_id, i;
        Front      *basefront = NULL;   
        Front      *front; 
        int        total_patch; 
        COMPONENT  dummy = -1; 
        RECT_GRID  *gr;  
        INTERFACE  *sav_intfc;
        boolean       sav_copy;    

        DEBUG_ENTER(set_copy_proc_frs)

        numnodes = pp_numnodes();
        myid = pp_mynode(); 
  
        for(i = 0; i < max_n_patch; i++)
        {
            if(-1 == redistr_table[myid][i].wv_id) continue;
            if(myid == redistr_table[myid][i].pc_id &&
               redistr_table[myid][i].wv_id == 0) 
            {
                basefront = redistr_table[myid][i].front;  
                total_patch = basefront->totalNumberOfPatches;
                break;  
            } 
        } 
        if((basefront == NULL) ||
           (basefront->patch_number != 0 && basefront->patch_level != 0))
        {
            printf("set_copy_proc_frs() "); 
            printf("did not find out the basefront\n");
            clean_up(ERROR);  
        }  

        uni_array(tmpfrs,total_patch,sizeof(Front*));
        for(i = 0; i < total_patch; i++) (*tmpfrs)[i] = NULL;

        /* Save local patches which are not transfered to the other procs */ 
        *nn = 0; 
        for(i = 0; i < max_n_patch; i++)
        {
            if(-1 == redistr_table[myid][i].wv_id) continue;
            if(myid == redistr_table[myid][i].pc_id)
            {
                patch_id = redistr_table[myid][i].wv_id;
                (*tmpfrs)[patch_id] = redistr_table[myid][i].front;
                if(patch_id != redistr_table[myid][i].front->patch_number)
                {
                    printf("ERROR: set_copy_proc_frs()\n");
                    printf("resid Front[%d] NOT same in redistr_table and frs\n",i);
                    clean_up(ERROR);
                }
                (*nn)++;
            }
        }

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        set_size_of_intfc_state(size_of_state(basefront->interf));
        set_copy_intfc_states(YES);

        /* make storage for recving patches from other procs */  
        for(i = 0; i < total_patch; i++)
        {
            if((*tmpfrs)[i] != NULL)
                continue;
            (*tmpfrs)[i] = deep_copy_front(basefront);
            (*tmpfrs)[i]->interf = copy_interface(basefront->interf);
            delete_patch_all_curves((*tmpfrs)[i]);
        }

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(set_copy_proc_frs)
        return  total_patch;  
}

/*  set_copy_proc_frs_ver2().
 * nn is the number of patches which are not transfered 
 * from other procs. Collect fronts (by making a copy) 
 * which are originally on this proc and make storages 
 * for the fronts which are transfer back to this proc.
 */  
LOCAL int set_copy_proc_frs_ver2(
	Front      ***tmpfrs,
        Front      **frs,
        int        num_patches,
        Wv_on_pc   **redistr_table,
        int        max_n_patch,
        int        *nn)
{
        int        source, numnodes, myid;
        int        patch_id, i;
        Front      *basefront = NULL;   
        Front      *front; 
        int        total_patch; 
        COMPONENT  dummy = -1; 
        RECT_GRID  *gr;  
        INTERFACE  *sav_intfc;
        boolean       sav_copy;    

        DEBUG_ENTER(set_copy_proc_frs_ver2)

        numnodes = pp_numnodes();
        myid = pp_mynode(); 
  
        for(i = 0; i < max_n_patch; i++)
        {
            if(-1 == redistr_table[myid][i].wv_id) continue;
            if(myid == redistr_table[myid][i].pc_id &&
               redistr_table[myid][i].wv_id == 0) 
            {
                basefront = redistr_table[myid][i].front;  
                total_patch = basefront->totalNumberOfPatches;
                break;  
            } 
        } 
        if((basefront == NULL) || 
           (basefront->patch_number != 0 && basefront->patch_level != 0))
        {
            printf("ERROR in assembly_distribute_patch_fronts()\n");  
            printf("set_copy_proc_frs_ver2() "); 
            printf("did not find out the basefront\n");
            clean_up(ERROR);  
        }  

        uni_array(tmpfrs,total_patch,sizeof(Front*));
        for(i = 0; i < total_patch; i++) (*tmpfrs)[i] = NULL; 

        /* Save local grids which are not transfered to the other procs */ 
        /* redistr_table[myid][i].front = frs[i], they are set to be equal */
        *nn = 0; 
        for(i = 0; i < max_n_patch; i++)
        {
            if(-1 == redistr_table[myid][i].wv_id) continue;
            if(myid == redistr_table[myid][i].pc_id)
            {
                /*  closed on 061003. 
                (*tmpfrs)[*nn] = copy_front(redistr_table[myid][i].front);
                */ 
                patch_id = redistr_table[myid][i].wv_id; 
                (*tmpfrs)[patch_id] = copy_front(redistr_table[myid][i].front);
                if(redistr_table[myid][i].front != frs[i] ||
                   redistr_table[myid][i].wv_id != 
                   redistr_table[myid][i].front->patch_number)
                {
                    printf("ERROR: set_copy_proc_frs_ver2()\n");
                    printf("resid Front[%d] NOT same in redistr_table and frs\n",i);
                    clean_up(ERROR);
                }
                /* This nn option will be removed later, 061003 */
                (*nn)++;
            }
        }

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        set_size_of_intfc_state(size_of_state(basefront->interf));
        set_copy_intfc_states(YES);

        /* make storage for recving patches from other procs */  
        /* 061003 closed. Use the below new alg. 
        for(i = *nn; i < total_patch; i++)
        {
            (*tmpfrs)[i] = deep_copy_front(basefront);
            (*tmpfrs)[i]->interf = copy_interface(basefront->interf);
            delete_patch_all_curves((*tmpfrs)[i]);
        }
        */  
        for(i = 0; i < total_patch; i++)
        {
            if((*tmpfrs)[i] != NULL) 
                continue;  
            (*tmpfrs)[i] = deep_copy_front(basefront);
            (*tmpfrs)[i]->interf = copy_interface(basefront->interf);
            delete_patch_all_curves((*tmpfrs)[i]);
        }

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(set_copy_proc_frs_ver2)
        return  total_patch;  
}

/* clear_copy_frs_intfc()
 * If the front is copied from local front, 
 * the associated interface pointer is set the NULL. 
 * If the front is copied from non-local front(by MPI),
 * the associated interface is deleted.
 */
LOCAL void clear_copy_frs_intfc(
	Front      **cpyfrs,
        Wv_on_pc   **redistr_table,
        int        max_n_patch,
        int        del_fr)
{
        int        myid;
        int        i, j;
        int        total_patch; 
        int        is_local; 

        DEBUG_ENTER(clear_copy_frs_intfc)

        myid = pp_mynode(); 
  
        total_patch = cpyfrs[0]->totalNumberOfPatches;

        for(i = 0; i < total_patch; i++)
        {
            is_local = NO;  
            for(j = 0; j < max_n_patch; j++)
            {
                if(-1 == redistr_table[myid][j].wv_id) 
                    continue;
                if(myid == redistr_table[myid][j].pc_id)
                {
                    if(redistr_table[myid][j].wv_id == i)
                        is_local = YES; 
                }
            }
            if(YES == is_local)
            {
                cpyfrs[i]->interf = NULL; 
                if(del_fr == YES)
                   free_front(cpyfrs[i]); 
            }
            else
            {
                delete_interface(cpyfrs[i]->interf); 
                cpyfrs[i]->interf = NULL; 
                if(del_fr == YES)
                    deep_free_front(cpyfrs[i]); 
            } 
        }
        DEBUG_LEAVE(clear_copy_frs_intfc)
}

/* 
 * assembly_distribute_patch_fronts(),
 * only work on the base patch and the finest level patches 
 * if all_level = NO. 
 * Otherwise work on all patches if all_level = YES.
 * 061303, all_level is set to YES only after
 * the front redistribution. AFTER tangential front
 * propagation, the patch interfaces are stitched
 * together on base grid, then base grid is responsible
 * for the interface redistribution. After the redistribution,
 * the base grid interface is ft_assigned to each patches. 
 */  

LOCAL  int    distribute_fronts_base_to_patch(
        Front      **frs,
        int        num_patches,    /* number of patches computed in the proc */ 
        Wv_on_pc   **redistr_table,
        int        max_n_patch)
{
        int        source, numnodes, myid;
        int        dist;  
        int        patch_id, i;
        Front      **tmpfrs, *front; 
        int        total_patch;
        int        nn, bal_n;  
        boolean       sav_copy; 
        long       intfc_modified;
        COMPONENT  dummy = -1; 
        RECT_GRID  *cgr, *tgr;
        RECT_GRID  *rgr; 

        DEBUG_ENTER(distribute_fronts_base_to_patch)  

        numnodes = pp_numnodes();
        myid = pp_mynode(); 
        sav_copy = copy_intfc_states();

        set_current_interface(frs[0]->interf); 
        intfc_modified = frs[0]->interf->modified;
        pp_global_lmax(&intfc_modified,1L);
        if (intfc_modified)
        {
            if(!scatter_front(frs[0]))
            {
                printf("ERROR distribute_fronts_base_to_patch(), "
                  "scatter_front() failed for the glued front\n");
                clean_up(ERROR);  
            }
        }

        total_patch = set_copy_proc_frs_ver2(&tmpfrs,frs,num_patches,
                         redistr_table,max_n_patch,&nn);
        bal_n = nn;

        for(source = 0; source < numnodes; source++)
        {
            for(i = 0; i < max_n_patch; i++)
            {
                patch_id = -100;
                if(-1 == redistr_table[source][i].wv_id) continue;
                if(source == redistr_table[source][i].pc_id) continue;

                if(myid == redistr_table[source][i].pc_id)
                {
                    patch_id = redistr_table[source][i].wv_id;
                    front = redistr_table[source][i].front;  

                    pp_send(0,(POINTER)(&patch_id),sizeof(int),source);
                    send_mini_front_misc(front,&dummy,source);
                    /* 
                    printf("patch[%d] front %p: Proc [%d] Send to proc[%d]\n",
                            redistr_table[source][i].wv_id, front, myid, source);
                    */
                }
                if(source == myid)
                {
                    pp_recv(0,redistr_table[source][i].pc_id,
                            (POINTER)(&patch_id),sizeof(int));
                    recv_mini_front_misc(tmpfrs[patch_id],
                        &dummy, redistr_table[source][i].pc_id);
                    front = tmpfrs[patch_id];
                    front->interf->modified = YES;
                    rgr = front->rect_grid;
                    cgr = computational_grid(front->interf);
                    copy_rect_grid(cgr,rgr);
                    tgr = &topological_grid(front->interf);
                    tgr->Remap.remap = rgr->Remap.remap;
                    set_patch_topo_grid(rgr,tgr);
                    /* 
                    printf("redist[%d][%d] received at original: ",source,i);
                    printf("patch[%d]: Proc [%d] recv from proc[%d]\n",
                          patch_id, source, redistr_table[source][i].pc_id);
                    */
                }
                pp_gsync();
            }
        }

        for(source = 0; source < numnodes; source++)
        {
            for(i = 1; i < max_n_patch; i++)
            {
                if(-1 == redistr_table[source][i].wv_id) continue; 
                if(source == redistr_table[source][i].pc_id &&
                   myid == source) 
                {
                    front = redistr_table[source][i].front;
                    delete_interface(front->interf); 
                    front->interf = NULL; 
                }
                else if(myid == redistr_table[source][i].pc_id &&
                   source != myid)
                {
                    front = redistr_table[source][i].front;
                    delete_patch_all_curves(front);
                } 
            }
        } 

        for(i = 1; i < total_patch; i++)
        {
            RECT_GRID   *tmpgr;
            set_patch_front(frs[0],tmpfrs[i],tmpfrs[i]->rect_grid,i);
            if(clip_patch_front(tmpfrs[i],NO) == FUNCTION_FAILED)
            {
                printf("ERROR in distribute_fronts_base_to_patch(),"
                   " clip_patch_front() for tmpfrs[i] failed \n",i);
                print_interface(tmpfrs[i]->interf);
                clean_up(ERROR);
            }
            /* 
            printf("The clipped interface[%d]\n",i);
            print_interface(tmpfrs[i]->interf); 
            */
        }

        for(i = 1; i < max_n_patch; i++)
        {
            if(-1 == redistr_table[myid][i].wv_id) continue; 
            if(myid == redistr_table[myid][i].pc_id) 
            {
                front = redistr_table[myid][i].front;
                patch_id = front->patch_number; 
                delete_interface(front->interf); 
                front->interf = tmpfrs[patch_id]->interf; 
                tmpfrs[patch_id]->interf = NULL;  
            }
        } 

        for(source = 0; source < numnodes; source++)
        {
            for(i = 0; i < max_n_patch; i++)
            {
                patch_id = -100;
                if(-1 == redistr_table[source][i].wv_id) continue;
                if(source == redistr_table[source][i].pc_id) continue;

                if(myid == source)
                {
                    patch_id = redistr_table[source][i].wv_id;
                    pp_send(0,(POINTER)(&patch_id),sizeof(int),
                            redistr_table[source][i].pc_id);

                    send_front_misc(tmpfrs[patch_id],
                        &dummy, redistr_table[source][i].pc_id);
                    /* 
                    printf("Proc[%d] sent patch[%d] front to Proc[%d]\n",
                         myid, patch_id, redistr_table[source][i].pc_id);
                    */
                    delete_interface(tmpfrs[patch_id]->interf);
                    tmpfrs[patch_id]->interf = NULL;
                }
                if(myid == redistr_table[source][i].pc_id)
                {
                    pp_recv(0,source,(POINTER)(&patch_id),sizeof(int)) ;
                    /* 
                    printf("Proc[%d] received patch[%d] front from Proc[%d]\n",
                            myid, patch_id, source);
                    */
                    front = front_on_redistr_table(redistr_table,
                         max_n_patch, myid, patch_id, source);
                    recv_front_misc(front, &dummy, source);
                }
                pp_gsync();
            }
        }

        clear_copy_frs_intfc(tmpfrs,redistr_table, max_n_patch,YES);
        free(tmpfrs);

        for(source = 0; source < numnodes; source++)
        {
            for(i = 1; i < max_n_patch; i++)
            {
                if(-1 == redistr_table[source][i].wv_id) continue;
                if(source == redistr_table[source][i].pc_id) continue;
                if(myid == source) continue;
                if(myid != redistr_table[source][i].pc_id) continue;
                front = front_on_redistr_table(redistr_table,
                     max_n_patch, myid, redistr_table[source][i].wv_id, source);
                rgr = front->rect_grid;
                cgr = computational_grid(front->interf);
                copy_rect_grid(cgr,rgr);
                tgr = &topological_grid(front->interf);
                tgr->Remap.remap = rgr->Remap.remap;
                set_patch_topo_grid(rgr,tgr);
            }
        }
            
        set_current_interface(frs[0]->interf); 
        set_copy_intfc_states(sav_copy);

        DEBUG_LEAVE(distribute_fronts_base_to_patch)  
        return GOOD_STEP; 
}

EXPORT  int    assembly_distribute_patch_fronts(
        Front      **frs,
        int        num_patches,    /* number of patches computed in the proc */ 
        Wv_on_pc   **redistr_table,
        int        max_n_patch,
        int        all_level)
{
        int        source, numnodes, myid;
        int        dist;  
        int        patch_id, i;
        Front      **tmpfrs, *front; 
        Front      *glue_fr; 
        int        total_patch, levels;
        int        nn, bal_n;  
        boolean       sav_copy; 
        INTERFACE  *current_intfc; 
        COMPONENT  dummy = -1; 
        RECT_GRID  *cgr, *tgr;
        RECT_GRID  *rgr; 

        DEBUG_ENTER(assembly_distribute_patch_fronts)  

        if(NULL == frs[0]->interf) 
        {
            printf("ERROR: assembly_distribute_patch_fronts()\n");  
            printf("frs[0] interface is NULL\n");  
            clean_up(ERROR);  
        } 
        if(all_level == YES)
        {
            /* applied only after the base 
             * front redistribution.
             */
            /* 
            distribute_fronts_base_to_patch(frs, num_patches, 
                  redistr_table, max_n_patch);  
            DEBUG_LEAVE(assembly_distribute_patch_fronts)  
            return GOOD_STEP; 
            */
        }

        numnodes = pp_numnodes();
        myid = pp_mynode(); 
        sav_copy = copy_intfc_states();
        
        if(DEBUG)
        {
            printf("assembly_distribute_patch_fronts() Entering : ");
            printf("STORAGE: %-d \n", get_vmalloc_storage_use());
        }

        /* total_patch: number of total patches before redistribute */
        /* NUMBER of tmpfrs = total_patch */  
        /* nn: number of patches which are not transfered to other procs */ 

        total_patch = set_copy_proc_frs_ver2(&tmpfrs,frs,num_patches,
                         redistr_table,max_n_patch,&nn); 
        levels = tmpfrs[0]->NumberOfLevels; 
        bal_n = nn;  

        set_current_interface(frs[0]->interf); 
        for(source = 0; source < numnodes; source++)
        {
            for(i = 0; i < max_n_patch; i++)
            {
                patch_id = -100;
                if(-1 == redistr_table[source][i].wv_id) continue;
                if(source == redistr_table[source][i].pc_id) continue;
                
                if(myid == redistr_table[source][i].pc_id)
                {
                    patch_id = redistr_table[source][i].wv_id; 
                    front = redistr_table[source][i].front;  
                    /*  
                    printf("redist[%d][%d] back to original: front[%p]->interf:%p\n",
                      source, i, front, front->interf);
                    */  
                    if(exists_interface(front->interf) != YES)
                        printf("WARNING: Invalid Interface send in "
                          " assembly_distribute_patch_fronts, "
                              "Probably already deleted\n");  
                    pp_send(0,(POINTER)(&patch_id),sizeof(int),source);   
                    send_front_misc(front,&dummy,source);  
                    /* 
                    printf("patch[%d] front %p: Proc [%d] Send to proc[%d]\n",
                            redistr_table[source][i].wv_id, front, myid, source);   
                    */ 
                } 
                if(source == myid)
                {
                    pp_recv(0,redistr_table[source][i].pc_id,
                            (POINTER)(&patch_id),sizeof(int));  
                    recv_front_misc(tmpfrs[patch_id],
                        &dummy, redistr_table[source][i].pc_id);  
                    front = tmpfrs[patch_id];
                    front->interf->modified = YES;
                    rgr = front->rect_grid;
                    cgr = computational_grid(front->interf);
                    copy_rect_grid(cgr,rgr);
                    tgr = &topological_grid(front->interf);
                    tgr->Remap.remap = rgr->Remap.remap;
                    set_patch_topo_grid(rgr,tgr); 

                    /* 061030 closed. The using of "nn" will be removed.
                    recv_front_misc(tmpfrs[nn],
                        &dummy, redistr_table[source][i].pc_id);  
                    */
                    nn++;  
                    /* 
                    printf("redist[%d][%d] received at original: ",source,i);
                    printf("patch[%d]: Proc [%d] recv from proc[%d]\n",
                          patch_id, source, redistr_table[source][i].pc_id);   
                    */   
                }  
                pp_gsync();   
            }
        }

        /* 070403, add set_amr_intfc_tol() in
         * assembly_fine_patch_fronts_to_one() to the
         * glue->interf to get the consistence of
         * the tolerance on the interface operation.
         * The glued interface based on the coarse grid should
         * also have fine grid interface tolerance.
         */ 
        glue_fr = copy_front(tmpfrs[0]); 
        assembly_fine_patch_fronts_to_one(tmpfrs, glue_fr); 
 
        set_current_interface(glue_fr->interf); 
        set_use_delete_short_bonds_flag();  
        /*
         * 070903 add. Use this flag, not to alter the interface during
         * the interface assembly and distribution process.
         */
        set_min_sc_sep_val_flag(YES);    
        if(!scatter_front(glue_fr))
        {
            printf("ERROR in assembly_distribute_patch_fronts(), "
                 "scatter_front() failed for the glued front\n");
            clean_up(ERROR);  
        }
        set_min_sc_sep_val_flag(NO); 
        /*
         * Set fine grids buffer zone interface on tmpfrs.
         * The buffer zone interface is going to stitch
         * to the interior interface originally contained
         * in the fine grids. This procedure is an
         * analogy to the scatter_front().
         * Base grid interface is not handled here.
         */
        /* Other level grid interfaces are also set here
         * by the calling of clip_patch_front().
         */
        set_current_interface(glue_fr->interf); 
        clear_copy_frs_intfc(tmpfrs,redistr_table, max_n_patch,NO);

        /*
         * 070903 add. Use this flag, not to alter the interface during
         * the interface assembly and distribution process.
         */
        set_min_sc_sep_val_flag(YES);
        for(i = 1; i < total_patch; i++)
        {
            RECT_GRID   *tmpgr;
            if(all_level != YES)
            {
                if(tmpfrs[i]->patch_level != (levels - 1))
                    continue;
            }
            set_patch_front(glue_fr,tmpfrs[i],tmpfrs[i]->rect_grid,i);
            if(tmpfrs[i]->patch_level == (levels - 1)) 
                use_delete_short_bonds = NO;  

            /* 070403 added set_amr_intfc_tol() */
            set_amr_intfc_tol(tmpfrs[i]->interf,
	    		pow(2.0,(double)tmpfrs[i]->patch_level));

            if(clip_patch_front(tmpfrs[i],NO) == FUNCTION_FAILED)
            {
                printf("ERROR in assembly_distribute_patch_fronts(),"
                   " clip_patch_front() for tmpfrs[%s] failed \n",i);
                print_interface(tmpfrs[i]->interf);
                clean_up(ERROR);
            }
            if(tmpfrs[i]->patch_level != (levels - 1))
                continue;
            tmpgr = computational_grid(tmpfrs[i]->interf);
            set_current_interface(tmpfrs[i]->interf);
            delete_subdomain_curves(tmpfrs[i]->interf);
            delete_passive_boundaries(tmpfrs[i]->interf);
            clip_interface_with_rect(tmpfrs[i]->interf,tmpgr->L,tmpgr->U,YES);

            if(debugging("pseudo_scatter_patch_fronts") && 
               (tmpfrs[i]->patch_number == 3))
            {
                printf("CLIPPED buffer interface[%d] level [%d]is:\n", 
                     tmpfrs[i]->patch_number, tmpfrs[i]->patch_level);
                print_rectangular_grid(tmpfrs[i]->rect_grid);  
                for(CURVE **cc = tmpfrs[i]->interf->curves; cc && *cc; cc++)
                {
                    if(wave_type(*cc) < FIRST_PHYSICS_WAVE_TYPE)
                        continue;
                    print_curve(*cc);
                }
            }
        }
        set_min_sc_sep_val_flag(NO);

        set_current_interface(frs[0]->interf); 
        pseudo_scatter_patch_fronts(frs, tmpfrs, glue_fr, bal_n,
            num_patches, redistr_table, max_n_patch, all_level);

        clear_copy_frs_intfc(tmpfrs,redistr_table, max_n_patch,YES);
        free(tmpfrs);

        if(all_level == YES)
        {
            set_current_interface(glue_fr->interf);
            delete_interface(frs[0]->interf);
            frs[0]->interf = glue_fr->interf;
            glue_fr->interf = NULL;

            frs[0]->interf->modified = YES;
            rgr = frs[0]->rect_grid;
            cgr = computational_grid(frs[0]->interf);
            copy_rect_grid(cgr,rgr);
            tgr = &topological_grid(frs[0]->interf);
            tgr->Remap.remap = rgr->Remap.remap;
            set_patch_topo_grid(rgr,tgr);
        }
        else
        {
            set_current_interface(frs[0]->interf);
            set_amr_intfc_tol(frs[0]->interf,pow(2.0,1.0-frs[0]->NumberOfLevels));
            set_use_delete_short_bonds_flag();  

            if(! scatter_front(frs[0]))
            {
                printf("ERROR in assembly_distribute_patch_fronts(), "
                     "scatter_front() failed for the base front\n");
                clean_up(ERROR);  
            }
        } 

        for(i = 0; i < num_patches; i++)
        {
            if(frs[i]->patch_level == (levels - 1) || 
               frs[i]->patch_level == 0)
                intfc_delete_very_short_bonds(frs[i]);

            /* 070403, set back the interface operation
             * tolerance, now the tolerance are not consistent
             * among different levels.
             */
            if(all_level != YES)
            {
                if(frs[i]->patch_level != (levels - 1) &&
                   frs[i]->patch_level != 0)
                {
                    continue;
                }
            } 
            set_amr_intfc_tol(frs[i]->interf,
                pow(2.0,frs[i]->NumberOfLevels-1.0-frs[i]->patch_level));
        }  
        free_front(glue_fr);
        set_copy_intfc_states(sav_copy);

        for(i = 0; i < num_patches; i++)
        {
            if(frs[i]->patch_number == 3 && DEBUG)
            {
                printf("IN  assembly_distribute_patch_fronts\n"); 
                printf("THE assembled scattered interface[3] is:\n");
                for(CURVE **cc = frs[i]->interf->curves; cc && *cc; cc++)
                {
                    if(wave_type(*cc) < FIRST_PHYSICS_WAVE_TYPE)
                        continue;
                    print_curve(*cc);
                }
                printf("assembly_distribute_patch_fronts() Leaving : ");
                printf("STORAGE: %-d \n", get_vmalloc_storage_use());
            }
        }

        DEBUG_LEAVE(assembly_distribute_patch_fronts)  
        return GOOD_STEP; 
}

/* pseudo_scatter_patch_fronts()
 * the fine grid interfaces are stitched by
 * the interior interfaces and the received 
 * interfaces. Base grid interface is not handled
 * in this function. Other level grid interfaces
 * are created by clip_patch_front().  
 */ 
LOCAL boolean pseudo_scatter_patch_fronts(
        Front      **frs,
        Front      **tmpfrs,  
        Front      *glue_fr, 
        int        bal_n,
        int        num_patches,  
        Wv_on_pc   **redistr_table,
        int        max_n_patch,
        int        all_level)
{
        int        source, numnodes, myid;
        int        dist, patch_id;  
        int        i, j, dim = frs[0]->rect_grid->dim;
        Front      *front; 
        int        total_patch, levels; 
        int        *lbuf, *ubuf;
        boolean       sav_copy, status;
        boolean       save_intrp;
        double      coords[MAXD];
        int        nn;  
        INTERFACE  *sav_intfc; 
        COMPONENT  dummy = -1, *i_comp; 
        RECT_GRID  *cgr, *tgr;
        RECT_GRID  *rgr; 

        DEBUG_ENTER(pseudo_scatter_patch_fronts)  

        numnodes = pp_numnodes();
        myid = pp_mynode(); 
        total_patch = tmpfrs[0]->totalNumberOfPatches; 
        levels = tmpfrs[0]->NumberOfLevels;
      
        sav_intfc = current_interface(); 
        sav_copy = copy_intfc_states(); 

        /*
         * 070903 add. Use this flag, not to alter the interface during
         * the interface assembly and distribution process.
         */
        set_min_sc_sep_val_flag(YES);

        /* 
         * Prepare LOCAL and NON-LOCAL FINE grid fronts for 
         * receiving buffer zone interface from other procs
         * by clipping off the buffer zone interfaces. 
         */
        uni_array(&i_comp, num_patches, sizeof(COMPONENT));  
        for(i = 1; i < num_patches; i++)
        {
            if(frs[i]->patch_level != (levels - 1)) 
                continue;
            lbuf = frs[i]->rect_grid->lbuf;
            ubuf = frs[i]->rect_grid->ubuf;
            set_current_interface(frs[i]->interf); 
            for (j = 0; j < dim; ++j)
                coords[j] = grid_center_coord(j,frs[i]->rect_grid);
            i_comp[i] = (frs[i]->interf->modified) ?
                long_component(coords,frs[i]->interf) : 
                component(coords,frs[i]->interf);
            save_intrp = interpolate_intfc_states(frs[i]->interf);
            interpolate_intfc_states(frs[i]->interf) = NO;
            delete_subdomain_curves(frs[i]->interf);
            delete_passive_boundaries(frs[i]->interf);
            amr_clip_to_interior_region(frs[i]->interf,lbuf,ubuf,YES);
            interpolate_intfc_states(frs[i]->interf) = save_intrp;                

            if(debugging("pseudo_scatter_patch_fronts") && 
               (frs[i]->patch_number == 3))
            {
                printf("CLIPPED interface[%d] level [%d]is:\n",
                     frs[i]->patch_number, frs[i]->patch_level);
                print_rectangular_grid(frs[i]->rect_grid);
                for(CURVE **cc = frs[i]->interf->curves; cc && *cc; cc++)
                {
                    if(wave_type(*cc) < FIRST_PHYSICS_WAVE_TYPE)
                        continue;
                    print_curve(*cc);
                }
            }
        } 
        set_min_sc_sep_val_flag(NO);

        /*
         * For the local FINE grid fronts which are not transfered,
         * copy the buffer zone interface into them. 
         */ 
        for(i = 0; i < max_n_patch; i++)
        {
            if(-1 == redistr_table[myid][i].wv_id) continue;
            if(myid != redistr_table[myid][i].pc_id) continue; 
            if(redistr_table[myid][i].front->patch_level != (levels - 1))
                continue;  

            patch_id = redistr_table[myid][i].wv_id; 
            front = redistr_table[myid][i].front; 
            if(front != frs[patch_id])
            {
                printf("ERROR: pseudo_scatter_patch_fronts()"); 
                printf("front from redistribute table and frs"
                       " are not consistent\n"); 
                printf("Patch_id[%d], level[%d]\n",patch_id,
                       redistr_table[myid][i].wv_level);
                printf("redistr_table[%d][%d]\n",myid, i); 
                print_interface(front->interf);
                print_interface(frs[patch_id]->interf);
                clean_up(ERROR);  
            } 
            save_intrp = interpolate_intfc_states(front->interf);
            set_current_interface(front->interf);
            interpolate_intfc_states(front->interf) = NO;
            copy_interface_into(tmpfrs[patch_id]->interf,front->interf);
            interpolate_intfc_states(front->interf) = save_intrp;

            delete_interface(tmpfrs[patch_id]->interf);    
            tmpfrs[patch_id]->interf = NULL;    

            for(j = 0; j < dim; j++)
            {
                status = merge_interface(front,j);
                if (status == FUNCTION_FAILED)
                {
                    printf("ERROR in pseudo_scatter_patch_fronts(), "
                      "local merge_interface() failed at dir = %d\n", j);
                    print_interface(front->interf);  
                    clean_up(ERROR);
                }
            }
            /* 
            if(debugging("pseudo_scatter_patch_fronts"))
            {
                printf("MERGED interface[%d], local id[%d]:\n", 
                    patch_id, i);
                print_interface(frs[patch_id]->interf);
            } 
            */ 
        }

        /* For the LOCAL OTHER LEVEL grid fronts, 
         * delete the OLD LOCAL front interface, 
         * set the grid front interface points to the copy 
         * front("tmpfrs") interface. 
         */  
        set_current_interface(sav_intfc); 
        if(YES == all_level)
        {
            for(i = 0; i < max_n_patch; i++)
            {
                if(-1 == redistr_table[myid][i].wv_id) continue;
                if(myid != redistr_table[myid][i].pc_id) continue; 
                if(redistr_table[myid][i].front->patch_level == (levels - 1))
                    continue;  
                if(redistr_table[myid][i].front->patch_level == 0)
                    continue;  

                patch_id = redistr_table[myid][i].wv_id; 
                front = redistr_table[myid][i].front; 
                if(front != frs[patch_id])
                {
                    printf("ERROR: pseudo_scatter_patch_fronts()"); 
                    printf("front from redistribute table and frs"
                          " are not consistent\n"); 
                    printf("Patch_id[%d], level[%d]\n",patch_id,
                          redistr_table[myid][i].wv_level);
                    printf("redistr_table[%d][%d]\n",myid, i); 
                    print_interface(front->interf);
                    print_interface(frs[patch_id]->interf);
                    clean_up(ERROR);  
                } 
                delete_interface(front->interf); 
                front->interf = tmpfrs[patch_id]->interf;
                tmpfrs[patch_id]->interf = NULL; 
            }
        } 
        /*
         * For the NON-LOCAL FINE grid fronts which are transfered
         * to the other processors, MPI-send the buffer zone 
         * interface into them. For the OTHER LEVEL non-local
         * grid fronts, the completed interfaces are also
         * sent by the MPI. 
         * The OLD NON-LOCAL OTHER LEVEL (NOT the FINE LEVEL) front 
         * interfaces are deleted. 
         */ 
       
        nn = bal_n;   
        for(source = 0; source < numnodes; source++)
        {
            for(i = 0; i < max_n_patch; i++)
            {
                patch_id = -100;
                if(-1 == redistr_table[source][i].wv_id) continue;
                if(source == redistr_table[source][i].pc_id) continue;
                if(all_level != YES)
                {
                    if(redistr_table[source][i].wv_level != (levels - 1)) 
                        continue;
                }

                if(myid == source)
                {
                    patch_id = redistr_table[source][i].wv_id;
                    pp_send(0,(POINTER)(&patch_id),sizeof(int),
                            redistr_table[source][i].pc_id);

                    /* 061003, send buffer interface should be 
                     * enough.
                     */
                    send_front_misc(tmpfrs[patch_id],
                        &dummy, redistr_table[source][i].pc_id);
                    /* 
                    printf("Proc[%d] sent patch[%d] front to Proc[%d]\n",
                         myid, patch_id, redistr_table[source][i].pc_id);
                    */  
                    delete_interface(tmpfrs[patch_id]->interf);
                    tmpfrs[patch_id]->interf = NULL;
                }
                if(myid == redistr_table[source][i].pc_id)
                {
                    pp_recv(0,source,(POINTER)(&patch_id),sizeof(int)) ;
                    /* 
                    printf("Proc[%d] received patch[%d] front from Proc[%d]\n",
                            myid, patch_id, source);
                    */  
                    /* 
                     * 061003, use front_on_redistr_table() to find
                     * the front which will receive sent interface.
                     */
                    front = front_on_redistr_table(redistr_table, 
                         max_n_patch, myid, patch_id, source);
                    if(front->patch_level != (levels-1))
                    {
                        delete_patch_all_curves(front);
                    }  
                    recv_front_misc(front, &dummy, source);
                    /* 
                    recv_front_misc(frs[nn], &dummy, source);
                    */
                    nn++;
                }
                pp_gsync();
            }
        }

        /*
         * For the NON-LOCAL FINE grid fronts,
         * interface needs to be merged.
         * For the other level non-local
         * grid fronts, set rect_grids.  
         */ 
        for(source = 0; source < numnodes; source++)
        {
            for(i = 1; i < max_n_patch; i++)
            {
                if(-1 == redistr_table[source][i].wv_id) continue;
                if(source == redistr_table[source][i].pc_id) continue;
                if(myid == source) continue;  
                if(myid != redistr_table[source][i].pc_id) continue;
                if(redistr_table[source][i].wv_level != (levels - 1))
                {
                    front = front_on_redistr_table(redistr_table,
                         max_n_patch, myid, redistr_table[source][i].wv_id, source);
                    rgr = front->rect_grid;
                    cgr = computational_grid(front->interf);
                    copy_rect_grid(cgr,rgr);
                    tgr = &topological_grid(front->interf);
                    tgr->Remap.remap = rgr->Remap.remap;
                    set_patch_topo_grid(rgr,tgr);
                    continue;
                } 

                front = front_on_redistr_table(redistr_table,
                      max_n_patch, myid, redistr_table[source][i].wv_id, source);
                set_current_interface(front->interf);
                for(j = 0; j < dim; j++)
                {
                    status = merge_interface(front,j);
                    if (status == FUNCTION_FAILED)
                    {
                        printf("ERROR in pseudo_scatter_patch_fronts(), "
                          "non-local merge_interface() failed at dir = %d\n", j);
                        clean_up(ERROR);
                    }
                }
                /* 
                if(debugging("pseudo_scatter_patch_fronts"))
                {
                    printf("MERGED non-local interface[%d]\n", 
                        front->patch_number);
                    print_interface(front->interf);
                } 
                */
            }
        } 

        for(i = 1; i < num_patches; i++) 
        {
            if (exists_interface(frs[i]->interf) != YES) 
            {
                printf("ERROR in pseudo_scatter_patch_fronts\n");
                printf("Patch[%d] interface is not in exist lists\n", i);  
                printf("Patch intfc = %p\n", frs[i]->interf);  
                clean_up(ERROR);  
            }  
        }  

        /* Complete the interfaces with boundary and physics info. link.
         * Only the FINE grid interface needs to do so. Other level
         * interfaces have been taken care of. 
         */
        for( i = 1; i < num_patches; i++)
        {
            if(frs[i]->patch_level != (levels-1))
                continue;
            use_delete_short_bonds = NO; 
            (*f_user_interface(frs[i]->interf)._form_patch_subintfc)(frs[i],i_comp[i]);
            if (debugging("merge_fronts") && 
                (frs[i]->patch_number == 3))
            {
                 CURVE **c;  
                 printf("IN pseudo_scatter_patch_fronts()\n");
                 printf("THE assembled front[%d] level[%d]\n",
                   frs[i]->patch_number, frs[i]->patch_level);
                 for(c = frs[i]->interf->curves; c && *c; c++)
                 {
                    if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
                        continue;
                    print_curve(*c);
                 }
            }
        }
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy); 
        free(i_comp); 
        DEBUG_LEAVE(pseudo_scatter_patch_fronts)  
        return GOOD_STEP; 
}

EXPORT  void    assembly_fine_patch_fronts_to_one2d_ver2(
        Front    **oldfrs,
        Front    *newfr)
{
        int             i, j, levels, num_fine_frs;
        int             total_patch;
        int             dim, dir, side;
        INTERFACE       *sav_intfc, *bdy_intfc;
        boolean            sav_copy;
        Front           **frs, *front = NULL;
        Front           *basefront = NULL;   

        DEBUG_ENTER(assembly_fine_patch_fronts_to_one_ver2)

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        levels = oldfrs[0]->NumberOfLevels;
        total_patch = oldfrs[0]->totalNumberOfPatches;
        num_fine_frs = 0; j = 0; 
        for (i = 0; i < total_patch; i++)
        {
            if( oldfrs[i]->patch_level == levels-1)
                num_fine_frs++;
            if( oldfrs[i]->patch_level == 0 && 
                oldfrs[i]->patch_number == 0)
                basefront = oldfrs[i];  
        }

        set_current_interface(basefront->interf);
        set_size_of_intfc_state(size_of_state(oldfrs[0]->interf));
        set_copy_intfc_states(YES);
        if (( newfr->interf = copy_interface(basefront->interf)) == NULL)
        {
            screen("ERROR in assembly_fine_patch_fronts_to_one2d_ver2()",
                 "copy_interface() failed\n");
            clean_up(ERROR);
        }

        /* 070403, add set_amr_intfc_tol()
         * to get the consistence of the tolerance
         * on the interface operation */
        set_amr_intfc_tol(newfr->interf,pow(2.0,1.0-newfr->NumberOfLevels));

        if(0 == num_fine_frs) 
        {
            /* 052203 closed 
            set_current_interface(sav_intfc);
            */  
            set_current_interface(newfr->interf);
            set_copy_intfc_states(sav_copy);
            DEBUG_LEAVE(assembly_fine_patch_fronts_to_one)
            return;   
        }  

        uni_array(&frs,num_fine_frs,sizeof(Front*));

        for (i = 0; i < total_patch; i++)
        {
            if (oldfrs[i]->patch_level == levels-1)
            {
                frs[j] = copy_front(oldfrs[i]);
                set_size_of_intfc_state(size_of_state(oldfrs[i]->interf));
                set_copy_intfc_states(YES);
                if (( frs[j]->interf = copy_interface(oldfrs[i]->interf)) == NULL)
                {
                    screen("ERROR in assembly_fine_patch_fronts_to_one()",
                         "copy_interface() failed\n");
                    clean_up(ERROR);
                }
                j++;
            }
        }
        if(j!=num_fine_frs)
        {
            printf("Error: in assembly_fine_patch_fronts_to_one()\n");
            printf("did not find finest level front, exit j = %d\n", j);
            clean_up(ERROR);
        }
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        merge_fronts_ver2(newfr,frs,num_fine_frs);
        set_current_interface(newfr->interf); 

        for(i = 0 ; i < num_fine_frs; i++)
            free_front(frs[i]);
        free_these(1,frs);

        if(debugging("assembly_fine_patch_fronts_to_one"))
        {
            printf("In assembly_patch_fronts_to_one()\n");
            printf("Print the assembled interface with boundary\n");
            print_interface(newfr->interf);
            show_intfc_states(newfr->interf);
            screen("EXIT in assembly_fine_patch_fronts_to_one\n");
            exit(0);
        }

        DEBUG_LEAVE(assembly_fine_patch_fronts_to_one2d_ver2)
}

EXPORT  void    assembly_fine_patch_fronts_to_one2d(
        Front    **oldfrs,
        Front    *newfr)
{
        int             i, j, levels, num_fine_frs;
        int             total_patch;
        int             dim, dir, side;
        INTERFACE       *sav_intfc, *bdy_intfc;
        boolean            sav_copy;
        Front           **frs, *front = NULL;
        Front           *basefront = NULL;   

        DEBUG_ENTER(assembly_fine_patch_fronts_to_one)

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        levels = oldfrs[0]->NumberOfLevels;
        total_patch = oldfrs[0]->totalNumberOfPatches;
        num_fine_frs = 0; j = 0;

        for (i = 0; i < total_patch; i++)
        {
            if (oldfrs[i]->patch_level == levels-1)
                num_fine_frs++;
            if(oldfrs[i]->patch_level == 0 and 
               oldfrs[i]->patch_number == 0)
                basefront = oldfrs[i];  
        }

        if(0 != num_fine_frs)
            uni_array(&frs,num_fine_frs,sizeof(Front*));

        for (i = 0; i < total_patch; i++)
        {
            if (oldfrs[i]->patch_level == levels-1)
            {
                frs[j] = copy_front(oldfrs[i]);
                set_size_of_intfc_state(size_of_state(oldfrs[i]->interf));
                set_copy_intfc_states(YES);
                if (( frs[j]->interf = copy_interface(oldfrs[i]->interf)) == NULL)
                {
                    screen("ERROR in assembly_fine_patch_fronts_to_one()",
                         "copy_interface() failed\n");
                    clean_up(ERROR);
                }
                j++;
            }
        }

        if(j!=num_fine_frs)
        {
            printf("Error: in assembly_fine_patch_fronts_to_one()\n");
            printf("did not find finest level front, exit j = %d\n", j);
            clean_up(ERROR);
        }
      
        if(0 == j) 
        {
            set_size_of_intfc_state(size_of_state(oldfrs[0]->interf));
            set_copy_intfc_states(YES);
            if (( newfr->interf = copy_interface(oldfrs[0]->interf)) == NULL)
            {
                screen("ERROR in assembly_fine_patch_fronts_to_one()",
                     "copy_interface() failed\n");
                clean_up(ERROR);
            }
            set_current_interface(sav_intfc);
            set_copy_intfc_states(sav_copy);
            DEBUG_LEAVE(assembly_fine_patch_fronts_to_one)
            return;   
        }  

        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        merge_fronts(newfr,frs,num_fine_frs);
        
        for(i = 0 ; i < num_fine_frs; i++)
            free_front(frs[i]);
        free_these(1,frs);

        /*  close 032103 
        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        */  
        front = oldfrs[0];
        dim = front->rect_grid->dim;
        for(dir = 0; dir < dim; dir++)
        {
            for(side = 0; side < 2; side ++)
            {
                rect_boundary_type(newfr->interf, dir, side) =
                    rect_boundary_type(front->interf, dir, side);
            }
        }


        if (debugging("assembly_fine_patch_fronts_to_one"))
        {
            printf("Patch interface after merging:\n");
            print_interface(newfr->interf);
        }

        /* assume only zero level patch contains boundary curves */
        retrieve_boundary_curves_from_zero(newfr, oldfrs[0]->interf);

        newfr->rect_grid = oldfrs[0]->rect_grid;
        Computational_grid(newfr->interf) = Computational_grid(oldfrs[0]->interf);
        topological_grid(newfr->interf) = topological_grid(oldfrs[0]->interf); 

        if(debugging("assembly_fine_patch_fronts_to_one"))
        {
            printf("In assembly_patch_fronts_to_one()\n");
            printf("Print the assembled interface with boundary\n");
            print_interface(newfr->interf);
            show_intfc_states(newfr->interf);
            screen("EXIT in assembly_fine_patch_fronts_to_one\n");
            exit(0);
        }

        DEBUG_LEAVE(assembly_fine_patch_fronts_to_one)
}

LOCAL  void amr_clip_to_interior_region(
        INTERFACE       *intfc,
        int             *lbuf,
        int             *ubuf,
        int             all_force)
{
        RECT_GRID       *gr = computational_grid(intfc);
        int             dir, dim = gr->dim;
        boolean            force_clip;
        NODE            **n;

        DEBUG_ENTER(amr_clip_to_interior_region)
        for (dir = 0; dir < dim; dir++)
        {
            if (lbuf[dir] > 0)
            {
                if(YES == all_force)
                {
                    force_clip = YES;
                }
                else
                {
                    if(rect_boundary_type(intfc,dir,0) == REFLECTION_BOUNDARY or
                       rect_boundary_type(intfc,dir,0) == AMR_SUBDOMAIN_BOUNDARY)
                        force_clip = YES;
                    else
                        force_clip = NO;
                }
                /* See fscat2d.c for the comment. */ 
                set_cut_none_local_flag(YES);  
                cut_interface(intfc,gr->L[dir],dir,1,YES,force_clip);
                set_cut_none_local_flag(NO);  
            }
            if (ubuf[dir] > 0)
            {
                if(YES == all_force)
                {
                    force_clip = YES;
                }
                else
                {
                    if(rect_boundary_type(intfc,dir,1) == REFLECTION_BOUNDARY or
                       rect_boundary_type(intfc,dir,1) == AMR_SUBDOMAIN_BOUNDARY)
                        force_clip = YES;
                    else
                        force_clip = NO;
                } 
                cut_interface(intfc,gr->U[dir],dir,0,YES,force_clip);
            }
        }
        /* 
        *  To assure that if two nodes from twodifferent patches matches, 
        *  one should set to be LOCAL, the other should not set 
        *  to be LOCAL. This is required by the match_overlaps.
        *  For nodes overlap with patch computational rect. boundry, 
        *  we now set node on lower side X_CUT and Y_CUT to be NONE_LOCAL,
        *  node on upper side X_CUT and Y_CUT to be LOCAL. 
        *  See set_cut_none_local_flag().  
        */  
        DEBUG_LEAVE(amr_clip_to_interior_region)
}

LOCAL  int merge_fronts_ver2(
        Front *newfront,
        Front **frs,         /* Fronts all come from finest level. */  
        int num_frs)
{
        int             i, j, dir, dim = frs[0]->rect_grid->dim;
        int             *lbuf,*ubuf;
        int             patch_num;
        INTERFACE       *sav_intfc;
        CURVE           **c;
        RECT_GRID       *gr = computational_grid(newfront->interf); 
        int             n_nodes;
        Locstate        *left_s_states, *left_e_states;
        Locstate        *right_s_states, *right_e_states; 
        NODE            **snodes, **enodes;  
        double           dist; 
        boolean            sav_copy;
        boolean            save_intrp;
        boolean            status;

        debug_print("repatch","Entering merge_fronts()\n");

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        set_current_interface(newfront->interf);  
        delete_sub_bdry_and_interior_curves(newfront->interf);  

        /* 
         * 070903 add. Use this flag, not to alter the interface during 
         * the interface assembly and distribution process.
         */
        set_min_sc_sep_val_flag(YES); 

        for (i = 0; i < num_frs; i++)
        {  
            RECT_GRID   *tmpgr; 
            tmpgr = computational_grid(frs[i]->interf); 
            clip_interface_with_rect(newfront->interf,tmpgr->L,tmpgr->U,YES); 
        }  

        if (debugging("merge_fronts"))
        {
            /* 
            printf("BASE INTERFACE after clip with rectangles\n");
            for(NODE **n = newfront->interf->nodes; n && *n; n++)
            {
                printf("NODE[%g,%g]\n", Coords((*n)->posn)[0],
                   Coords((*n)->posn)[1]);
                print_node_flags(*n);
            }
            for(c = newfront->interf->curves; c && *c; c++)
            {
                if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
                    continue;
                print_curve(*c);
            }
            */  
            /* 
            print_interface(newfront->interf);
            geomview_intfc_plot2d("jet_patch0_intfc",
                      newfront->interf, newfront->rect_grid); 
            printf("EXIT Base interface after clip\n");
            exit(0); 
            */  
        }
 
        for (i = 0; i < num_frs; i++)
        {
            lbuf = frs[i]->rect_grid->lbuf;
            ubuf = frs[i]->rect_grid->ubuf;
            set_current_interface(frs[i]->interf);
            save_intrp = interpolate_intfc_states(frs[i]->interf);
            interpolate_intfc_states(frs[i]->interf) = NO;
         
            delete_subdomain_curves(frs[i]->interf);
            delete_passive_boundaries(frs[i]->interf);
            if (debugging("merge_fronts") &&
                (frs[i]->patch_number == 3))
            {
                 printf("IN merge_fronts_ver2()\n");
                 printf("THE BEFORE-CLIPPED front[%d] level[%d]\n",
                   frs[i]->patch_number, frs[i]->patch_level);
                 print_rectangular_grid(frs[i]->rect_grid);  
                 for(c = frs[i]->interf->curves; c && *c; c++)
                 {
                    if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
                        continue;
                    print_curve(*c);
                 }
            }
            /* 
             * 051503, now fine level grids also contribute to 
             * boundary curve reconstruction.  
             * remove_patch_all_boundary_curves(frs[i]);
             */  
            amr_clip_to_interior_region(frs[i]->interf,lbuf,ubuf,YES);
            interpolate_intfc_states(frs[i]->interf) = save_intrp;

            if (debugging("merge_fronts") && 
                (frs[i]->patch_number == 3))
            {
                 printf("IN merge_fronts_ver2()\n");
                 printf("THE AFTER-CLIPPED front[%d] level[%d]\n",
                   frs[i]->patch_number, frs[i]->patch_level);
                 for(c = frs[i]->interf->curves; c && *c; c++)
                 {
                    if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE)
                        continue;
                    print_curve(*c);
                 }
            }
            /* 
             * 051503: Only the nodes of physical curves (contact curve, etc.)
             * which overlap with the base grid computational domain boundary
             * are cleaned cut_node_flags. Because no two physical curves
             * will be merged on these nodes at this moment.
             */  
            for(c = frs[i]->interf->curves; c && *c; c++)
            {
                if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE) 
                    continue;  
                for(dir = 0; dir < dim; dir++)
                {
                    if( fabs(Coords((*c)->start->posn)[dir] - gr->L[dir]) <= 10.0*MACH_EPS || 
                        fabs(Coords((*c)->start->posn)[dir] - gr->U[dir]) <= 10.0*MACH_EPS) 
                        clear_node_flags((*c)->start);
                    if( fabs(Coords((*c)->end->posn)[dir] - gr->L[dir]) <= 10.0*MACH_EPS || 
                        fabs(Coords((*c)->end->posn)[dir] - gr->U[dir]) <= 10.0*MACH_EPS) 
                        clear_node_flags((*c)->end);
                    if( Coords((*c)->start->posn)[dir] < gr->L[dir] || 
                        Coords((*c)->start->posn)[dir] > gr->U[dir]) 
                        clear_node_flags((*c)->start);
                    if( Coords((*c)->end->posn)[dir] < gr->L[dir] || 
                        Coords((*c)->end->posn)[dir] > gr->U[dir]) 
                        clear_node_flags((*c)->end);
                } 
            } 
        }

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        save_intrp = interpolate_intfc_states(newfront->interf);

        set_current_interface(newfront->interf);
        interpolate_intfc_states(newfront->interf) = NO;
        for (i = 0; i < num_frs; i++)
            copy_interface_into(frs[i]->interf,newfront->interf);
        interpolate_intfc_states(newfront->interf) = save_intrp;

        /* 
         * 070903 add. Use this flag, not to alter the interface during 
         * the interface assembly and distribution process.
         */
        set_min_sc_sep_val_flag(NO); 

        if (debugging("merge_fronts"))
        {
            /* 
            printf("Patch tmp interface after copied into\n");
            print_interface(newfront->interf);
            */
        }

        for(i = 0; i < dim; i++)
        {
            status = merge_interface(newfront,i);
            if (status == FUNCTION_FAILED)
            {
                (void) printf("ERROR in merge_fronts(), "
                      "merge_interface() failed at dir = %d\n", i);
                clean_up(ERROR);
            }
        }
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        if (debugging("merge_fronts") && newfront->step == 5115)
        {
            printf("AFTER MEGRGE interface in merge_fronts_ver2\n");
            /* 
            print_interface(newfront->interf);
            printf("EXIT AFTER MEGRGE interface in merge_fronts_ver2\n");
            printf("BASE INTERFACE after merge\n");
            for(NODE **n = newfront->interf->nodes; n && *n; n++)
            {
                printf("NODE[%g,%g]\n", Coords((*n)->posn)[0],
                   Coords((*n)->posn)[1]);
                print_node_flags(*n);
            }
            */
            for(c = newfront->interf->curves; c && *c; c++)
            {
                if(wave_type(*c) < FIRST_PHYSICS_WAVE_TYPE) 
                    continue;
                print_curve(*c);   
            }
        }

        if (amr_check_for_cut_nodes("merge_fronts_ver2()",newfront->interf) == YES)
        {
            (void) printf("WARNING in  merge_fronts()");
            (void) printf(" amr_check_for_cut_nodes() detected cut node\n");
            clean_up(ERROR);
        }
 
        debug_print("repatch","Leaving merge_fronts_ver2()\n");
        return YES;  
} 

LOCAL  int merge_fronts(
        Front *newfront,
        Front **frs,
        int num_frs)
{
        int             i, j, dir, dim = frs[0]->rect_grid->dim;
        int             *lbuf,*ubuf;
        int             patch_num;
        INTERFACE       *sav_intfc, *tmp_intfc;
        CURVE           **c;
        RECT_GRID       *gr = computational_grid(newfront->interf); 
        int             n_nodes;
        Locstate        *left_s_states, *left_e_states;
        Locstate        *right_s_states, *right_e_states; 
        NODE            **snodes, **enodes;  
        double           dist; 
        boolean            sav_copy;
        boolean            save_intrp;
        boolean            status;

        debug_print("repatch","Entering merge_fronts()\n");

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();

        for (i = 0; i < num_frs; i++)
        {
            lbuf = frs[i]->rect_grid->lbuf;
            ubuf = frs[i]->rect_grid->ubuf;
            set_current_interface(frs[i]->interf);
            save_intrp = interpolate_intfc_states(frs[i]->interf);
            interpolate_intfc_states(frs[i]->interf) = NO;

            remove_patch_all_boundary_curves(frs[i]);
            amr_clip_to_interior_region(frs[i]->interf,lbuf,ubuf,YES);
            interpolate_intfc_states(frs[i]->interf) = save_intrp;
            for(NODE **n = frs[i]->interf->nodes; n && *n; n++)
            {
                for(dir = 0; dir < dim; dir++)
                {
                    if( fabs(Coords((*n)->posn)[dir] - gr->L[dir]) <= 10.0*MACH_EPS || 
                        fabs(Coords((*n)->posn)[dir] - gr->U[dir]) <= 10.0*MACH_EPS) 
                        clear_node_flags(*n);
                    if(Coords((*n)->posn)[dir] < gr->L[dir] || 
                       Coords((*n)->posn)[dir] > gr->U[dir])
                        clear_node_flags(*n);
                    /* 
                     * 051203. Clear cut flags for this type of node. 
                     * This is because at the current moment, only the
                     * interior curve is reconstructed from the fine grids.
                     * If the fine grid boundary overlaps with this node,
                     * No matching physical cut nodes will be found.  
                     * merge_double_physical_cut_nodes() fails.  
                     * For the node_type(), see g_fprint_hsbdry_type(). 
                    */ 
                    if(node_type(*n) >= FIRST_PHYSICS_HSBDRY_TYPE)
                        clear_node_flags(*n);  
                } 
            }
        }

        n_nodes = 0; 
        for (i = 0; i < num_frs; i++)
        {
            for(c = frs[i]->interf->curves; c && *c; c++)
                n_nodes++;  
        } 
        if(n_nodes != 0)
        {
            uni_array(&snodes, n_nodes, sizeof(NODE*));  
            uni_array(&enodes, n_nodes, sizeof(NODE*));  
            uni_array(&left_s_states, n_nodes, sizeof(Locstate));  
            uni_array(&left_e_states, n_nodes, sizeof(Locstate));  
            uni_array(&right_s_states, n_nodes, sizeof(Locstate));  
            uni_array(&right_e_states, n_nodes, sizeof(Locstate));  
            j = 0; 
            for (i = 0; i < num_frs; i++)
            {
                for(c = frs[i]->interf->curves; c && *c; c++)
                {
                    snodes[j] = (*c)->start; 
                    left_s_states[j] = left_start_state(*c); 
                    right_s_states[j] = right_start_state(*c); 
                    enodes[j] = (*c)->end; 
                    left_e_states[j] = left_end_state(*c); 
                    right_e_states[j] = right_end_state(*c); 
                    j++;  
                }
            } 
        }

        set_copy_intfc_states(YES);
        set_size_of_intfc_state(size_of_state(frs[0]->interf));
        tmp_intfc = copy_interface(frs[0]->interf);
        if (tmp_intfc == NULL)
        {
            (void) printf("ERROR in merge_fronts(), ");
            (void) printf("copy_interface() returns NULL\n");
            clean_up(ERROR);
        }
        Computational_grid(tmp_intfc) = Computational_grid(newfront->interf);
        topological_grid(tmp_intfc) = topological_grid(newfront->interf);
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        save_intrp = interpolate_intfc_states(tmp_intfc);

        set_current_interface(tmp_intfc);
        interpolate_intfc_states(tmp_intfc) = NO;
        for (i = 1; i < num_frs; i++)
        {
            copy_interface_into(frs[i]->interf,tmp_intfc);
        }
        interpolate_intfc_states(tmp_intfc) = save_intrp;

        if (debugging("merge_fronts"))
        {
            printf("Patch tmp interface after copied into\n");
            print_interface(tmp_intfc);
        }

        newfront->interf = tmp_intfc; 

        for(i = 0; i < dim; i++)
        {
            status = merge_interface(newfront,i);
            if (status == FUNCTION_FAILED)
            {
                (void) printf("ERROR in merge_fronts(), "
                      "merge_interface() failed at dir = %d\n", i);
                clean_up(ERROR);
            }
        }
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        if (amr_check_for_cut_nodes("merge_fronts()",newfront->interf) == YES)
        {
            (void) printf("WARNING in  merge_fronts()");
            (void) printf(" amr_check_for_cut_nodes() detected cut node\n");
            clean_up(ERROR);
        }
 
        if(n_nodes != 0)
        {
            for(c = newfront->interf->curves; c && *c; c++)
            {
                for(i = 0; i < n_nodes; i++)
                {
                    dist = 0.0; 
                    for(j = 0; j < dim; j++)
                        dist += fabs(Coords((*c)->start->posn)[j]-Coords(snodes[i]->posn)[j]); 
                    if(dist <= 100.0*MACH_EPS)
                    {
                        ft_assign(left_start_state(*c),left_s_states[i],
                                size_of_state(newfront->interf)); 
                        ft_assign(right_start_state(*c),right_s_states[i],
                                size_of_state(newfront->interf)); 
                    } 
                    dist = 0.0; 
                    for(j = 0; j < dim; j++)
                        dist += fabs(Coords((*c)->end->posn)[j]-Coords(enodes[i]->posn)[j]); 
                    if(dist <= 100.0*MACH_EPS)
                    {
                        ft_assign(left_end_state(*c),left_e_states[i],
                                size_of_state(newfront->interf)); 
                        ft_assign(right_end_state(*c),right_e_states[i],
                                size_of_state(newfront->interf)); 
                    } 
                }    
            } 
            free_these(2,snodes,enodes); 
            free_these(4,left_s_states,left_e_states,right_s_states,right_e_states); 
        } 
 
        debug_print("repatch","Leaving merge_fronts()\n");
        return YES;  
} 

/* If the base interface boundary nodes and the assembled
*  new interface have same nodes, remove the duplicated 
*  nodes from interface after copied the base interface into
*  the assembled interface. 
*/ 
LOCAL    void remove_dup_nodes_from_assembly_intfc(
        INTERFACE       *intfc)
{
        int             dim, i;  
        NODE            **n, **nextn;
        CURVE           **c;    
        BOND            *b;   
        NODE            **delete_nodes = NULL;
        double           dist; 
        INTERFACE       *sav_intfc; 


        DEBUG_ENTER(remove_dup_nodes_from_assembly_intfc) 

        sav_intfc = current_interface();
        set_current_interface(intfc); 
        dim = intfc->dim;  
loop_again:  
        for (n = intfc->nodes; n && *n; n++)
        {
            for(nextn = n+1; nextn && *nextn; nextn++)
            {
                dist = 0.0; 
                for(i = 0; i < dim; i++)
                {
                    dist += fabs(Coords((*n)->posn)[i]-Coords((*nextn)->posn)[i]); 
                } 
                if( dist <= MACH_EPS*1000) 
                {
                    for(c = (*nextn)->in_curves; c && *c; c++)
                         change_node_of_curve(*c,NEGATIVE_ORIENTATION,*n);  
                    for(c = (*nextn)->out_curves; c && *c; c++)
                         change_node_of_curve(*c,POSITIVE_ORIENTATION,*n); 
                    delete_node(*nextn);
                    goto loop_again;  
                }  
            }  
        } 
        set_current_interface(sav_intfc);  

        DEBUG_LEAVE(remove_dup_nodes_from_assembly_intfc) 
}  

EXPORT   void retrieve_boundary_curves_from_zero(
        Front           *newfr,
        INTERFACE       *intfc)
{
        INTERFACE       *tmp_intfc, *sav_intfc, *send_intfc;
        NODE            **n;
        CURVE           **c;
        CURVE           **delete_curves = NULL;
        NODE            **delete_nodes = NULL;
        boolean            sav_copy;
        boolean            save_intrp;


        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        set_size_of_intfc_state(size_of_state(intfc));
        set_copy_intfc_states(YES);

        tmp_intfc = copy_interface(intfc);
        if (tmp_intfc == NULL)
        {
            (void) printf("ERROR in retrieve_boundary_curves_from_zero()\n");
            (void) printf("copy_interface() returns NULL\n");
            clean_up(ERROR);
        }

        /* remove subdomain bdry curves and interior physical curves */
        delete_subdomain_curves(tmp_intfc);
        for(c = tmp_intfc->curves; c and *c; c++)
        {
            if(wave_type(*c) >= FIRST_PHYSICS_WAVE_TYPE)
            {
                if (not add_to_pointers((POINTER)*c,(POINTER**)&delete_curves))
                {
                    screen("ERROR in retrieve_boundary_curves_from_zero(), "
                        "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (c = delete_curves; c and *c; c++)
            (void) delete_curve(*c);

        delete_nodes = NULL;
        for (n = tmp_intfc->nodes; n and *n; n++)
        {
            if (((*n)->in_curves == NULL) and ((*n)->out_curves == NULL))
            {
                if (not add_to_pointers((POINTER)*n,(POINTER**)&delete_nodes))
                {
                    screen("ERROR in retrieve_boundary_curves_from_zero(), "
                              "add_to_pointers() failed\n");
                    clean_up(ERROR);
                }
            }
        }
        for (n = delete_nodes; n and *n; n++)
            (void) delete_node(*n);

        if(debugging("retrieve_boundary_curves_from_zero"))
        {
            printf("In retrieve_boundary_curves_from_zero\n");
            printf("Print retrieved tmp interface:::\n");
            print_interface(tmp_intfc);
            show_intfc_states(tmp_intfc);
        }

        set_size_of_intfc_state(size_of_state(intfc));
        set_current_interface(newfr->interf);
        save_intrp = interpolate_intfc_states(newfr->interf);
        interpolate_intfc_states(newfr->interf) = NO;
        copy_interface_into(tmp_intfc, newfr->interf);
        delete_interface(tmp_intfc); 

        interpolate_intfc_states(newfr->interf) = save_intrp;
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        remove_dup_nodes_from_assembly_intfc(newfr->interf); 

        if(debugging("retrieve_boundary_curves_from_zero"))
        {
            printf("after copy tmp_intfc into newfront, print new intfc\n");
            print_interface(newfr->interf);
            show_intfc_states(newfr->interf);
        }
        DEBUG_LEAVE(retrieve_boundary_curves_from_zero)
}  

EXPORT void send_front_misc(
        Front      *fr,
        COMPONENT  *comp,  
        int        dist)
{
        int        i, j;
        int        len;
        byte       *storage = NULL;
        byte       *buf;
        POINTER    info;

        DEBUG_ENTER(send_front_misc)
#if defined(__MPI__)

        send_interface(fr->interf, dist);

        /* fr->rect_grid */
        /* wave->patch_component */
        /* patch_number */
        /* patch_level  */
        /* rect_bdry_type[MAXD][2] */
        /* Patch_bdry_flag         */  

        len = sizeof(RECT_GRID) + sizeof(COMPONENT) + sizeof(int) +sizeof(int)
             + sizeof(int)*(fr->interf->dim)*2 + sizeof(Patch_bdry_flag);

        scalar(&storage, len);
        buf = storage;

        info = (POINTER) buf;
        ft_assign(info, fr->rect_grid, sizeof(RECT_GRID));
        buf += sizeof(RECT_GRID);

        info = (POINTER) buf;
        ft_assign(info, comp, sizeof(COMPONENT));
        buf += sizeof(COMPONENT);

        info = (POINTER) buf;
        ft_assign(info, &(fr->patch_number), sizeof(int));
        buf += sizeof(int);

        info = (POINTER) buf;
        ft_assign(info, &(fr->patch_level), sizeof(int));
        buf += sizeof(int);

        for(i = 0; i < fr->interf->dim; i++)
        {
            info = (POINTER) buf;
            ft_assign(info, &(fr->interf->rect_bdry_type[i][0]), sizeof(int));
            buf += sizeof(int);
            info = (POINTER) buf;
            ft_assign(info, &(fr->interf->rect_bdry_type[i][1]), sizeof(int));
            buf += sizeof(int);
        }

        info = (POINTER) buf;
        ft_assign(info, fr->pd_flag, sizeof(Patch_bdry_flag));
        buf += sizeof(Patch_bdry_flag);

        pp_send(0, (POINTER)storage, len, dist);

        free(storage);
#endif /* defined(__MPI__) */
        DEBUG_LEAVE(send_front_misc)
}

EXPORT void recv_front_misc(
        Front      *fr,
        COMPONENT  *comp,  
        int        source)
{
        INTERFACE  *recv_intfc;
        INTERFACE  *sav_intfc;
        boolean       sav_copy;
        boolean       save_intrp;

        int        i, j;
        int        len;
        byte       *storage = NULL;
        byte       *buf;
        POINTER    info;

        DEBUG_ENTER(recv_front_misc)
#if defined(__MPI__)

        recv_intfc = receive_interface(source);

        sav_intfc = current_interface();
        sav_copy = copy_intfc_states();
        save_intrp = interpolate_intfc_states(fr->interf);
        set_current_interface(fr->interf);
        interpolate_intfc_states(fr->interf) = NO;

        copy_interface_into(recv_intfc,fr->interf);
        (void) delete_interface(recv_intfc);

        interpolate_intfc_states(fr->interf) = save_intrp;
        set_current_interface(sav_intfc);
        set_copy_intfc_states(sav_copy);

        /* fr->rect_grid */
        /* wave->patch_component */
        /* patch_number */
        /* patch_level  */
        /* rect_boundary_type[MAXD][2] */
        /* Patch_bdry_flag             */ 

        len = sizeof(RECT_GRID) + sizeof(COMPONENT) + sizeof(int) + sizeof(int)
              + sizeof(int)*(fr->interf->dim)*2 + sizeof(Patch_bdry_flag);
        scalar(&storage, len);

        pp_recv(0, source, (POINTER)(storage), len);

        buf = storage;

        info = (POINTER) buf;
        ft_assign(fr->rect_grid, info, sizeof(RECT_GRID));
        buf += sizeof(RECT_GRID);
  
        info = (POINTER) buf;
/*  
        ft_assign(&(wave_of_front(fr)->patch_component),info,sizeof(COMPONENT));
*/ 
        ft_assign(comp,info,sizeof(COMPONENT));
        buf += sizeof(COMPONENT);
  
        info = (POINTER) buf;
        ft_assign(&(fr->patch_number),info,sizeof(int));
        buf += sizeof(int);

        info = (POINTER) buf;
        ft_assign(&(fr->patch_level),info,sizeof(int));
        buf += sizeof(int);

        for(i = 0; i < fr->rect_grid->dim; i++)
        {
            info = (POINTER) buf;
            ft_assign(&(fr->interf->rect_bdry_type[i][0]), info, sizeof(int));
            buf += sizeof(int);
            info = (POINTER) buf;
            ft_assign(&(fr->interf->rect_bdry_type[i][1]), info, sizeof(int));
            buf += sizeof(int);
        }

        info = (POINTER) buf;
        ft_assign(fr->pd_flag, info, sizeof(Patch_bdry_flag));
        buf += sizeof(Patch_bdry_flag);
 
        free(storage);
#endif /* defined(__MPI__) */
        DEBUG_LEAVE(recv_front_misc)
}


EXPORT void send_mini_front_misc(
        Front      *fr,
        COMPONENT  *comp,  
        int        dist)
{
        int        i, j;
        int        len;
        byte       *storage = NULL;
        byte       *buf;
        POINTER    info;

        DEBUG_ENTER(send_front_misc)
#if defined(__MPI__)

        /* fr->rect_grid */
        /* wave->patch_component */
        /* patch_number */
        /* patch_level  */
        /* rect_bdry_type[MAXD][2] */
        /* Patch_bdry_flag         */  

        len = sizeof(RECT_GRID) + sizeof(COMPONENT) + sizeof(int) +sizeof(int)
             + sizeof(int)*(fr->interf->dim)*2 + sizeof(Patch_bdry_flag);

        scalar(&storage, len);
        buf = storage;

        info = (POINTER) buf;
        ft_assign(info, fr->rect_grid, sizeof(RECT_GRID));
        buf += sizeof(RECT_GRID);

        info = (POINTER) buf;
        ft_assign(info, comp, sizeof(COMPONENT));
        buf += sizeof(COMPONENT);

        info = (POINTER) buf;
        ft_assign(info, &(fr->patch_number), sizeof(int));
        buf += sizeof(int);

        info = (POINTER) buf;
        ft_assign(info, &(fr->patch_level), sizeof(int));
        buf += sizeof(int);

        for(i = 0; i < fr->interf->dim; i++)
        {
            info = (POINTER) buf;
            ft_assign(info, &(fr->interf->rect_bdry_type[i][0]), sizeof(int));
            buf += sizeof(int);
            info = (POINTER) buf;
            ft_assign(info, &(fr->interf->rect_bdry_type[i][1]), sizeof(int));
            buf += sizeof(int);
        }

        info = (POINTER) buf;
        ft_assign(info, fr->pd_flag, sizeof(Patch_bdry_flag));
        buf += sizeof(Patch_bdry_flag);

        pp_send(0, (POINTER)storage, len, dist);

        free(storage);
#endif /* defined(__MPI__) */
        DEBUG_LEAVE(send_front_misc)
}

EXPORT void recv_mini_front_misc(
        Front      *fr,
        COMPONENT  *comp,  
        int        source)
{
        boolean       sav_copy;
        boolean       save_intrp;

        int        i, j;
        int        len;
        byte       *storage = NULL;
        byte       *buf;
        POINTER    info;

        DEBUG_ENTER(recv_front_misc)
#if defined(__MPI__)

        /* fr->rect_grid */
        /* wave->patch_component */
        /* patch_number */
        /* patch_level  */
        /* rect_boundary_type[MAXD][2] */
        /* Patch_bdry_flag             */ 

        len = sizeof(RECT_GRID) + sizeof(COMPONENT) + sizeof(int) + sizeof(int)
              + sizeof(int)*(fr->interf->dim)*2 + sizeof(Patch_bdry_flag);
        scalar(&storage, len);

        pp_recv(0, source, (POINTER)(storage), len);

        buf = storage;

        info = (POINTER) buf;
        ft_assign(fr->rect_grid, info, sizeof(RECT_GRID));
        buf += sizeof(RECT_GRID);
  
        info = (POINTER) buf;
        ft_assign(comp,info,sizeof(COMPONENT));
        buf += sizeof(COMPONENT);
  
        info = (POINTER) buf;
        ft_assign(&(fr->patch_number),info,sizeof(int));
        buf += sizeof(int);

        info = (POINTER) buf;
        ft_assign(&(fr->patch_level),info,sizeof(int));
        buf += sizeof(int);

        for(i = 0; i < fr->rect_grid->dim; i++)
        {
            info = (POINTER) buf;
            ft_assign(&(fr->interf->rect_bdry_type[i][0]), info, sizeof(int));
            buf += sizeof(int);
            info = (POINTER) buf;
            ft_assign(&(fr->interf->rect_bdry_type[i][1]), info, sizeof(int));
            buf += sizeof(int);
        }

        info = (POINTER) buf;
        ft_assign(fr->pd_flag, info, sizeof(Patch_bdry_flag));
        buf += sizeof(Patch_bdry_flag);
 
        free(storage);
#endif /* defined(__MPI__) */
        DEBUG_LEAVE(recv_front_misc)
}


#endif  /* if defined(USE_OVERTURE) */

