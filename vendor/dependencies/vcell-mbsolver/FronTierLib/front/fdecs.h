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
*				fdecs.h:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains declarations of variables related to the front.  A front
*	consists of an interface (see int.c for documentation) and
*	and the values of the physical state variables on each side of the
*	interface, together with further variables which specify algorithms for
*	the processing of fronts.
*/

#if !defined(_FDECS_H)
#define _FDECS_H

#include <intfc/int.h>


#define LOCSTATE
typedef POINTER Locstate;

#include <front/frp.h>
#include <front/fvelo.h>
#if defined(USE_HDF)
#include <hdf.h>
#include <mfhdf.h>
#endif /* defined(USE_HDF) */

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif



		/* Possible values for fr->hyperbolic */

enum {
	NO_STATES   = 0,
	FULL_STATES = 1
};

		/* Possible values returned by node_propagate */

enum {
	GOOD_NODE = 1,
	NO_STORAGE_NODE,
	ERROR_NODE,
	PSEUDOCROSS_NODE_NODE,
	CROSS_NODE_NODE,
	PSEUDOCROSS_CURVE_NODE,
	CROSS_CURVE_NODE,
	BIFURCATION_NODE,
	CROSS_PAST_CURVE_NODE,
	NO_CROSS_NODE,
	MODIFY_TIME_STEP_NODE,
	REPEAT_TIME_STEP_NODE
};

enum {
                GOOD_CURVE = 1
};

		/* Possible values returned by redistribute() */

enum {
	BAD_REDISTRIBUTION	      = 0x0,
	GOOD_REDISTRIBUTION	      = 0x1,
	UNABLE_TO_UNTANGLE	      = 0x2,
	MODIFY_TIME_STEP_REDISTRIBUTE = 0x4,
	INCONSISTENT_RECONSTRUCTION   = 0x5
};

		/* Possible values returned by fr_bdry_untangle */

enum {
	ERROR_IN_UNTANGLE			      = 0,
	CURVES_UNTANGLED			      = 1,
	MODIFY_TIME_STEP_TO_UNTANGLE		      = 2,
	MODIFY_TIME_STEP_TO_UNTANGLE_BUT_FORCE_TANGLE = 3
};

		/* values for loops in untangle */
enum {
	UNPHYSICAL = 0,
	PHYSICAL   = 1
};

		/* Possible values for node propagation flag */

#if !defined(MAX_N_NODE_FLAGS)
#define MAX_N_NODE_FLAGS 64
#endif /* !defined(MAX_N_NODE_FLAGS) */

struct _NODE_FLAG {
	boolean _node_flags[MAX_N_NODE_FLAGS];
};
typedef struct _NODE_FLAG NODE_FLAG;

enum _F_NODE_FLAG_INDEX {
	_TO_NEXT_NODE_ONLY_INDEX	      = 0,
	_CONTINUE_PAST_FIXED_NODE_INDEX,
	_CONTINUE_PAST_BIFURCATION_INDEX,
	_POSITION_ONLY_INDEX,
	_PHYS_VIRTUALS_PRESET_INDEX,
	_P_VIRTUALS_SET_FROM_NODE_INDEX,
	_NODE_VELOCITY_PRESET_INDEX,
	_SET_VIRTUALS_BY_ADJACENT_BOND_INDEX,
	_DONT_INSERT_ON_H_EXTEND_INDEX,
	_CONTINUE_PAST_BOUNDARY_INDEX,
	_ORDINARY_CROSS_ONLY_INDEX,
	_SINGLE_EXTEND_TO_CROSS_INDEX,
	_DOUBLE_EXTEND_TO_CROSS_INDEX,
	_FIRST_PHYSICS_NODE_FLAG_INDEX
};

#define clear_node_flag(flag)	zero_scalar(&flag,sizeof(NODE_FLAG))
#define set_to_next_node_only(flag)					\
     (clear_node_flag(flag), to_next_node_only(flag) = YES)
#define set_ordinary_cross_only(flag)					\
     clear_node_flag(flag)
#define to_next_node_only(flag)  					\
    (flag)._node_flags[_TO_NEXT_NODE_ONLY_INDEX]
#define continue_past_fixed_node(flag)					\
    (flag)._node_flags[_CONTINUE_PAST_FIXED_NODE_INDEX]
#define continue_past_bifurcation(flag)					\
    (flag)._node_flags[_CONTINUE_PAST_BIFURCATION_INDEX]
#define position_only(flag)						\
    (flag)._node_flags[_POSITION_ONLY_INDEX]
#define phys_virtuals_preset(flag)					\
    (flag)._node_flags[_PHYS_VIRTUALS_PRESET_INDEX]
#define p_virtuals_set_from_node(flag)					\
    (flag)._node_flags[_P_VIRTUALS_SET_FROM_NODE_INDEX]
#define node_velocity_preset(flag)					\
    (flag)._node_flags[_NODE_VELOCITY_PRESET_INDEX]
#define set_virtuals_by_adjacent_bond(flag)				\
    (flag)._node_flags[_SET_VIRTUALS_BY_ADJACENT_BOND_INDEX]
#define dont_insert_on_h_extend(flag)					\
    (flag)._node_flags[_DONT_INSERT_ON_H_EXTEND_INDEX]
#define continue_past_boundary(flag)					\
    (flag)._node_flags[_CONTINUE_PAST_BOUNDARY_INDEX]
#define single_extend_to_cross(flag)					\
    (flag)._node_flags[_SINGLE_EXTEND_TO_CROSS_INDEX]
#define double_extend_to_cross(flag)					\
    (flag)._node_flags[_DOUBLE_EXTEND_TO_CROSS_INDEX]


	/* Flags returned by check_cross() and is_good_cross() */

enum {
	NO_CROSS	    = 0x0,
	GOOD_CROSS	    = 0x1,
	END_OF_CURVE_CROSS  = 0x2,
	OUT_OF_BOUNDS_CROSS = 0x4,
	FOLD_BACK_CROSS	    = 0x8
};

		/* Possible values for flag in untangle_front */

enum {
	NORMAL_ATTEMPT_TO_UNTANGLE    = 1,
	DIFFICULT_ATTEMPT_TO_UNTANGLE,
	LAST_ATTEMPT_TO_UNTANGLE
};

		/* Integer flags for type of curvature at a point on a curve */

enum {
	PINCHED	= 1, /* Infinite curvature of Front */
	FLAT,	     /* No Curvature of Front */
	CURVED	     /* Finite, Non-zero Curvature */
};


		/* Classes distinguished by redistribute */

enum {
	GENERAL_WAVE = 0,
	VECTOR_WAVE,
	GENERAL_NODE
};

struct _SAMPLE {
	int start_step;
	int end_step;
	int step_interval;
	char sample_name[100];
	char sample_type[100];
	double sample_coords[MAXD];
};
typedef struct _SAMPLE SAMPLE;

		/* Tracking algorithm */

enum _TRACKING_ALGORITHM {
	NO_DYNAMIC_TRACKING,
	GRID_FREE_TRACKING,
	GRID_BASED_TRACKING,
	THREE_COMP_GRID_BASED_TRACKING,
	MIXED_TRACKING,
	HYBRID_TRACKING,
	LOCALLY_GRID_BASED_TRACKING
};
typedef enum _TRACKING_ALGORITHM TRACKING_ALGORITHM;

		/* Typedefs */

struct _Nor_stencil {
	int npts;
	double **pts;
	double nor[MAXD];
	double curvature;
	COMPONENT comp;
};
typedef struct _Nor_stencil Nor_stencil;

struct _Tan_stencil {
	int                npts;
	FT_POINT	 	   **p,		**pstore;
	HYPER_SURF_ELEMENT **hse,	**hsestore;
	HYPER_SURF	   **hs,	**hsstore;
	Locstate	   *leftst,	*leftststore;
	Locstate	   *rightst,	*rightststore;
	double	 	   *t,		*tstore;
	double		   curvature; /*Mean curvature*/
	COMPONENT          comp;
	HYPER_SURF         *newhs;
	Locstate           *states;
	const double        *dir;
        const double        *dir1;
        double              *nor;
};
typedef struct _Tan_stencil Tan_stencil;

typedef struct {
	TRI     *tri; 
	BOND    *b; 
	boolean    is_vertex;
	int     side;
	int     iv;
	double   pc[3];
} TN;

typedef struct {
	HYPER_SURF *hs;
	FT_POINT      *p;
	double      *nor;
	double      tan[3]; 
	double      plane[4]; 
	double      ds;
	double      dt;
	TN         tnl, tnr;
} Tparams;

