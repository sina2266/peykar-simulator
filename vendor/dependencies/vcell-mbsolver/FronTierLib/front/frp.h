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
*				frp.h
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains structures related to the solution of two dimensional
*	Riemann problems.
*/

#if !defined(_FRP_H)
#define _FRP_H

#include <front/fdecs.h>

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

struct  _RP_NODE {
	NODE *node;
	NODE *old_node;		/* Corresponding node in fr->interf */
	int states_assigned_at_node;
	struct _RP_NODE *prev,*next;
	struct _O_CURVE	*neumann1,*neumann2,*dirichlet1,*dirichlet2,
			*subdomain1,*subdomain2;
	struct _O_CURVE_FAMILY *_reflected1;
	struct _O_CURVE_FAMILY *_reflected2;
};
typedef struct _RP_NODE RP_NODE;

#define rp_node(rpn)  ((RP_NODE *) (rpn))
#define rpn_reflected1(rpn)     (rp_node(rpn)->_reflected1)
#define rpn_reflected2(rpn)     (rp_node(rpn)->_reflected2)

struct _RPROBLEM {
		/* time zero data */

	INTERFACE	*new_intfc;	/* newfr->interf */

		/* in data */

	INTERFACE	*old_intfc;	/* oldfr->interf */
	struct _Front	*fr;		/* oldfr */
	POINTER		wave;
	double		dt;
	double		dt_frac;
	struct _RP_NODE *first_rp_node;
	struct _RP_NODE *last_rp_node;

		/* Curves in the rproblem */

	struct _O_CURVE_FAMILY *bdry_curves;
	struct _O_CURVE_FAMILY *old_bdry_curves;
	struct _O_CURVE_FAMILY *ang_ordered_curves;
	struct _O_CURVE_FAMILY *old_ang_ordered_curves;

		/* global information */

	struct _RPROBLEM	*next;
	struct _RPROBLEM	*prev;

		/* Statistical Information */
	
	int bdry_type1, bdry_type2;
	int num_nd, num_fxd, num_srce;
	int num_nod, num_bdry_nod, num_phys;
	int num_ang_ordered_curves;

};
typedef struct _RPROBLEM	RPROBLEM;

struct _F_USER_RPROBLEM {
	int size_rproblem;
	int size_rp_node;

	void (*_init_rp_nodes)(RPROBLEM*);
	void (*_delete_curve_from_rp_node)(CURVE*,RP_NODE*,RPROBLEM*);
	void (*_free_rp_node)(RP_NODE*,RPROBLEM*);
	void (*_user_free_rp_node)(RP_NODE*,RPROBLEM*);
	void (*_print_rp_node)(RP_NODE*,RPROBLEM*);
	void (*_user_print_rp_node)(RP_NODE*,RPROBLEM*);
	void (*_print_rproblem)(RPROBLEM*);
	void (*_user_print_rproblem)(RPROBLEM*);
	void (*_set_rp_statistics)(RPROBLEM*);
	void (*_set_phys_ocurves_to_null)(RP_NODE*,RPROBLEM*);
};
typedef struct _F_USER_RPROBLEM F_USER_RPROBLEM;

struct _F_RPROBLEM {
	RPROBLEM rproblem;
	F_USER_RPROBLEM f_user_rp;
	int _num_incident;
	int _num_refl;
};
typedef struct _F_RPROBLEM F_RPROBLEM;

#define f_rproblem(rp)	((F_RPROBLEM *) (rp))
#define rp_num_incident(rp)             (f_rproblem(rp)->_num_incident)
#define rp_num_refl(rp)                 (f_rproblem(rp)->_num_refl)
#define f_user_rproblem(rp)	(f_rproblem(rp)->f_user_rp)


#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif /* defined(_FRP_H) */
