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
*				fstate.c:
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	Contains routines related to states on the front:
*
*		nearest_intfc_state_and_pt()
*/


#include <front/fdecs.h>

	/* LOCAL Function Declarations */
LOCAL	FlowSpecifiedRegion *FSR_for_comp(COMPONENT,Front*,INTERFACE*);
LOCAL	boolean	SetConstantRegionState(Locstate,Locstate,double*,COMPONENT,
				       FlowSpecifiedRegion*,Front*);
LOCAL	boolean	SetSkipComponentState(Locstate,Locstate,double*,COMPONENT,
				      FlowSpecifiedRegion*,Front*);
LOCAL	boolean	SkipAllComps(COMPONENT,COMPONENT,INTERFACE*);
LOCAL	void	DestroyConstantFlowRegion(FlowSpecifiedRegion*);
LOCAL	void	DestroySkipComponentRegion(FlowSpecifiedRegion*);
LOCAL	void	fprint_ConstantFlowRegion_data(FILE*,FlowSpecifiedRegion*,
					       Front*);

EXPORT	void	f_alloc_state(
	Locstate        *state,
	size_t          sizest)
{
	scalar(state,sizest);
}               /*end f_alloc_state*/

EXPORT	Locstate	f_alloc_intfc_state(
	size_t          sizest)
{
	Locstate	newst;

	newst = (Locstate) store(sizest);
	return newst;
}               /*end f_alloc_intfc_state*/

EXPORT	void	f_clear_state(
	Locstate        state,
	size_t          sizest)
{
	zero_scalar(state,sizest);
}               /*end f_clear_state*/

/*
*		nearest_intfc_state_and_pt():
*
*	Finds nearest interface point and associated state. Tries old intfc
*	first. If unsuccessful, tries newintfc.
*/

EXPORT	void nearest_intfc_state_and_pt(
	double		*coords,
	COMPONENT	comp,
	Front		*front,
	Front		*newfront,
	Locstate	state,
	double		*coords_on,
	HYPER_SURF	**hs_on)
{
	boolean		status;
	INTERFACE	*intfc;

			/* try old intfc first */
	intfc = front->interf;
	status = nearest_intfc_state(coords,comp,intfc,state,coords_on,hs_on);
	if ((status != YES) && (newfront != NULL))
	{
	    (void) printf("WARNING in nearest_intfc_state_and_pt(), "
	    	      "nearest_intfc_state failed on front->interf\n");
	    intfc = newfront->interf;
	    status = nearest_intfc_state(coords,comp,intfc,
				         state,coords_on,hs_on);
	}
	if (status != YES)
	{
	    int dim = front->rect_grid->dim;
	    screen("ERROR in nearest_intfc_state_and_pt(), "
	           "nearest_intfc_state() failed on both interfaces\n");
	    (void) printf("nearest_intfc_state_and_pt() fails to find\n");
	    print_general_vector("intfc point for ",coords,dim,"");
	    (void) printf(" comp %d\n",comp);
	    clean_up(ERROR);
	}
}		/*end nearest_intfc_state_and_pt*/


EXPORT	FlowSpecifiedRegion	*AddToFsrList(
	FlowSpecifiedRegion	*fsr)
{
	static	FlowSpecifiedRegion	Head;/*Permanent head of list*/
	static	FlowSpecifiedRegion	Tail;/*Permanent tail of list*/

	if (fsr == NULL)
	    return &Head;

	if (Head.next == NULL) /*New list*/
	{
	    Head.next = fsr;
	    fsr->prev = NULL;
	}
	else
	{
	    fsr->prev = Tail.prev;
	    Tail.prev->next = fsr;
	}

	Tail.prev = fsr;
	fsr->next = NULL;
	fsr->head = &Head;
	fsr->tail = &Tail;
	return fsr;
}


/*
*			RegionIsFlowSpecified():
*
*	Determines whether COMPONENT comp corresponds to a region where
*	the flow is specified by a user defined function.  For such regions
*	the specified state value is recorded in nst.  Return YES if the
*	value of nst is set,  NO otherwise.
*/