struct _TSTEP_CONTROL {
	double	time_step_factor;
	boolean	apply_cfl_at_nodes;
	double	max_sep;
	double	cfl_fudge;
	double	frac_floor;
	double	frac_ceil;
};
typedef struct _TSTEP_CONTROL TSTEP_CONTROL;

struct _AFLIN {
	double a[MAXD][MAXD];
	double b[MAXD];
	double det;
} ;
typedef struct _AFLIN AFLIN;

struct _NNLIST {
	struct _NNLIST	*prev, *next;
	CURVE		*nc[4];
	NODE    	*nn[4], *m;
	COMPONENT 	comp[4];
	double		area[4];
	ORIENTATION	orient[4];
	int     	ni[4];
	int		num_phys;
};
typedef struct _NNLIST NNLIST;

enum _REDISTRIBUTION_MODE {
	NO_REDIST        = 0,
	EXPANSION_REDIST = 1,
	FULL_REDIST      = 2
};
typedef enum _REDISTRIBUTION_MODE REDISTRIBUTION_MODE;

enum _REDISTRIBUTION_VERSION {
	ORDINARY_REDISTRIBUTE  = 1,
	EQUI_BOND_REDISTRIBUTE = 2
};
typedef enum _REDISTRIBUTION_VERSION REDISTRIBUTION_VERSION;

struct _CURVE_REDIST_OPTIONS {
	REDISTRIBUTION_VERSION _full_curve_redist_version; /* Prompted */
	boolean _rect_bdry_redist;          /* Prompted */
	double   _cos_big_angle[2];          /* Prompted */
};
typedef struct _CURVE_REDIST_OPTIONS CURVE_REDIST_OPTIONS;

struct _SURFACE_REDIST_OPTIONS {
	double   _max_tri_area_factor[2];     /* Prompted */
	double   _min_tri_area_factor[2];     /* Prompted */
	double   _min_angle_at_tri_vertex[2]; /* Prompted */
	double   _max_scaled_tri_side_len; /* Prompted */
};
typedef struct _SURFACE_REDIST_OPTIONS SURFACE_REDIST_OPTIONS;


struct _REDIST_CONTROL {

	REDISTRIBUTION_MODE _Redistribution_mode;
	CURVE_REDIST_OPTIONS _CurveRedistributionOptions;
	SURFACE_REDIST_OPTIONS _SurfaceRedistributionOptions;

	/* Redistribution initializer */
	void (*_init_redistribute)(INIT_DATA*,struct _Front*);
	void (*_rect_boundary_redistribute)(INTERFACE*,RECT_GRID*,int);

	struct _CURVE_REDISTRIBUTE {
	    struct _CURVE_REDISTRIBUTORS {
	    	/* Redistribute Algorithm for curves*/
	    	boolean (*curve_redist_func)(struct _Front*,boolean*);

	    	/* Forward orient curve redistribute */
	    	boolean (*forward_cur_redist)(struct _Front*,CURVE*,boolean);

	    	/* Backward orient curve redistribute */
	    	boolean (*backward_cur_redist)(struct _Front*,CURVE*,boolean);

	    	/* Node redistribute */
 	    	boolean (*node_redistribute)(struct _Front*,boolean);
	    } Redistributors;

		/* Clean up function for interface */
	    struct _CURVE_CLEANERS {
	    	void	(*_delete_small_loops)(struct _Front*);
	    	void	(*_delete_fold_back_bonds)(struct _Front*,CURVE*,double,
                                                   int*,int*);
	    	void	(*_delete_exterior_curves)(struct _Front*,INTERFACE*);
	    	boolean	(*_delete_phys_remn_on_bdry)(struct _Front*);
	    	boolean (*_delete_point_adjacent_to_node)(struct _Front*,
                                                          CURVE*,ORIENTATION);
	    } Cleaners;

			/* Grid based redist. of rect bdry */
	    boolean rect_bdry_redist;
	    	/* Big Angles not Truncated in Redis */
	    double cos_big_angle[2];
	    	/* Arc Length along the Front */
	    double length;
	} Curve_redist;

	struct _SURFACE_REDISTRIBUTE {
	    /* Redistribute Algorithm for surfaces*/
	    boolean (*surface_redist_func)(struct _Front*,boolean*);
	    	/* maximum length for bond in 3d curve */
	    double max_len[2];
	    	/* minimum length for bond in 3d curve */
	    double min_len[2];
	    	/* square of maximum area allowed for triangle */
	    double max_area2[2];
	    	/* square of minimum area allowed for triangle */
	    double min_area2[2];
	    	/* linear tolerance of aspect ratio     */
	    double max_sqr_tri_len;
	    	/* square of maximum scaled triangle side length */
	    double ar_tol[2];


	} Surface_redist;

	double grid_size_redist;         /*Nmber of grid size to redistribute*/
	int count_redist;		/* Number of calls to Redistribute */
	int freq_redist[3];		/* Frequency for Redistribution */
	int freq_reconstruct;		/* Frequency for reconstruction */
					/* (for hybrid tracking) */
	double spacing[2];		/* Spacing of Points on 2d Front */
	boolean intfc_redistributed;	/* Yes if redistribute was performed */
	TRACKING_ALGORITHM tracking_algorithm;	/* method of tracking */
};
typedef struct _REDIST_CONTROL REDIST_CONTROL;

#if defined(__cplusplus)

typedef struct _REDIST_CONTROL::_CURVE_REDISTRIBUTE CURVE_REDISTRIBUTE;
typedef struct _REDIST_CONTROL::_CURVE_REDISTRIBUTE::_CURVE_REDISTRIBUTORS CURVE_REDISTRIBUTORS;
typedef struct _REDIST_CONTROL::_CURVE_REDISTRIBUTE::_CURVE_CLEANERS CURVE_CLEANERS;
typedef struct _REDIST_CONTROL::_SURFACE_REDISTRIBUTE SURFACE_REDISTRIBUTE;

#else /* defined(__cplusplus) */

typedef struct _CURVE_REDISTRIBUTE CURVE_REDISTRIBUTE;
typedef struct _CURVE_REDISTRIBUTORS CURVE_REDISTRIBUTORS;
typedef struct _CURVE_CLEANERS CURVE_CLEANERS;
typedef struct _SURFACE_REDISTRIBUTE SURFACE_REDISTRIBUTE;

#endif /* defined(__cplusplus) */

struct _UNTRACK_FLAG {
	boolean start_states_set;
	boolean end_states_set;
	int user_untrack_flag;
};
typedef struct _UNTRACK_FLAG UNTRACK_FLAG;

struct _HDF_MOVIE_VAR {
	boolean plot_bullet;
	boolean plot_comp;
	int resolution;
	int num_var;
	int *idir;
	char **var_name;
	double **top_var;
	boolean *use_preset_bounds;
	double *var_min;
	double *var_max;
	COMPONENT *obstacle_comp;
	double (*get_state_var[20])(Locstate);
};
typedef struct _HDF_MOVIE_VAR HDF_MOVIE_VAR;

struct _VTK_MOVIE_VAR {
	int num_var;
	char **var_name;
	double ***top_var;
};
typedef struct _VTK_MOVIE_VAR VTK_MOVIE_VAR;

struct _Front {
		/* Grid Specification */
	RECT_GRID *rect_grid;		/* Grid Info */
	PP_GRID* pp_grid;

		/* advancing the front */
	void	(*_pre_advance_front)(struct _Front*);
	int	(*_advance_front)(double,double*,struct _Front*,
				  struct _Front**,POINTER);

		/* Copy and freeing front */
	void	(*_free_front)(struct _Front*);
	void	(*_copy_into_front)(struct _Front*,struct _Front*);
	struct _Front*	(*_copy_front)(struct _Front*);

		/* printing the front */
	void	(*_print_Front_structure)(struct _Front*);
	void	(*_fprint_front)(struct _Front*,FILE*);
	void	(*_read_print_front)(INIT_DATA*,struct _Front*);

		/* State Variable Specification */
	size_t  sizest;			/* Size of State Variables in Bytes */
	void (*_hyp_solution)(double*,COMPONENT,HYPER_SURF*,SIDE,struct _Front*,
					POINTER,Locstate,Locstate);
	void (*_hyp_grad_solution)(double*,COMPONENT,HYPER_SURF*,SIDE,
					struct _Front*,POINTER,Locstate*);
	void (*_state_interpolator)(double,double,double*,Locstate,double*,
				       	Locstate,RECT_GRID*,Locstate);
	boolean (*_tri_state_interpolator)(double,double,double,double*,Locstate,
				   	double*,Locstate,double*,Locstate,
					RECT_GRID*,Locstate);
	void (*transform_state)(Locstate,AFLIN*); /* Coordinate transforms
						     on states */

		/* Hyper surface corresponder */
	boolean (*_is_correspondence_possible)(HYPER_SURF*,HYPER_SURF*,
					       HYPER_SURF_BDRY**,
					       HYPER_SURF_BDRY**);

