#ifndef CHOMBODOMAINBC_H
#define CHOMBODOMAINBC_H

#include "RefCountedPtr.H"
#include <VCELL/VarContext.h>
#include <VCELL/Structure.h>
#include <DirichletPoissonDomainBC.H>
#include <NeumannPoissonDomainBC.H>

class ChomboSemiImplicitScheduler;
class Variable;
class Feature;
class ChomboDomainBCValue;

class ChomboDomainBCValue: public BaseBCValue
{
private:
	Feature* feature;
	Variable* var;
	ExpressionIndex bcExpIndex;
	
public:
  ChomboDomainBCValue();
  virtual ~ChomboDomainBCValue();               
  Real value(const RealVect& a_point, const RealVect& a_normal, const Real& a_time, const int& a_comp) const;
	void setBCValueInfo(Feature* f, Variable* v, ExpressionIndex ei);
};

class ChomboDomainBC: public BaseDomainBC
{
private:
	void getDomainBCInfo(const int&  a_idir, const Side::LoHiSide& a_side, ExpressionIndex& expIndex, BoundaryType& bcType) const;
	BaseDomainBC* getDomainBC(const int&  a_idir, const Side::LoHiSide& a_side);
	DirichletPoissonDomainBC dirichletBC_Value;
	NeumannPoissonDomainBC neumannBC_Value;
	// once set to function, you can never go back to
	// value, thus define them separately
	DirichletPoissonDomainBC dirichletBC_Function;
	NeumannPoissonDomainBC neumannBC_Function;
	ChomboDomainBCValue *domainBCValue;
	
	ChomboSemiImplicitScheduler* semiImpScheduler;
	int iphase;
	int ivol;
	Feature* feature;
	Variable* var;
	
public:
   ChomboDomainBC(ChomboSemiImplicitScheduler* scheduler, int iphase, int ivol, Feature* f, Variable* var);
  ~ChomboDomainBC();

	void getFaceFlux(BaseFab<Real>&        a_faceFlux,
									 const BaseFab<Real>&  a_phi,
									 const RealVect&       a_probLo,
									 const RealVect&       a_dx,
									 const int&            a_idir,
									 const Side::LoHiSide& a_side,
									 const DataIndex&      a_dit,
									 const Real&           a_time,
									 const bool&           a_useHomogeneous);

  // virtual LayoutData<BaseIVFAB<VoFStencil> >* getFluxStencil(int ivar)= 0;

	void getFluxStencil(VoFStencil&      a_stencil,
											const VolIndex&        a_vof,
											const int&             a_comp,
											const RealVect&        a_dx,
											const int&             a_idir,
											const Side::LoHiSide&  a_side,
											const EBISBox&         a_ebisBox);

	void getFluxStencil(VoFStencil&      a_stencil,
											const FaceIndex&       a_face,
											const int&             a_comp,
											const RealVect&        a_dx,
											const int&             a_idir,
											const Side::LoHiSide&  a_side,
											const EBISBox&         a_ebisBox);

  ///
  /**
     This is used by the elliptic solvers to get
     the flux of stuff through domain faces.
   */
  void getFaceFlux(Real& a_faceFlux,
									const VolIndex&       a_vof,
									const int&            a_comp,
									const EBCellFAB&      a_phi,
									const RealVect&       a_probLo,
									const RealVect&       a_dx,
									const int&            a_idir,
									const Side::LoHiSide& a_side,
									const DataIndex&      a_dit,
									const Real&           a_time,
									const bool&           a_useHomogeneous);


  ///
  /**
     A query of whether a_jvof is Dirichlet Domain boundary to a_ivof. a_jvof is the ghost vof; a_ivof is the valid computational vof.
     For now this is mainly used in form_matrix, which needs to know the type of BC at the boundary.
     Default implementation is MayDay::Error();
   */
  bool isDirichletDom(const VolIndex&   a_ivof,
											const VolIndex&   a_jvof,
											const EBCellFAB&  a_phi) const;


  ///
  /**
     This is used by the elliptic solvers to get
     the flux of stuff through domain faces.
   */
  void getInhomFaceFlux(Real& a_faceFlux,
												const VolIndex&       a_vof,
												const int&            a_comp,
												const EBCellFAB&      a_phi,
												const RealVect&       a_probLo,
												const RealVect&       a_dx,
												const int&            a_idir,
												const Side::LoHiSide& a_side,
												const DataIndex&      a_dit,
												const Real&           a_time);

  ///
  /**
     This is used by the projections to get
     a) grad(phi) at domain faces.
   */
  void getFaceGradPhi(Real& a_faceFlux,
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
											const bool&           a_useHomogeneous);

  ///
  /**
     This is used by the projections to get
     velocity at domain faces.
   */
 void getFaceVel(Real& a_faceFlux,
									const FaceIndex&      a_vof,
									const EBFluxFAB&      a_vel,
									const RealVect&       a_probLo,
									const RealVect&       a_dx,
									const int&            a_idir,
									const int&            a_icomp,
									const Real&           a_time,
									const Side::LoHiSide& a_side,
									const bool&           a_doDivFreeOutflow);	
};

class ChomboDomainBCFactory: public BaseDomainBCFactory
{
public:
	ChomboDomainBCFactory(ChomboSemiImplicitScheduler* scheduler, int iphase, int ivol, Feature* f, Variable* var);
 	~ChomboDomainBCFactory();

  virtual ChomboDomainBC* create(
		  const ProblemDomain& a_domain,
          const EBISLayout&    a_layout,
          const RealVect&      a_dx);

private:
	ChomboSemiImplicitScheduler* semiImpScheduler;
	int iphase;
	int ivol;
	Feature* feature;
	Variable* var;
};

#endif
