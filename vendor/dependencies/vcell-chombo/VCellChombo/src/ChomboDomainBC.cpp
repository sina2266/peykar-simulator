
#include "BoxIterator.H"
#include <VCELL/ChomboDomainBC.h>
#include "EBArith.H"
#include "Stencils.H"
#include "VoFIterator.H"
#include <VCELL/Feature.h>
#include <VCELL/ChomboSemiImplicitScheduler.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/Variable.h>

#include <assert.h>

ChomboDomainBCValue::ChomboDomainBCValue()
{
}

ChomboDomainBCValue::~ChomboDomainBCValue()
{
}

void ChomboDomainBCValue::setBCValueInfo(Feature* f, Variable* v, ExpressionIndex ei)
{
	feature = f;
	var = v;
	bcExpIndex = ei;
} 

Real ChomboDomainBCValue::value(const RealVect& a_point,
                        const RealVect& a_normal,
                        const Real&     a_time,
                        const int&      a_comp) const
{
	double vectValues[4];
	vectValues[0] = a_time;
	vectValues[1] = a_point[0];
	vectValues[2] = a_point[1];
	vectValues[3] = SpaceDim > 2 ? a_point[2] : 0.5;
	VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)var->getVarContext();
	double eval = varContextExp->evaluateExpression(bcExpIndex, vectValues);
  return eval;
}

ChomboDomainBC::ChomboDomainBC(ChomboSemiImplicitScheduler* scheduler, int a_iphase, int a_ivol, 
				Feature* f, Variable* a_var)
	: semiImpScheduler(scheduler),
		iphase(a_iphase),
		ivol(a_ivol),
		feature(f),
		var(a_var)
{
	domainBCValue = new ChomboDomainBCValue();
	RefCountedPtr<BaseBCValue> domainBCValueRef(domainBCValue);
	dirichletBC_Function.setFunction(domainBCValueRef);
	neumannBC_Function.setFunction(domainBCValueRef);
}

ChomboDomainBC::~ChomboDomainBC() 
{
}

void ChomboDomainBC::getDomainBCInfo(const int&  a_idir, const Side::LoHiSide& a_side, 
				ExpressionIndex& expIndex, BoundaryType& bcType) const
{
	if (a_idir == 0)
	{
		if (a_side == Side::Lo)
		{
			expIndex = BOUNDARY_XM_EXP;
			bcType = feature->getXmBoundaryType();
		}
		else if (a_side == Side::Hi)
		{
			expIndex = BOUNDARY_XP_EXP;
			bcType = feature->getXpBoundaryType();
		}
	}
	else if (a_idir == 1)
	{
		if (a_side == Side::Lo)
		{
			expIndex = BOUNDARY_YM_EXP;
			bcType = feature->getYmBoundaryType();
		}
		else if (a_side == Side::Hi)
		{
			expIndex = BOUNDARY_YP_EXP;
			bcType = feature->getYpBoundaryType();
		}
	}
	else if (a_idir == 2)
	{
		if (a_side == Side::Lo)
		{
			expIndex = BOUNDARY_ZM_EXP;
			bcType = feature->getZmBoundaryType();
		}
		else if (a_side == Side::Hi)
		{
			expIndex = BOUNDARY_ZP_EXP;
			bcType = feature->getZpBoundaryType();
		}
	}
}

BaseDomainBC* ChomboDomainBC::getDomainBC(const int&  a_idir, const Side::LoHiSide& a_side)
{
	ExpressionIndex expIndex;
	BoundaryType bcType;
	getDomainBCInfo(a_idir, a_side, expIndex, bcType);
	double bcValue;
	bool bConstBC = true;
	try
	{
		bcValue = semiImpScheduler->getExpressionConstantValue(var, expIndex, feature);
	}
	catch (...)
	{
		bConstBC = false;
	}	
	if (bcType == BOUNDARY_VALUE)
	{
		if (bConstBC)
		{
			dirichletBC_Value.setValue(bcValue);
			return &dirichletBC_Value;
		}
		else
		{
			domainBCValue->setBCValueInfo(feature, var, expIndex);
			return &dirichletBC_Function;
		}
	}
	else if (bcType == BOUNDARY_FLUX)
	{
		if (bConstBC)
		{
			neumannBC_Value.setValue(bcValue);
			return &neumannBC_Value;
		}
		else
		{
			domainBCValue->setBCValueInfo(feature, var, expIndex);
			return &neumannBC_Function;
		}
	}
	else if (bcType == BOUNDARY_PERIODIC)
	{
		return NULL;
	}
}