		/* Redistribution Specification */
	REDIST_CONTROL  Redist;         /* Redistribution control parameters */
	boolean Auto_Redist;		/* Redistribute when need is detected */

		/* Time Step Selection */
	boolean (*_last_time_step_modification)(void);
					/* Computes front maximum time step */
	double (*max_front_time_step)(struct _Front*,double*);
	struct	_MAX_FRONT_SPEED	*_MaxFrontSpeed;
	TSTEP_CONTROL	Tstep;

		/* Real and mesh times for front->interf */
	double dt, *dt_frac, time, max_time;
	double print_time_interval, movie_frame_interval,resolution_level;
	boolean is_print_time,is_movie_time,time_limit_reached;
	int im,ip;
	int step, max_step;
	int num_mts,_max_num_mts;
	boolean   redis_flag; /*flag for the redistribution after LGB*/
	double distance, dismin, dismax, hmin_fre, hmax_fre;
	boolean print_sdl_file;
        boolean tan_sec, parab;
        double subgrid_time;

		/* Advancing the Front */
	int  hyperbolic;
	int  npts_tan_sten;
	int  movingframe;
	boolean adaptive_partition;     /* Use adaptive partition if YES */
	POINTER vparams;	/* parameters for velocity function */
	POINTER extra1;		/* pointer to extra data structure */
	POINTER extra2;		/* pointer to extra data structure */
	char *out_name;		/* Directory name of output files */
	SAMPLE *sample;
	COMPONENT *hdf_comps[MAXD];	/* Saved for hdf plotting */
	HDF_MOVIE_VAR *hdf_movie_var;	/* variables for hdf movies */
	VTK_MOVIE_VAR *vtk_movie_var;	/* variables for vtk movies */
	int  (*init_topology_of_new_interface)(struct _Front*,struct _Front*);
	struct _F_WAVE_CAPTURE *_f_wave_capture;
	void (*_init_propagate)(struct _Front*);
	void (*intfc_propagate)(struct _Front*,POINTER,INTERFACE*,INTERFACE*,
                                double);
	void (*curve_propagate)(struct _Front*,POINTER,CURVE*,CURVE*,double);
	int  (*node_propagate)(struct _Front*,POINTER,NODE*,NODE*,RPROBLEM**,
			       double,double*,NODE_FLAG,POINTER);
	void (*_point_propagate)(struct _Front*,POINTER,FT_POINT*,FT_POINT*,
			        HYPER_SURF_ELEMENT*,HYPER_SURF*,double,double*);
	void (*_point_propagate_along_wall)(struct _Front*,POINTER,FT_POINT*,
				BOND*,CURVE*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
				FT_POINT*,double,double*);
	void (*bond_propagate)(struct _Front*,POINTER,BOND*,BOND*,
			       CURVE*,double);
	int  (*snd_node_propagate)(struct _Front*,struct _Front*,POINTER,
				INTERFACE*,NODE*,NODE*,double);
	void (*tan_curve_propagate)(struct _Front*,struct _Front*,INTERFACE*,
				CURVE*,CURVE*,double);
	void (*tan_surface_propagate)(struct _Front*,struct _Front*,INTERFACE*,
				SURFACE*,SURFACE*,double);
	boolean (*_tan_point_propagate)(struct _Front*,FT_POINT*,FT_POINT*,
				HYPER_SURF_ELEMENT*,HYPER_SURF*,double,int);
	void (*_npt_tang_solver)(double,double,Tan_stencil*,
	                        Locstate,Locstate,struct _Front*);
	void (*_one_side_npt_tang_solver)(double,double,Tan_stencil*,Locstate,
				struct _Front*);
        void (*_npt_parab_tan_solver2d)(double,double,Tan_stencil*,
                                Locstate,Locstate,struct _Front*);
        void (*_npt_parab_tan_solver3d)(struct _Front*,const Tparams*,
                                const Tparams*,FT_POINT*);
	void (*impose_bc)(FT_POINT*,BOND*,CURVE*,double*,struct _Front*,
			  boolean,boolean);
	int  (*vfunc)(POINTER,struct _Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
				HYPER_SURF*,double*); /* analytical velo func */
	void (*_compute_force_and_torque)(struct _Front*,HYPER_SURF*,double,
				double*,double*);
	boolean (*_untrack_surface)(SURFACE*,COMPONENT,struct _Front*);
	boolean (*_reconstruct_front_at_grid_crossing)(struct _Front*);
	boolean (*_repair_front_at_grid_crossing)(struct _Front*);
	void (*_principal_tangent)(FT_POINT*,HYPER_SURF_ELEMENT*,HYPER_SURF*,
				double*,double*);
	void (*surface_propagate)(struct _Front*,struct _Front*,POINTER,
				double,double*);

		/* Untangling the Front */
	boolean (*_check_delete_redundant_node)(NODE*,CURVE*,CURVE*);
	int  (*fr_bdry_untangle)(struct _Front*,CROSS**,RPROBLEM*,NODE*,int);
	int  (*fr_vec_bdry_untangle)(CURVE*,CURVE*,CURVE**,ORIENTATION,
				ANGLE_DIRECTION,int,struct _Front*);
	int  (*untangle_front)(struct _Front*,CROSS**,int);
	int  (*grid_based_untangle)(struct _Front*,CROSS**);
	int  (*elastic_untangle)(struct _Front*,CROSS**);
	boolean (*_replace_unphys_loop)(NNLIST*,NNLIST**,CURVE**,
				struct _Front*,int,double,int);
	int  (*B_node_bifurcation)(struct _Front*,POINTER,O_CURVE*,O_CURVE*,
				O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,O_CURVE*,
				O_CURVE*,O_CURVE*,O_CURVE*,FT_POINT*,Locstate,
				Locstate,ANGLE_DIRECTION,RPROBLEM**,
				double,double*,NODE_FLAG);
	int  (*twodrproblem)(struct _Front*,struct _Front*,POINTER,RPROBLEM**);
	boolean (*_untrack_curve)(O_CURVE*,O_CURVE*,COMPONENT,double,
			        struct _Front*,POINTER,RPROBLEM*,
				UNTRACK_FLAG);
	boolean (*_untrack_point)(FT_POINT*,COMPONENT,struct _Front*);
	void (*identify_physical_node)(NODE*);
	void (*init_2drproblem)(RPROBLEM*,struct _Front*);
	void (*phys_split_bdry_cross)(CURVE**,CURVE**);
	void (*phys_set_node_types)(struct _Front*);
	void (*parallel_refl_vec_wave)(CURVE*,int,int,CURVE*,int,int,
				RPROBLEM*,struct _Front*,POINTER);

		/* Identification of boundary states and components */

	int  (*neumann_bdry_state)(double*,COMPONENT,FT_POINT*,HYPER_SURF*,
				struct _Front*,POINTER,Locstate);
	ANGLE_DIRECTION  (*_find_i_to_prop_dir)(struct _Front*,POINTER,NODE*,
				CURVE*,ORIENTATION,double,COMPONENT*,FT_POINT*,
				double*);
	int  (*is_nzn_bdry)(double,double,COMPONENT,CURVE*);

		/*Locstate allocation and clearing*/

	void (*_alloc_state)(Locstate*,size_t);
	void (*_clear_state)(Locstate,size_t);
	void (*_obstacle_state)(Locstate,size_t);

		/* Output printout functions */

	int nfloats;			/* Number of floats in Locstate */
	void (*print_state)(Locstate);
	void (*_fgraph_front_states)(FILE*,struct _Front*);
	void (*_fprint_header_for_graph_curve_states)(FILE*,struct _Front*,
				const char*);
	void (*_fgraph_curve_states)(FILE*,CURVE*,struct _Front*,double*);
	void (*_check_front_state_consistency)(struct _Front*);
	void (*mass_consv_diagn_driver)(struct _Front*,POINTER,double);

	struct	_FlowSpecifiedRegion *head_fsr;

	void (*_EnforceFlowSpecifedStates)(struct _Front*);

		/* Marker for Copy Routine */
	int FDIVIDER;

		/* (the rest of the front consists of pointers) */

	INTERFACE *interf;		/* Interface */
	INTERFACE *grid_intfc;		/* Grid Interface */
	INTERFACE *emb_grid_intfc;	/* Grid Interface for embedded bdry */

#if defined(USE_OVERTURE)

        int     (*_normal_advance_front)(double,double*,struct _Front*,
                                struct _Front**,POINTER);
        int     (*_tangnt_advance_front)(double,double*,struct _Front*,
                                struct _Front**,POINTER);
        int     (*_redist_advance_front)(double,double*,struct _Front*,
                                struct _Front**,POINTER);