EXPORT	boolean	RegionIsFlowSpecified(
	Locstate	nst,
	Locstate	ost,
	double		*coords,
	COMPONENT	new_comp,
	COMPONENT	old_comp,
	Front		*fr)
{
	FlowSpecifiedRegion *fsr;

	for (fsr = Fsr_list(fr); fsr != NULL; fsr = fsr->next)
	    if (ComponentsMatch(fsr,new_comp,fsr->comp,fr->interf))
	        return SetFlowSpecifiedState(nst,ost,coords,old_comp,fsr,fr);
	return NO;
}		/*end RegionIsFlowSpecified*/

/*
*			ComponentIsFlowSpecified():
*
*	Determines whether COMPONENT comp corresponds to a region where the
*	flow is specified by a user defined function.
*/

EXPORT boolean	ComponentIsFlowSpecified(
	COMPONENT	comp,
	Front		*fr)
{
	if (is_excluded_comp(comp,fr->interf))
	    return YES;
	return (FSR_for_comp(comp,fr,fr->interf) != NULL) ? YES : NO;
}		/*end ComponentIsFlowSpecified*/

EXPORT	void	SetActiveFlowComponent(
	COMPONENT	comp,
	Front		*front)
{
	FlowSpecifiedRegion *fsr;

	for (fsr = Fsr_list(front); fsr != NULL; fsr = fsr->next)
	{
	    if (fsr->comp != comp)
		continue;

	    if (fsr == fsr->head->next)/*First in list*/
	    {
	    	fsr->head->next = fsr->next;
	    	if (fsr->head->next != NULL)
	    		fsr->head->next->prev = NULL;
	    }
	    else
	    {
	    	fsr->prev->next = fsr->next;
	    }
	    if (fsr == fsr->tail->prev)/*Last in list*/
	    {
	    	fsr->tail->prev = fsr->prev;
	    	if (fsr->tail->prev != NULL)
	    	    fsr->tail->prev->next = NULL;
	    }
	    else
	    {
	    	fsr->next->prev = fsr->prev;
	    }
	    DestroyFlowSpecifiedRegion(fsr);
	}
}		/*end SetActiveFlowComponent*/

/*
*			SetConstantRegionState():
*
*	Sets the value of state for a region of constant flow.
*/

/*ARGSUSED*/
LOCAL	boolean	SetConstantRegionState(
	Locstate		nst,
	Locstate		ost,
	double			*coords,
	COMPONENT		old_comp,
	FlowSpecifiedRegion	*fsr,
	Front			*fr)
{
	ConstantFlowRegion *cfr = (ConstantFlowRegion*)fsr;
	Locstate	const_state = cfr->state;

	ft_assign(nst,const_state,fr->sizest);
	return YES;
}		/*end SetConstantRegionState*/

/*
*			SetSkipComponentState():
*
*	Copies ost to nst if old_comp is equivalent to fsr->comp.
*/

/*ARGSUSED*/
LOCAL	boolean	SetSkipComponentState(
	Locstate		nst,
	Locstate		ost,
	double			*coords,
	COMPONENT		old_comp,
	FlowSpecifiedRegion	*fsr,
	Front			*fr)
{
	if (ComponentsMatch(fsr,old_comp,fsr->comp,fr->interf))
	{
	    ft_assign(nst,ost,fr->sizest);
	    return YES;
	}
	return NO;
}		/*end SetSkipComponentState*/