void ChomboDomainBC::getFaceVel(Real& a_faceFlux,
           const FaceIndex&      a_face,
           const EBFluxFAB&      a_vel,
           const RealVect&       a_probLo,
           const RealVect&       a_dx,
           const int&            a_idir,
           const int&            a_icomp,
           const Real&           a_time,
           const Side::LoHiSide& a_side,
           const bool&           a_doDivFreeOutflow) {

	BaseDomainBC* domainBC = getDomainBC(a_idir, a_side);
	if (domainBC != NULL)
	{
		domainBC->getFaceVel(a_faceFlux, a_face, a_vel, a_probLo, a_dx, a_idir, a_icomp, a_time, a_side, a_doDivFreeOutflow);
	}
}

///
/**
   This is called by InflowOutflowPoissonDomainBC::getFaceFlux,
   which is called by EBAMRPoissonOp::applyDomainFlux for applyOp in reg cells.
   For Dirichlet boundary conditions on pressure (usually at outflow) in projection.
*/
void ChomboDomainBC::getFaceFlux(BaseFab<Real>& a_faceFlux,
																		const BaseFab<Real>&  a_phi,
																		const RealVect&       a_probLo,
																		const RealVect&       a_dx,
																		const int&            a_idir,
																		const Side::LoHiSide& a_side,
																		const DataIndex&      a_dit,
																		const Real&           a_time,
																		const bool&           a_useHomogeneous)
{
	BaseDomainBC* domainBC = getDomainBC(a_idir, a_side);
	if (domainBC != NULL)
	{
		domainBC->getFaceFlux(a_faceFlux, a_phi, a_probLo, a_dx, a_idir, a_side, a_dit, a_time, a_useHomogeneous);
	}
}

///
/**
This is called by InflowOutflowPoissonDomainBC::getFaceFlux,
which is called by EBAMRPoissonOp::applyOp when EB x domain.
For domain boundary conditions on pressure in projections.
*/
void ChomboDomainBC::getFaceFlux(Real&                 a_faceFlux,
																	const VolIndex&       a_vof,
																	const int&            a_comp,
																	const EBCellFAB&      a_phi,
																	const RealVect&       a_probLo,
																	const RealVect&       a_dx,
																	const int&            a_idir,
																	const Side::LoHiSide& a_side,
																	const DataIndex&      a_dit,
																	const Real&           a_time,
																	const bool&           a_useHomogeneous)
{
	BaseDomainBC* domainBC = getDomainBC(a_idir, a_side);
	if (domainBC != NULL)
	{
		domainBC->getFaceFlux(a_faceFlux, a_vof, a_comp, a_phi, a_probLo, a_dx, a_idir, a_side, a_dit, a_time, a_useHomogeneous);
	}
}

///
/**
   This is called by InflowOutflowPoissonDomainBC::getFaceGradPhi
   when enforcing boundary gradients in projection (called by macEnforceGradientBC).
   if true, then just get the inhomogeneous value;
   if false, then use whole stencil
   does matter because this is not only called for the pressure at operator constructor time (defineStencils)
   but also by macEnforceGradientBC which needs the whole stencil (can stencilize this later)
*/
void ChomboDomainBC::getFaceGradPhi(Real&  a_faceFlux,
																		const FaceIndex&      a_face,
																		const int&            a_comp,
																		const EBCellFAB&      a_phi,
																		const RealVect&       a_probLo,
																		const RealVect&       a_dx,
																		const int&            a_idir,
																		const Side::LoHiSide& a_side,
																		const DataIndex&      a_dit,
																		const Real&           a_time,
																		const bool&           a_useAreaFrac,
																		const RealVect&       a_centroid,
																		const bool&           a_useHomogeneous)
{
  BaseDomainBC* domainBC = getDomainBC(a_idir, a_side);
	if (domainBC != NULL)
	{
		domainBC->getFaceGradPhi(a_faceFlux, a_face, a_comp, a_phi, a_probLo, a_dx, 
						a_idir, a_side, a_dit, a_time, a_useAreaFrac, a_centroid, a_useHomogeneous);
	}
}