                /* deep Copy and freeing front, create rect_grid */
        void    (*_deep_free_front)(struct _Front*);
        struct _Front*  (*_deep_copy_front)(struct _Front*);

        int                       use_overture_state;
        POINTER                   cg_over;   /* pointer of CompositeGrid */
        int                       patch_number;
        int                       patch_level;
        int                       NumberOfLevels;
        int                       totalNumberOfPatches;
/*         Neighbor                        *neighbor; */
        Patch_bdry_flag           *pd_flag;
        struct _Front             *newfront;
#endif /* defined(USE_OVERTURE) */
};
typedef struct _Front Front;

#if defined(USE_OVERTURE)
struct _Wv_on_pc{
        int    pc_id;   /* proc id */
        int    pc_ic[MAXD]; /* proc icrds */
        int    wv_id;   /* patch of wv id */
        int    wv_level; /* for the reduce of communication */
        int    base[MAXD];  /* grid location index on subdomain */
        int    bound[MAXD]; /* grid location index on subdomain */
        int    off_set[MAXD]; /* location of index[0,0] offset  */
        Front  *front;
};
typedef struct _Wv_on_pc Wv_on_pc;
#endif /* if defined(USE_OVERTURE) */

#define	npt_tang_solver(ds,dt,sten,ansl,ansr,fr)			\
	(*(fr)->_npt_tang_solver)(ds,dt,sten,ansl,ansr,fr)

#define	one_side_npt_tang_solver(ds,dt,sten,ans,fr)			\
	(*(fr)->_one_side_npt_tang_solver)(ds,dt,sten,ans,fr)

#define npt_parab_tan_solver2d(ds,dt,sten,ansl,ansr,fr)                        \
        (*(fr)->_npt_parab_tan_solver2d)(ds,dt,sten,ansl,ansr,fr)

#define npt_parab_tan_solver3d(fr,tp,tp1,newp)                        \
        (*(fr)->_npt_parab_tan_solver3d)(fr,tp,tp1,newp)

#define	f_front(front)	((Front*)front)

	/*
	*  Data structure for keeping tracking of the maximum wave speed
	*  on the tracked front.
	*/

struct	_MAX_FRONT_SPEED {
	double		_spfr[MAXD+1];	/* Max front speeds in coord dirs    */
					/* spfr[dim+1] = speed in prop. dirs */
	double		**_coords;	/* Location of maximum front speeds  */
	Locstate	*_mxspst;	/* Copy of state which set the       */
					/* maxium front speed.		     */
	size_t		_sizest;
	struct _MAX_FRONT_SPEED_OPERATORS {
	    void	(*_set)(int,double,Locstate,double*,Front*);
	    void	(*_include)(struct _MAX_FRONT_SPEED*,Front*);
	    void	(*_initialize)(Front*);
	    void	(*_print)(FILE*,Front*);
	    boolean	(*_read_print)(INIT_DATA*,const IO_TYPE*,Front*,
				       struct _MAX_FRONT_SPEED*);
	    void	(*_copy)(struct _MAX_FRONT_SPEED*,
				 struct _MAX_FRONT_SPEED*,Front*);
	    void	(*_destroy)(Front*);
	} operators;
};
typedef struct _MAX_FRONT_SPEED MAX_FRONT_SPEED;
#if defined(__cplusplus)
typedef struct _MAX_FRONT_SPEED::_MAX_FRONT_SPEED_OPERATORS MAX_FRONT_SPEED_OPERATORS;
#else /* defined(__cplusplus) */
typedef struct _MAX_FRONT_SPEED_OPERATORS MAX_FRONT_SPEED_OPERATORS;
#endif /* defined(__cplusplus) */

#define	MaxFrontSpeed(fr)	(fr)->_MaxFrontSpeed
#define	Spfr(fr)		MaxFrontSpeed(fr)->_spfr
#define	MaxFrontSpeedState(fr)	MaxFrontSpeed(fr)->_mxspst
#define	MaxFrontSpeedCoords(fr)	MaxFrontSpeed(fr)->_coords
#define	MaxFrontNumModStep(fr)	MaxFrontSpeed(fr)->_max_num_mts

#define	MaxFrontSpeedOperators(fr)	MaxFrontSpeed(fr)->operators
#define	SetMaxFrontSpeed(fr)		MaxFrontSpeedOperators(fr)._set
#define	IncludeMaxFrontSpeedInfo(fr)	MaxFrontSpeedOperators(fr)._include
#define	InitializeMaxFrontSpeed(fr)	MaxFrontSpeedOperators(fr)._initialize
#define	PrintMaxFrontSpeedInfo(fr)	MaxFrontSpeedOperators(fr)._print
#define	ReadPrintMaxFrontSpeedInfo(fr)	MaxFrontSpeedOperators(fr)._read_print
#define	CopyMaxFrontSpeed(fr)		MaxFrontSpeedOperators(fr)._copy
#define	DestroyMaxFrontSpeed(fr)	MaxFrontSpeedOperators(fr)._destroy

#define	set_max_front_speed(i,spd,st,crds,fr)				\
		(*SetMaxFrontSpeed(fr))(i,spd,st,crds,fr)
#define	include_max_front_speedInfo(mxsp,fr)				\
		(*IncludeMaxFrontSpeedInfo(fr))(mxsp,fr)
#define	initialize_max_front_speed(fr)					\
		(*InitializeMaxFrontSpeed(fr))(fr)
#define	print_max_front_speed_info(file,fr)				\
		(*PrintMaxFrontSpeedInfo(fr))(file,fr)
#define	read_print_max_front_speed_info(init,io_type,fr,mfs)		\
		(*ReadPrintMaxFrontSpeedInfo(fr))(init,io_type,fr,mfs)
#define	copy_max_front_speed(nmxsp,omxsp,fr)				\
		(*CopyMaxFrontSpeed(fr))(nmxsp,omxsp,fr)
#define	destroy_max_front_speed(fr)					\
		(*DestroyMaxFrontSpeed(fr))(fr)

struct _F_WAVE_CAPTURE {
	void (*_wave_capture)(Front*);
};
typedef struct _F_WAVE_CAPTURE F_WAVE_CAPTURE;

#define	f_wave_capture(fr)	(fr)->_f_wave_capture
#define capture_waves(fr)						\
	if ((f_wave_capture(fr) != NULL) 	&& 			\
			(f_wave_capture(fr)->_wave_capture != NULL))	\
		(*f_wave_capture(fr)->_wave_capture)(fr)


	/*
	 *  Data structures for specifying a region where the flow
	 *  is specified by a user defined function.
	 */

struct	_FlowSpecifiedRegion {

	COMPONENT	comp;	/*Component number that identifies region*/
	char	        type[100];/*String that identifies type of region*/
	struct _FlowSpecifiedRegion *next, *prev;
	struct _FlowSpecifiedRegion *head, *tail;

	boolean (*_ComponentsMatch)(COMPONENT,COMPONENT,INTERFACE*);
	boolean (*_SetFlowSpecifiedState)(Locstate,Locstate,double*,COMPONENT,
					  struct _FlowSpecifiedRegion*,Front*);
	void (*_fprint_FlowSpecifiedRegion_data)(FILE*,
					 struct _FlowSpecifiedRegion*,Front*);
	void (*_DestroyFlowSpecifiedRegion)(struct  _FlowSpecifiedRegion*);
};
typedef struct _FlowSpecifiedRegion FlowSpecifiedRegion;

#define	Fsr_list(front)		(front)->head_fsr->next
#define ComponentsMatch(fsr,comp1,comp2,intfc)				\
	(*(fsr)->_ComponentsMatch)(comp1,comp2,intfc)
#define	SetFlowSpecifiedState(nst,ost,coords,ocomp,fsr,fr)		\
	(*(fsr)->_SetFlowSpecifiedState)(nst,ost,coords,ocomp,fsr,fr)
#define	fprint_FlowSpecifiedRegion_data(file,fsr,fr)			\
	(*(fsr)->_fprint_FlowSpecifiedRegion_data)(file,fsr,fr)
#define	DestroyFlowSpecifiedRegion(fsr)					\
	(*(fsr)->_DestroyFlowSpecifiedRegion)(fsr)

struct	_ConstantFlowRegion {
	FlowSpecifiedRegion Fsr;
	Locstate	state;
};
typedef struct _ConstantFlowRegion ConstantFlowRegion;

	/* Macros */

#define	last_time_step_modification(fr)					\
	(((fr)->_last_time_step_modification != NULL) ?			\
		(*(fr)->_last_time_step_modification)() : NO)

#define check_delete_redundant_node(n,c1,c2,fr)                         \
	(((fr)->_check_delete_redundant_node != NULL) ?			\
		(*(fr)->_check_delete_redundant_node)(n,c1,c2) : YES)