EXPORT	ConstantFlowRegion	*SetConstantFlowRegion(
	COMPONENT	comp,
	Locstate	state,
	INTERFACE	*intfc)
{
	ConstantFlowRegion *cfr;
	FlowSpecifiedRegion *fsr;

	fsr = FSR_for_comp(comp,NULL,intfc);
	if (fsr != NULL)
	{
	    size_t sizest = size_of_state(intfc);
	    if (strcmp(fsr->type,"CONSTANT_REGION") != 0)
	    {
		screen("ERROR in SetConstantFlowRegion(), "
		       "attempt to respecify a flow specified region\n");
		clean_up(ERROR);
	    }
	    cfr = (ConstantFlowRegion*)fsr;
	    if (sizest != 0)
	    {
		if (memcmp(state,cfr->state,sizest) != 0)
		{
		    screen("ERROR in SetConstantFlowRegion(), "
		           "attempt to respecify a constant region state\n");
		    clean_up(ERROR);
		}
	    }
	    return cfr;
	}
	scalar(&cfr,sizeof(ConstantFlowRegion));
	cfr->Fsr.comp = comp;
	sprintf(cfr->Fsr.type,"CONSTANT_REGION");
	cfr->Fsr._ComponentsMatch = equivalent_comps;
	cfr->Fsr._SetFlowSpecifiedState = SetConstantRegionState;
	cfr->Fsr._fprint_FlowSpecifiedRegion_data =
				fprint_ConstantFlowRegion_data;
	cfr->Fsr._DestroyFlowSpecifiedRegion = DestroyConstantFlowRegion;
	alloc_state(intfc,&cfr->state,size_of_state(intfc));
	ft_assign(cfr->state,state,size_of_state(intfc));
	(void) AddToFsrList(&cfr->Fsr);
	return cfr;
}		/*end SetConstantFlowRegion*/

/*ARGSUSED*/
EXPORT	FlowSpecifiedRegion	*SetSkipComponentRegion(
	COMPONENT	comp)
{
	FlowSpecifiedRegion	*fsr;

	scalar(&fsr,sizeof(FlowSpecifiedRegion));
	fsr->comp = comp;
	sprintf(fsr->type,"SKIP_COMPONENT_REGION");
	fsr->_ComponentsMatch = equivalent_comps;
	fsr->_SetFlowSpecifiedState = SetSkipComponentState;
	fsr->_fprint_FlowSpecifiedRegion_data =
				f_fprint_FlowSpecifiedRegion_data;
	fsr->_DestroyFlowSpecifiedRegion = DestroySkipComponentRegion;
	(void) AddToFsrList(fsr);
	return fsr;
}		/*end SetSkipComponentRegion*/

/*ARGSUSED*/
EXPORT	FlowSpecifiedRegion	*SetSkipAllComponents(void)
{
	FlowSpecifiedRegion	*fsr;

	scalar(&fsr,sizeof(FlowSpecifiedRegion));
	fsr->comp = NO_COMP;
	sprintf(fsr->type,"SKIP_ALL_COMPONENTS");
	fsr->_ComponentsMatch = SkipAllComps;
	fsr->_SetFlowSpecifiedState = SetSkipComponentState;
	fsr->_fprint_FlowSpecifiedRegion_data =
				f_fprint_FlowSpecifiedRegion_data;
	fsr->_DestroyFlowSpecifiedRegion = DestroySkipComponentRegion;
	(void) AddToFsrList(fsr);
	return fsr;
}		/*end SetSkipAllComponents*/

/*ARGSUSED*/
LOCAL	boolean	SkipAllComps(
	COMPONENT	comp1,
	COMPONENT	comp2,
	INTERFACE	*intfc)
{
	return YES;
}		/*end SkipAllComps*/

/*ARGSUSED*/
LOCAL	void	fprint_ConstantFlowRegion_data(
	FILE			*file,
	FlowSpecifiedRegion	*fsr,
	Front			*fr)
{
	ConstantFlowRegion *cfr = (ConstantFlowRegion*)fsr;
	f_fprint_FlowSpecifiedRegion_data(file,fsr,fr);
	(void) fprintf(file,"Constant State Data\n");
	fprint_state_data(file,cfr->state,fr->interf);
}		/*end fprint_ConstantFlowRegion_data*/