///
/**
   This is called by InflowOutflowPoissonDomainBC::getFaceFlux,
   which is called by EBAMRPoissonOp::applyOp when EB x domain.
   For domain boundary conditions on pressure in projections.
*/
void ChomboDomainBC::getInhomFaceFlux(Real& a_faceFlux,
																			const VolIndex&       a_vof,
																			const int&            a_comp,
																			const EBCellFAB&      a_phi,
																			const RealVect&       a_probLo,
																			const RealVect&       a_dx,
																			const int&            a_idir,
																			const Side::LoHiSide& a_side,
																			const DataIndex&      a_dit,
																			const Real&           a_time)
{
  BaseDomainBC* domainBC = getDomainBC(a_idir, a_side);
	if (domainBC != NULL)
	{
		domainBC->getInhomFaceFlux(a_faceFlux, a_vof, a_comp, a_phi, a_probLo, a_dx, a_idir, a_side, a_dit, a_time);
	}
}

void ChomboDomainBC::getFluxStencil(VoFStencil&      a_stencil,
																		const VolIndex&        a_vof,
																		const int&             a_comp,
																		const RealVect&        a_dx,
																		const int&             a_idir,
																		const Side::LoHiSide&  a_side,
																		const EBISBox&         a_ebisBox)
{
  BaseDomainBC* domainBC = getDomainBC(a_idir, a_side);
	if (domainBC != NULL)
	{
		domainBC->getFluxStencil(a_stencil, a_vof, a_comp, a_dx, a_idir, a_side, a_ebisBox);
	}
}

void ChomboDomainBC::getFluxStencil(VoFStencil&      a_stencil,
																		const FaceIndex&       a_face,
																		const int&             a_comp,
																		const RealVect&        a_dx,
																		const int&             a_idir,
																		const Side::LoHiSide&  a_side,
																		const EBISBox&         a_ebisBox)
{
  BaseDomainBC* domainBC = getDomainBC(a_idir, a_side);
	if (domainBC != NULL)
	{
		domainBC->getFluxStencil(a_stencil, a_face, a_comp, a_dx, a_idir, a_side, a_ebisBox);
	}
}

bool ChomboDomainBC::isDirichletDom(const VolIndex&   a_ivof,
                      const VolIndex&   a_jvof,
                      const EBCellFAB&  a_phi) const
{
	FaceIndex face(a_ivof,a_jvof);
  Side::LoHiSide a_side = (Side::LoHiSide)face.faceSign(a_ivof);
  int a_idir = face.direction();
	ExpressionIndex expIndex;
	BoundaryType bcType;
	getDomainBCInfo(a_idir, a_side, expIndex, bcType);
	return bcType == BOUNDARY_VALUE;
}

ChomboDomainBCFactory::ChomboDomainBCFactory(ChomboSemiImplicitScheduler* scheduler, int a_iphase, 
				int a_ivol, Feature* f, Variable* a_var)
	: semiImpScheduler(scheduler),
		iphase(a_iphase),
		ivol(a_ivol),
		feature(f),
		var(a_var)
{
}

ChomboDomainBCFactory::~ChomboDomainBCFactory()
{
}

 ChomboDomainBC* ChomboDomainBCFactory::create(const ProblemDomain& a_domain,
  	        	const EBISLayout&    a_layout,
    	      	const RealVect&      a_dx)
{
	ChomboDomainBC* newBC = new ChomboDomainBC(semiImpScheduler, iphase, ivol, feature, var);
  return newBC;
}