#define find_i_to_prop_dir(fr,wave,oldn,oldc,c_orient,dt,ahead_comp,newn,V) \
	(((fr)->_find_i_to_prop_dir != NULL) ?				\
	    (*(fr)->_find_i_to_prop_dir)(fr,wave,oldn,oldc,c_orient,dt, \
					 ahead_comp,newn,V) :           \
	    (f_find_i_to_prop_dir(fr,wave,oldn,oldc,c_orient,dt,        \
				  ahead_comp,newn,V)))

#define pre_advance_front(front)					\
	(*(front)->_pre_advance_front)(front)

#define advance_front(dt,dt_frac,front,newfront,wave)			\
	(*(front)->_advance_front)(dt,dt_frac,front,newfront,wave)

#define copy_front(fr)	(*(fr)->_copy_front)(fr)

#define FrontForceAndTorqueOnHs(fr,hs,dt,force,torque)		\
		(*(fr)->_compute_force_and_torque)(fr,hs,dt,force,torque)

#define copy_into_front(newfr,fr)	(*(fr)->_copy_into_front)(newfr,fr)

#define print_Front_structure(fr)	(*(fr)->_print_Front_structure)(fr)

#define fprint_front(fr,file)		(*(fr)->_fprint_front)(fr,file)

#define print_front(fr)			fprint_front(fr,stdout)

#define read_print_front(init,fr)	(*(fr)->_read_print_front)(init,fr)

#define free_front(fr)	(*(fr)->_free_front)(fr)

#if defined(USE_OVERTURE)
#define deep_copy_front(fr)     (*(fr)->_deep_copy_front)(fr)
#define deep_free_front(fr)     (*(fr)->_deep_free_front)(fr)
#endif /* if defined(USE_OVERTURE) */

#define	init_propagate(fr)						\
	if ((fr)->_init_propagate != NULL)				\
		(*(fr)->_init_propagate)(fr)

#define point_propagate(fr,wave,oldp,newp,oldhse,oldhs,dt,V)		\
	if ((fr)->_point_propagate != NULL)				\
		(*(fr)->_point_propagate)(fr,wave,oldp,newp,		\
					  Hyper_surf_element(oldhse),	\
					  Hyper_surf(oldhs),dt,V)

#define point_propagate_along_wall(fr,wave,oldp,oldb,oldc,oldhse,oldhs,newp,dt,V)	\
	if((fr)->_point_propagate_along_wall != NULL)					\
		(*(fr)->_point_propagate_along_wall)(fr,wave,oldp,oldb,oldc,		\
					  Hyper_surf_element(oldhse),			\
					  Hyper_surf(oldhs), newp, dt,V)

#define intfc_propagate(fr,wave,old_intfc,new_intfc,dt)			\
    if ((fr)->intfc_propagate != NULL)					\
	(*(fr)->intfc_propagate)(fr,wave,old_intfc,new_intfc,dt);


#define curve_propagate(fr,wave,old_curve,new_curve,dt)			\
    if ((fr)->curve_propagate != NULL)					\
	(*(fr)->curve_propagate)(fr,wave,old_curve,new_curve,dt)

#define tan_point_propagate(fr,tmpp,newp,tmphse,tmphs,dt,dir)  		\
	(((fr)->_tan_point_propagate != NULL) ?				\
	    (*(fr)->_tan_point_propagate)(fr,tmpp,newp,		        \
					  Hyper_surf_element(tmphse),   \
					  Hyper_surf(tmphs),dt,dir) : YES)

#define principal_tangent(fr,p,hse,hs,nor,t)				\
	(*(fr)->_principal_tangent)(p,hse,hs,nor,t)

#define	reconstruct_front_at_grid_crossing(fr)				\
	(*(fr)->_reconstruct_front_at_grid_crossing)(fr)

#define	repair_front_at_grid_crossing(fr)				\
	(*(fr)->_repair_front_at_grid_crossing)(fr)

#define untrack_surface(s,newcmp,fr)					\
    (((fr)->_untrack_surface != NULL) ?					\
	    (*(fr)->_untrack_surface)(s,newcmp,fr) : NO)

#define surface_propagate(fr,new_fr,wave,dt,V)			\
    if ((fr)->surface_propagate != NULL)			\
	(*(fr)->surface_propagate)(fr,new_fr,wave,dt,V)


#define untrack_curve(oc,ooc,newcmp,dt,fr,wave,rp,flag)			\
    (((fr)->_untrack_curve != NULL) ?	 				\
        (*(fr)->_untrack_curve)(oc,ooc,newcmp,dt,fr,(POINTER)wave,rp,flag) : NO)

#define	untrack_point(p,comp,fr)					\
    (((fr)->_untrack_point != NULL) ? (*(fr)->_untrack_point)(p,comp,fr) : NO)

#define end_of_curve(s,b,c,orient) 					\
	( (!Following_bond(b,orient))	&&			\
	((orient) == POSITIVE_ORIENTATION ? 				\
		(s) >= END_OF_CURVE((c)->interface) :			\
		(s) <= START_OF_CURVE((c)->interface)))

#define	bond_tangent_to_curve(p,b,c,orient,bdir,fr)			\
		bond_secant_to_curve(p,b,c,orient,bdir,fr,0.0)

#define	correspondence_is_possible(hs,c_hs,p_hsb,n_hsb,fr)		\
	(((fr)->_is_correspondence_possible != NULL) ?			\
		(*(fr)->_is_correspondence_possible)(			\
					Hyper_surf(hs),			\
					Hyper_surf(c_hs),		\
					(HYPER_SURF_BDRY **)p_hsb,	\
					(HYPER_SURF_BDRY **)n_hsb	\
		)							\
		:							\
		YES)

#define hyp_solution(coords,comp,hs,side,fr,wave,state,dflt_state)   \
        (*(fr)->_hyp_solution)(coords,comp,hs,side,fr,wave,state,dflt_state)

#define hyp_grad_solution(coords,comp,hs,side,fr,wave,grad_st)   \
        (*(fr)->_hyp_grad_solution)(coords,comp,hs,side,fr,wave,grad_st)

#define interpolate_states(fr,alpha,beta,coords0,s0,coords1,s1,ans)	\
	(*(fr)->_state_interpolator)(alpha,beta,coords0,s0,coords1,s1,  \
				     (fr)->rect_grid,ans)

#define tri_interpolate_states(fr,a,b,g,crds0,s0,crds1,s1,crds2,s2,ans)    \
	(*(fr)->_tri_state_interpolator)(a,b,g,crds0,s0,crds1,s1,crds2,s2, \
					 (fr)->rect_grid,ans)

	/* Untangle related macros */
#define replace_unphys_loop(nl,new_node_list,newc,fr,i,min_area,flag)	\
	(*(fr)->_replace_unphys_loop)(nl,new_node_list,newc,fr,i,min_area,flag)

	/* Macros to access front time step control */
#define Time_step_factor(fr)		       ((fr)->Tstep.time_step_factor)
#define Apply_CFL_at_nodes(fr)		       ((fr)->Tstep.apply_cfl_at_nodes)
#define Max_new_node_separation(fr)	       ((fr)->Tstep.max_sep)
#define Time_step_increase_factor(fr)	       ((fr)->Tstep.cfl_fudge)
#define Min_time_step_modification_factor(fr)  ((fr)->Tstep.frac_floor)
#define Max_time_step_modification_factor(fr)  ((fr)->Tstep.frac_ceil)

	/* Macros to access front redistribution control */

#define Redistribution_info(fr)	(fr)->Redist

#define	Curve_redistribution_info(fr)	Redistribution_info(fr).Curve_redist

#define Tracking_algorithm(fr)						\
	(Redistribution_info(fr).tracking_algorithm)

#define Interface_redistributed(fr)					\
	(Redistribution_info(fr).intfc_redistributed)

#define Redistribution_mode(fr)						\
	(Redistribution_info(fr)._Redistribution_mode)

#define CurveRedistributionOptions(fr)					\
	(Redistribution_info(fr)._CurveRedistributionOptions)

#define SurfaceRedistributionOptions(fr)				\
	(Redistribution_info(fr)._SurfaceRedistributionOptions)

#define Init_redistribution_function(fr)				\
	(Redistribution_info(fr)._init_redistribute)
#define Init_redistribution(init,fr)					\
	if (Init_redistribution_function(fr) != NULL)			\
		(*Init_redistribution_function(fr))(init,fr)

#define Curve_redistribution_function(fr)				\
	(Curve_redistribution_info(fr).Redistributors.curve_redist_func)
#define Curve_redistribute(fr,force)					\
	((Curve_redistribution_function(fr) != NULL) ?			\
			(*Curve_redistribution_function(fr))(fr,force) : YES)

