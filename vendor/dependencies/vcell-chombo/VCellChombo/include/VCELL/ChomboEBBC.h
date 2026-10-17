#ifndef CHOMBOEBBC_H
#define CHOMBOEBBC_H

#include <DirichletPoissonEBBC.H>
#include <NeumannPoissonEBBC.H>

#include "Structure.h"

class ChomboSemiImplicitScheduler;
class Variable;
class Feature;

class ChomboEBBC : public BaseEBBC
{
private:
	ChomboSemiImplicitScheduler* semiImpScheduler;
	int iphase;
	int ivol;
	Feature* iFeature;
	int ivar;
	Variable* var;

	DirichletPoissonEBBC* dirichletEbBc;
	NeumannPoissonEBBC* neumannEbBc;
	BaseEBBC* ebbc;

	ProblemDomain domain;
  EBISLayout    layout;
  RealVect dx;
  IntVect ghostCellsPhi;
  IntVect ghostCellsRhs;

	// ===============================================
	// the following is copied from NeumannPoissonEBBC
	// ===============================================
public:
	ChomboEBBC(ChomboSemiImplicitScheduler* scheduler, int iphase, int ivol, Feature* f, int ivar,
							const ProblemDomain& a_domain,
							const EBISLayout&    a_layout,
							const RealVect&      a_dx,
							const IntVect*       a_ghostCellsPhi=0,
							const IntVect*       a_ghostCellsRhs=0);

  virtual ~ChomboEBBC();

  virtual void define(const LayoutData<IntVectSet>& a_cfivs, const Real& a_factor);
	
  //no homogeneous stencil.
  virtual LayoutData<BaseIVFAB<VoFStencil> >* getFluxStencil(int ivar);
	
  virtual void applyEBFlux(EBCellFAB&                    a_lphi,
                           const EBCellFAB&              a_phi,
                           VoFIterator&                  a_vofit,
                           const LayoutData<IntVectSet>& a_cfivs,
                           const DataIndex&              a_dit,
                           const RealVect&               a_probLo,
                           const RealVect&               a_dx,
                           const Real&                   a_factor,
                           const bool&                   a_useHomogeneous,
                           const Real&                   a_time);
};

class ChomboEBBCFactory: public BaseEBBCFactory
{
public:
  ChomboEBBCFactory(ChomboSemiImplicitScheduler* scheduler, int iphase, int ivol, Feature* f, int ivar);
  virtual ~ChomboEBBCFactory();
  virtual ChomboEBBC* create(const ProblemDomain& a_domain,
														const EBISLayout&    a_layout,
														const RealVect&      a_dx,
														const IntVect*       a_ghostCellsPhi=0,
														const IntVect*       a_ghostCellsRhs=0);
private:
  ChomboSemiImplicitScheduler* semiImpScheduler;
	int iphase;
	int ivol;
	Feature* iFeature;
	int ivar;
};
#endif /* CHOMBOFLUXEBBC_H */