LOCAL	void	DestroyConstantFlowRegion(
	FlowSpecifiedRegion	*fsr)
{
	ConstantFlowRegion *cfr = (ConstantFlowRegion*)fsr;
	free(cfr->state);
	free(cfr);
}		/*end DestroyConstantFlowRegion*/

LOCAL	void	DestroySkipComponentRegion(
	FlowSpecifiedRegion	*fsr)
{
	free(fsr);
}		/*end DestroySkipComponentRegion*/

LOCAL	FlowSpecifiedRegion *FSR_for_comp(
	COMPONENT comp,
	Front     *fr,
	INTERFACE *intfc)
{
	FlowSpecifiedRegion *fsr, *head;

	if (fr != NULL)
	{
	    head = Fsr_list(fr);
	    intfc = fr->interf;
	}
	else
	    head = AddToFsrList(NULL)->next;
	for (fsr = head; fsr != NULL; fsr = fsr->next)
	{
	    if (ComponentsMatch(fsr,comp,fsr->comp,intfc))
		return fsr;
	}
	return NULL;
}		/*end FSR_for_comp*/

#if defined(THREED)
EXPORT	boolean	f_sort_bond_tris(
	INTERFACE	*intfc)
{
	BOND            *b;
	CURVE           **c;
	Locstate        s0, s1;
	int             i, N;
	size_t          sizest;
	static Locstate stemp = NULL;

	if (!i_sort_bond_tris(intfc))
	    return NO;

	sizest = size_of_state(intfc);
	if ((sizest == 0) || !interpolate_intfc_states(intfc))
	    return YES;

	if (stemp == NULL)
	    scalar(&stemp,size_of_state(intfc));

	for (c = intfc->curves; c && *c; ++c)
	{
	    N = (int) size_of_pointers(Btris((*c)->first));
	    for (b = (*c)->first; b != (*c)->last; b = b->next)
	    {
		for (i = 0; i < N; ++i)
		{
		    s0 = left_end_btri_state(Btris(b)[i]);
		    s1 = left_start_btri_state(Btris(b->next)[i]);
		    if (s0 != s1)
		    {
			bi_interpolate_intfc_states(intfc,0.5,0.5,
						    Coords(b->end),s0,
						    Coords(b->end),s1,
						    stemp);
			left_start_btri_state(Btris(b->next)[i]) = s0;
			ft_assign(s0,stemp,sizest);
		    }
		    s0 = right_end_btri_state(Btris(b)[i]);
		    s1 = right_start_btri_state(Btris(b->next)[i]);
		    if (s0 != s1)
		    {
			bi_interpolate_intfc_states(intfc,0.5,0.5,
						    Coords(b->end),s0,
						    Coords(b->end),s1,
						    stemp);
			right_start_btri_state(Btris(b->next)[i]) = s0;
			ft_assign(s0,stemp,sizest);
		    }
		}
	    }
	}
	return YES;
}		/*end f_sort_bond_tris*/

/*#bjet2 */
EXPORT boolean  f_assign_btri_states(
        BOND_TRI *newbtri,
        BOND_TRI *btri)
{
INTERFACE       *cintfc = current_interface();
size_t          sizest = size_of_state(cintfc);        

        if (sizest == 0)
        { 
            left_start_btri_state(newbtri) = NULL;            
            right_start_btri_state(newbtri) = NULL;
            left_end_btri_state(newbtri) = NULL;            
            right_end_btri_state(newbtri) = NULL;
            return YES;
        }
	
	if (copy_intfc_states() == YES)
	{
            ft_assign(left_start_btri_state(newbtri),left_start_btri_state(btri),sizest);
            ft_assign(left_end_btri_state(newbtri),left_end_btri_state(btri),sizest);
            ft_assign(right_start_btri_state(newbtri),right_start_btri_state(btri),sizest);
            ft_assign(right_end_btri_state(newbtri),right_end_btri_state(btri),sizest);
        }
	return YES;
}

#endif /* defined(THREED) */