#define Forward_curve_redistribute_function(fr)				\
    (Curve_redistribution_info(fr).Redistributors.forward_cur_redist)
#define Forward_curve_redistribute(fr,c,status)				\
    ((Forward_curve_redistribute_function(fr) != NULL) ?		\
         (*Forward_curve_redistribute_function(fr))(fr,c,status) : status)

#define Backward_curve_redistribute_function(fr)			\
    (Curve_redistribution_info(fr).Redistributors.backward_cur_redist)
#define Backward_curve_redistribute(fr,c,status)			\
    ((Backward_curve_redistribute_function(fr) != NULL) ?	\
	 (*Backward_curve_redistribute_function(fr))(fr,c,status) : status)

#define Node_redistribute_function(fr)					\
    (Curve_redistribution_info(fr).Redistributors.node_redistribute)
#define Node_redistribute(fr,status)					\
    ((Node_redistribute_function(fr) != NULL) ?				\
        (*Node_redistribute_function(fr))(fr,status) : status)

#define Rect_boundary_redistribute_function(fr)				\
	(Redistribution_info(fr)._rect_boundary_redistribute)
#define rect_boundary_redistribute(intfc,gr,step,fr)			\
	if (Rect_boundary_redistribute_function(fr) != NULL)		\
	    (*Rect_boundary_redistribute_function(fr))(intfc,gr,step)

#define Delete_small_loops_function(fr)					\
	(Curve_redistribution_info(fr).Cleaners._delete_small_loops)
#define delete_small_loops(fr)						\
	if (Delete_small_loops_function(fr) != NULL)			\
	    (*Delete_small_loops_function(fr))(fr)

#define Delete_fold_back_bonds_function(fr)				\
	(Curve_redistribution_info(fr).Cleaners._delete_fold_back_bonds)
#define delete_fold_back_bonds(fr,c,min_sc_sep,found,zl_c)		\
	if (Delete_fold_back_bonds_function(fr) != NULL) 		\
	 (*Delete_fold_back_bonds_function(fr))(fr,c,min_sc_sep,found,zl_c)

#define Delete_exterior_curves_function(fr)				\
	(Curve_redistribution_info(fr).Cleaners._delete_exterior_curves)
#define delete_exterior_curves(fr,intfc)				\
	if (Delete_exterior_curves_function(fr) != NULL)		\
	    (*Delete_exterior_curves_function(fr))(fr,intfc)

#define Delete_phys_remn_on_bdry_function(fr)				\
	(Curve_redistribution_info(fr).Cleaners._delete_phys_remn_on_bdry)
#define delete_phys_remn_on_bdry(fr)					\
	((Delete_phys_remn_on_bdry_function(fr) != NULL) ?		\
	 (*Delete_phys_remn_on_bdry_function(fr))(fr) : YES)

#define Delete_point_adjacent_to_node_function(fr)			\
	(Curve_redistribution_info(fr).Cleaners._delete_point_adjacent_to_node)
#define delete_point_adjacent_to_node(fr,c,orient)			\
	((Delete_point_adjacent_to_node_function(fr) != NULL) ?		\
		(*Delete_point_adjacent_to_node_function(fr))(fr,c,orient) :\
		FUNCTION_FAILED)

#define Grid_size_of_redistribution(fr)                                 \
	        (Redistribution_info(fr).grid_size_redist)
#define Redistribution_count(fr)					\
		(Redistribution_info(fr).count_redist)
#define Frequency_of_redistribution(fr,i) 				\
		(Redistribution_info(fr).freq_redist[i])
#define Frequency_of_reconstruction(fr)					\
		(Redistribution_info(fr).freq_reconstruct)
#define Use_rect_boundary_redistribution(fr) 				\
		(Curve_redistribution_info(fr).rect_bdry_redist)
#define Front_spacing(fr,i) 						\
		(Redistribution_info(fr).spacing[i])
#define Cosine_big_angle(fr,i)						\
		(Curve_redistribution_info(fr).cos_big_angle[i])
#define Front_length(fr)						\
		(Curve_redistribution_info(fr).length)

#define reconstruction_needed(fr)					\
			(Redistribution_count(fr) %			\
			Frequency_of_reconstruction(fr)) ?		\
		NO : YES;

	/* Redistribution of surfaces macros */

#define	Surface_redistribution_info(fr)	Redistribution_info(fr).Surface_redist

#define Surface_redistribution_function(fr)				\
	(Surface_redistribution_info(fr).surface_redist_func)

#define Surface_redistribute(fr,force)					\
	((Surface_redistribution_function(fr) != NULL) ?		\
			(*Surface_redistribution_function(fr))(fr,force) : YES)

#define Max_bond_len(fr,i)						\
		(Surface_redistribution_info(fr).max_len[i])

#define Min_bond_len(fr,i)						\
		(Surface_redistribution_info(fr).min_len[i])

#define Max_tri_sqr_area(fr,i)						\
		(Surface_redistribution_info(fr).max_area2[i])

#define Min_tri_sqr_area(fr,i)						\
		(Surface_redistribution_info(fr).min_area2[i])

#define	Max_scaled_tri_side_sqr_length(fr)				\
		(Surface_redistribution_info(fr).max_sqr_tri_len)

#define Aspect_ratio_tolerance(fr,i)					\
		(Surface_redistribution_info(fr).ar_tol[i])

	/* Macros for untrack_curve control*/

#define set_states_set_at_node_flag(flag,orient,status)			\
	switch (orient)							\
	{								\
	case POSITIVE_ORIENTATION:					\
	    (flag).start_states_set = (status);				\
	    break;							\
	case NEGATIVE_ORIENTATION:					\
	    (flag).end_states_set = (status);				\
	    break;							\
	case ORIENTATION_NOT_SET:					\
	    screen("ERROR in set_states_set_at_node_flag() macro "	\
		   "invalid orientation value\n");			\
	    clean_up(ERROR);						\
	    break;							\
	}

#define states_set_at_node(flag,orient)					 \
	(((orient)==POSITIVE_ORIENTATION) ? (flag).start_states_set :    \
	                                    (flag).end_states_set)

		/* Macros for printing */

#define fgraph_front_states(file,fr)					\
	if ((fr)->_fgraph_front_states != NULL)				\
		(*(fr)->_fgraph_front_states)((file),(fr))

#define graph_front_states(fr)	fgraph_front_states(stdout,fr)

#define fprint_header_for_graph_curve_states(file,fr,title)		\
	if ((fr)->_fprint_header_for_graph_curve_states != NULL)	\
		(*(fr)->_fprint_header_for_graph_curve_states)((file),	\
							       (fr),(title))

#define print_header_for_graph_curve_states(fr,title)			\
		fprint_header_for_graph_curve_states(stdout,(fr),(title))

#define fgraph_curve_states(file,c,fr,arclen)				\
	if ((fr)->_fgraph_curve_states != NULL)				\
	    (*(fr)->_fgraph_curve_states)((file),(c),(fr),(arclen))

#define check_front_state_consistency(fr)				\
	if ((fr)->_check_front_state_consistency != NULL)		\
	    (*(fr)->_check_front_state_consistency)((fr))

#define graph_curve_states(c,fr,arclen)					\
		fgraph_curve_states(stdout,(c),(fr),(arclen))

struct _RESTART_DATA {
	const IO_TYPE   *_io_type;
	INTERFACE	*_intfc;
	RECT_GRID	_comp_grid, _top_grid;
	int		_multi_data;
	boolean		_got_intfc_from_file;
	int		_time_step;	/*PROMPTED*/
	double		_time;		/*Read from restart file*/
	double		_dt_last;	/*Read from restart file*/
	double		_next_dt;	/*Read from restart file,
					  can be overridder by prompting*/
	IO_TYPE         _IO_type_store;
};
typedef struct _RESTART_DATA RESTART_DATA;

/*
*	Initialization structure for front library
*/

enum _TANGENT_METHOD {
	LINEAR_SECANT,
	LANGRANGIAN_INTERPOLANT,
	CUBIC_SPLINE,
	WLSP_TANGENT,
	TANGENT_METHOD_FROM_RESTART
};
typedef enum _TANGENT_METHOD TANGENT_METHOD;

enum _NORMAL_METHOD {
	FIRST_ORDER_NORMAL,
	AREA_WEIGHTED_NORMAL,
	SINE_WEIGHTED_NORMAL,
	PLANE_FIT_NORMAL,
	WLSP_NORMAL,
	NORMAL_METHOD_FROM_RESTART
};
typedef enum _NORMAL_METHOD NORMAL_METHOD;

enum _CURVATURE_METHOD {
	NORMAL_CURVATURE,
	WLSP_CURVATURE,
	CURVATURE_METHOD_FROM_RESTART
};
typedef enum _CURVATURE_METHOD CURVATURE_METHOD;

struct _F_INIT_DATA {
	I_INIT_DATA	I_init_data;
	boolean		use_default_front_data;

	RESTART_DATA	_Restart_data;

	size_t		_StateSize;
	int		_NumberFloats;

	TANGENT_METHOD	_tangent_method;
	NORMAL_METHOD _normal3d_method;

	double  _redistribution_grid_size;     /*Prompted*/
	int    _redistribution_count;        /* Prompted or from restart file */
	boolean   _redis_flag;                  /*used for LGB restart. */
	int    _redistribution_frequency[3]; /* Prompted */
	int    _reconstruction_frequency;    /* Prompted */
	double  _front_spacing[2];            /* Prompted */
	TRACKING_ALGORITHM _tracking_algorithm; /* Prompted for 3d */
	REDISTRIBUTION_MODE _front_redist_mode; /* Prompted */

	TSTEP_CONTROL _Tstep;

	void (*_set_redistribution_defaults)(INIT_DATA*);
	void (*_set_front_time_step_control_default)(INIT_DATA*);
	void (*_copy_redistribution_values)(INIT_DATA*,Front*);
	void (*_prompt_for_redistribute)(INIT_DATA*);

	CURVE_REDIST_OPTIONS _curve_redist_options;

	SURFACE_REDIST_OPTIONS _surface_redist_options;

	MAX_FRONT_SPEED	*_MaxFrontSpeed;

	boolean _EnforceFlowSpecifedStates;
	boolean	(*_read_print_FlowSpecifiedRegion_data)(INIT_DATA*,
	                                                const IO_TYPE*,
				struct _FlowSpecifiedRegion**);
	void    (*_prompt_for_front_options)(INIT_DATA*,Front*);
	void    (*_read_print_front_options)(INIT_DATA*,Front*);
	int  movingframe;
};
typedef struct _F_INIT_DATA F_INIT_DATA;

#define	f_init_data(init)	((F_INIT_DATA*)(init))
#define	Comp_grid(init)		Computational_grid(i_intfc(init))

/* Macros for restart initialization*/
#define	Restart_data(init)		f_init_data(init)->_Restart_data
#define	restart_intfc(init)		Restart_data(init)._intfc
#define	restart_io_type(init)		Restart_data(init)._io_type
#define	Restart_comp_grid(init)		Restart_data(init)._comp_grid
#define	Restart_top_grid(init)		Restart_data(init)._top_grid
#define	restart_multi_data(init)	Restart_data(init)._multi_data
#define	got_intfc_from_file(init)	Restart_data(init)._got_intfc_from_file
#define	restart_time_step(init)		Restart_data(init)._time_step
#define	restart_time(init)		Restart_data(init)._time
#define	restart_dt_last(init)		Restart_data(init)._dt_last
#define	restart_next_dt(init)		Restart_data(init)._next_dt
#define	restart_IO_type_store(init)	Restart_data(init)._IO_type_store

#define	InitialMaxFrontSpeed(init)	f_init_data(init)->_MaxFrontSpeed
#define	initial_read_print_max_front_speed_info(init,front)		\
    (*ReadPrintMaxFrontSpeedInfo(f_init_data(init)))			\
	(init,restart_io_type(init),front,InitialMaxFrontSpeed(init))

#define StateSize(init)		f_init_data(init)->_StateSize
#define NumberFloats(init)	f_init_data(init)->_NumberFloats

#define tangent_method(init)      	f_init_data(init)->_tangent_method
#define movingframe(init)               f_init_data(init)->movingframe
#define normal3d_method(init)      	f_init_data(init)->_normal3d_method

/* Tracking method macros,  3D only */
#define	tracking_algorithm(init)   (f_init_data(init)->_tracking_algorithm)

#define	time_step_control_options(init)	f_init_data(init)->_Tstep

/* Redistribution macros for initialization */
#define redistribution_count(init) (f_init_data(init)->_redistribution_count)
#define redis_flag(init) (f_init_data(init)->_redis_flag)
#define redistribution_grid_size(init)                                  \
	       (f_init_data(init)->_redistribution_grid_size)
#define redistribution_frequency(init,i)   				\
    (f_init_data(init)->_redistribution_frequency[i])
#define reconstruction_frequency(init)					\
    (f_init_data(init)->_reconstruction_frequency)
#define front_spacing(init,i) (f_init_data(init)->_front_spacing[i])
#define front_redist_mode(init)    f_init_data(init)->_front_redist_mode
#define	curve_redist_options(init) f_init_data(init)->_curve_redist_options
#define full_curve_redist_version(init)					\
    (curve_redist_options(init)._full_curve_redist_version)
#define use_rect_bdry_redistribution(init)				\
    (curve_redist_options(init)._rect_bdry_redist)
#define cosine_big_angle(init,i)					\
    (curve_redist_options(init)._cos_big_angle[i])
#define set_redistribution_defaults(init)   				\
    (*(f_init_data(init)->_set_redistribution_defaults))(init)
#define set_front_time_step_control_default(init)   			\
    (*(f_init_data(init)->_set_front_time_step_control_default))(init)
#define copy_redistribution_values(init,front)				\
    (*(f_init_data(init)->_copy_redistribution_values))(init,front)
#define prompt_for_redistribute(init)   				\
    (*(f_init_data(init)->_prompt_for_redistribute))(init)
#define surface_redist_options(init)					\
    f_init_data(init)->_surface_redist_options
#define maximum_triangle_area_factor(init,i)				\
    (surface_redist_options(init)._max_tri_area_factor[i])
#define minimum_triangle_area_factor(init,i)				\
    (surface_redist_options(init)._min_tri_area_factor[i])
#define minimum_angle_at_triangle_vertex(init,i)			\
    (surface_redist_options(init)._min_angle_at_tri_vertex[i])
#define maximum_scaled_triangle_side_length(init)			\
    (surface_redist_options(init)._max_scaled_tri_side_len)
#define	read_print_FlowSpecifiedRegion_data(init,iot,pfsr)		\
    (*f_init_data(init)->_read_print_FlowSpecifiedRegion_data)(init,iot,pfsr)
#define	read_print_front_options(init,front)				\
    (*f_init_data(init)->_read_print_front_options)(init,front)
#define	prompt_for_front_options(init,front)				\
    (*f_init_data(init)->_prompt_for_front_options)(init,front)
#define enforce_flow_specified_states(init)				\
    (f_init_data(init)->_EnforceFlowSpecifedStates)

struct _LEVEL_FUNC_PACK {
	/* Not needed for restart initialization */
        COMPONENT neg_component;
        COMPONENT pos_component;

	/* For level set initialization */
        double (*func)(POINTER,double*);
        POINTER func_params;

	/* For point array initialization */
	int num_points;
	double **point_array;
	boolean is_closed_curve;
	/* For initialization by reading SDL file */
	boolean read_sdl_input;
	char *sdl_name;
	/* For initialization by reading VTK file */
	boolean read_vtk_input;
	char *vtk_name;
	int wave_type;

	boolean is_mono_hs;
	int num_mono_hs;
	/* For constrained level set initialization */
        boolean (*constr_func)(POINTER,double*);
        POINTER constr_params;
	/* For constrained level set initialization */
        boolean (*string_func)(INTERFACE*,SURFACE*,POINTER,int);
        POINTER string_params;
	
	boolean set_3d_bdry;
	boolean attach_string;
};
typedef struct _LEVEL_FUNC_PACK LEVEL_FUNC_PACK;

struct _F_BASIC_DATA {
	/* Need to assign before calling FT_Init() */
        int dim;

	/* The following will get from FT_Init() */
	char 	in_name[200];
	char 	out_name[200];
	int 	subdomains[MAXD];
	boolean ReadFromInput;
	boolean RestartRun;
	int 	RestartStep;
        char 	restart_name[200];
	char	restart_state_name[200];

	/* The following are needed before FT_StartUp() */
        double 	L[MAXD],U[MAXD];
        int 	gmax[MAXD];
        int 	boundary[MAXD][2];
	size_t 	size_of_intfc_state;
	GEOMETRY_REMAP coord_system;
};
typedef struct _F_BASIC_DATA F_BASIC_DATA;

struct _VELO_FUNC_PACK {
        int (*func)(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*,
                        HYPER_SURF*,double*);
	void (*point_propagate)(struct _Front*,POINTER,FT_POINT*,FT_POINT*,
                        HYPER_SURF_ELEMENT*,HYPER_SURF*,double,double*);
        POINTER func_params;
};
typedef struct _VELO_FUNC_PACK VELO_FUNC_PACK;

enum {
        TSTEP_FIELD_WIDTH   = 7,         
	PP_NODE_FIELD_WIDTH = 4
}; 

struct _INTRP_CELL {
        int dim;
        boolean is_bilinear;
        double **coords;
        double *var;
        int nv;
        double **p_lin;
        double *var_lin;
};
typedef struct _INTRP_CELL INTRP_CELL;

#if defined(USE_HDF)

typedef	enum {HDF_RASTER, HDF_SDS} HDF_DATA_TYPE;

struct _HDF_PRINT_OPTIONS {
	HDF_DATA_TYPE	_hdf_data_type;
	double		_hdf_L0[3];	/* plotting window is defined    */
	double		_hdf_U0[3];	/* by the rectangle with corners */
	double		_hdf_len[3];	/* U0 - L0			 */
	double		_hdf_V[3];	/* L0 + V*t, U0 + V*t		 */
	int		_hdf_pixels[3];	/* pixels in direction           */
	int		_hdf_num_vars;
	comp_coder_t	_sds_compression_type;
	uint16		_ras_compression_type;
	comp_info	_hdf_c_info;
	boolean		_hdf_sub_div;
	/* needed for VTK */
	int             numvects;
	int             vecselect[3][20];
	/* end needed for VTK */

};
typedef struct _HDF_PRINT_OPTIONS HDF_PRINT_OPTIONS;
/* needed for VTK */
#define HDF_numvects(Hdf_po)        (Hdf_po).numvects
#define hdf_numvects(hdf_po)        (hdf_po)->numvects
#define HDF_vecselect(Hdf_po)        (Hdf_po).vecselect
#define hdf_vecselect(hdf_po)        (hdf_po)->vecselect
/* end needed for VTK */
#define	HDF_data_type(Hdf_po)        (Hdf_po)._hdf_data_type
#define	hdf_data_type(hdf_po)        (hdf_po)->_hdf_data_type
#define	HDF_L0(Hdf_po)	             (Hdf_po)._hdf_L0
#define	hdf_L0(hdf_po)	             (hdf_po)->_hdf_L0
#define	HDF_U0(Hdf_po)	             (Hdf_po)._hdf_U0
#define	hdf_U0(hdf_po)	             (hdf_po)->_hdf_U0
#define	HDF_len(Hdf_po)	             (Hdf_po)._hdf_len
#define	hdf_len(hdf_po)	             (hdf_po)->_hdf_len
#define	HDF_V(Hdf_po)	             (Hdf_po)._hdf_V
#define	hdf_V(hdf_po)	             (hdf_po)->_hdf_V
#define	HDF_pixels(Hdf_po)           (Hdf_po)._hdf_pixels
#define	hdf_pixels(hdf_po)           (hdf_po)->_hdf_pixels
#define	HDF_num_vars(Hdf_po)         (Hdf_po)._hdf_num_vars
#define	hdf_num_vars(hdf_po)	     (hdf_po)->_hdf_num_vars
#define	RAS_compression_type(Hdf_po) (Hdf_po)._ras_compression_type
#define	ras_compression_type(hdf_po) (hdf_po)->_ras_compression_type
#define	SDS_compression_type(Hdf_po) (Hdf_po)._sds_compression_type
#define	sds_compression_type(hdf_po) (hdf_po)->_sds_compression_type
#define	HDF_compression_info(Hdf_po) (Hdf_po)._hdf_c_info
#define	hdf_compression_info(hdf_po) (hdf_po)->_hdf_c_info
#define	HDF_subdomain_div(hdf_po)    (hdf_po)._hdf_sub_div
#define	hdf_subdomain_div(hdf_po)    (hdf_po)->_hdf_sub_div

typedef double (HDF_PLOT_FILTER)(double);

struct _HDF_FRAME_OPTS {
	boolean	_hdf_dflt_scale; /* Use default scaling?          */
	char	_hdf_selector[256];
	char	_hdf_plot_name[1024];
	char	_hdf_palette_name[1024];
	double	_hdf_scale_min;  /* for non-default scaling       */
	double	_hdf_scale_max;	 /* for non-default scaling       */
	double	(*_hdf_plot_function)(double*,Front*,POINTER,
				      COMPONENT,Locstate);
	HDF_PLOT_FILTER *_hdf_plot_filter;
};
typedef struct _HDF_FRAME_OPTS HDF_FRAME_OPTS;

#define	HDF_dflt_scale(Fopts)		(Fopts)._hdf_dflt_scale
#define	hdf_dflt_scale(fopts)		(fopts)->_hdf_dflt_scale
#define	HDF_selector(Fopts)		(Fopts)._hdf_selector
#define	hdf_selector(fopts)		(fopts)->_hdf_selector
#define	HDF_plot_name(Fopts)		(Fopts)._hdf_plot_name
#define	hdf_plot_name(fopts)		(fopts)->_hdf_plot_name
#define	HDF_palette_name(Fopts)		(Fopts)._hdf_palette_name
#define	hdf_palette_name(fopts)		(fopts)->_hdf_palette_name
#define	HDF_scale_min(Fopts)		(Fopts)._hdf_scale_min
#define	hdf_scale_min(fopts)		(fopts)->_hdf_scale_min
#define	HDF_scale_max(Fopts)		(Fopts)._hdf_scale_max
#define	hdf_scale_max(fopts)		(fopts)->_hdf_scale_max
#define	HDF_plot_function(Fopts)	(Fopts)._hdf_plot_function
#define	hdf_plot_function(fopts)	(fopts)->_hdf_plot_function
#define	HDF_plot_filter(Fopts)		(Fopts)._hdf_plot_filter
#define	hdf_plot_filter(fopts)		(fopts)->_hdf_plot_filter

struct _HDF_plot_data {

	HDF_PRINT_OPTIONS	_HDF_print_opts;

	int		dim;
	double		step[3];	/* step = (U0 - L0)/pixels	 */
	uint8		*raster_data;
	double		*scale[4];
	COMPONENT	*comp;
	int		num_values;
	int		num_raster_data;

	struct _HDF_frame_data {

		HDF_FRAME_OPTS	_HDF_frame_opts;

		boolean	        first;
		boolean	        append;
		double	        current_time_min;
		double	        current_time_max;
		double	        cumulative_time_min;
		double	        cumulative_time_max;
		char            *file_name;
		double	        *values;
		uint8	        palette[3*256];
		uint8	        num_table_colors;
		uint8	        num_colors;
		uint8	        line_color;
	}	*frame_data;
};
typedef struct _HDF_plot_data HDF_plot_data;
#if defined(__cplusplus)
typedef struct HDF_plot_data::_HDF_frame_data HDF_frame_data;
#else /* defined(__cplusplus) */
typedef struct _HDF_frame_data HDF_frame_data;
#endif /* defined(__cplusplus) */

#define	HDF_print_opts(hdf_pdata)	(hdf_pdata)->_HDF_print_opts

#define	HDF_frame_opts(Fdata)		(Fdata)._HDF_frame_opts
#define	hdf_frame_opts(fdata)		(fdata)->_HDF_frame_opts

#define HDF_frame_dflt_scale(Fdata)	HDF_dflt_scale(HDF_frame_opts(Fdata))
#define hdf_frame_dflt_scale(fdata)	HDF_dflt_scale(hdf_frame_opts(fdata))
#define HDF_frame_scale_min(Fdata)	HDF_scale_min(HDF_frame_opts(Fdata))
#define hdf_frame_scale_min(fdata)	HDF_scale_min(hdf_frame_opts(fdata))
#define HDF_frame_scale_max(Fdata)	HDF_scale_max(HDF_frame_opts(Fdata))
#define hdf_frame_scale_max(fdata)	HDF_scale_max(hdf_frame_opts(fdata))
#define HDF_frame_plot_name(Fdata)	HDF_plot_name(HDF_frame_opts(Fdata))
#define hdf_frame_plot_name(fdata)	HDF_plot_name(hdf_frame_opts(fdata))
#define HDF_frame_plot_filter(Fdata)	HDF_plot_filter(HDF_frame_opts(Fdata))
#define hdf_frame_plot_filter(fdata)	HDF_plot_filter(hdf_frame_opts(fdata))
#define HDF_frame_plot_function(Fdata)	HDF_plot_function(HDF_frame_opts(Fdata))
#define hdf_frame_plot_function(fdata)	HDF_plot_function(hdf_frame_opts(fdata))
#endif /* defined(USE_HDF) */

/* Front Macros for outside users */

#define FrontRedistribute(front)				\
	redistribute((front),YES,NO)

#define GetFrontNormal(p,hse,hs,nor,front)			\
	normal(p,hse,hs,nor,front)

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#include <front/fuserint.h>
#include <front/fprotos.h>
#endif /* !defined(_FDECS_H) */
