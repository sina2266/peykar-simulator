#ifndef CHOMBOSEMIIMPLICITSECHDULER_H
#define CHOMBOSEMIIMPLICITSECHDULER_H

#include <VCELL/ChomboScheduler.h>
#include <BiCGStabSolver.H>
#include <EBBackwardEuler.H>
#include <AggStencil.H>

class EBLevelGrid;
class EBQuadCFInterp;

class ChomboSemiImplicitScheduler : public ChomboScheduler {

	friend class ChomboEBBC;
	friend class FastSystemExpression;
	
public:
	ChomboSemiImplicitScheduler(SimulationExpression* sim, ChomboSpec* chomboSpec);
	~ChomboSemiImplicitScheduler();


protected:
	void iterate();
	void initValues();

private:
	Vector< Vector< Vector<LevelData<EBCellFAB>*> > > volSolnWorkspace;

	Vector< Vector< Vector<LevelData<EBCellFAB>*> > > volSource;
	Vector< Vector< Vector<LevelData<EBCellFAB>*> > > volSourceWorkspace;
	Vector< Vector< Vector<LevelData<EBCellFAB>*> > > volSolnOld;
	Vector< Vector< Vector<LevelData<EBCellFAB>*> > > volSolnOldWorkspace;
	
	Vector< Vector< RefCountedPtr< LevelData<BaseIVFAB<Real> > > > > memSolnOld;
	
	Vector< Vector< Vector<RefCountedPtr<EBBackwardEuler> > > > ebBEIntegratorList;
	Vector< Vector< Vector< RefCountedPtr<AMRMultiGrid<LevelData<EBCellFAB> > > > > > ebMlgSolver;

	//this is the stencil that extrapolates data to the irregular boundary
	Vector< Vector< Vector< LayoutData< RefCountedPtr< AggStencil< EBCellFAB, BaseIVFAB<Real> > > >* > > > extrapStencils;
	
	void updateSource();

	IntVect numGhostSource;

	void defineSolver();
	void setInitialConditions();
//	void createVariableCoeffOpFactory(RefCountedPtr<EBConductivityOpFactory>& a_factory, int a_ivol, int a_ivar);
	void createConstantCoeffOpFactory(RefCountedPtr<EBAMRPoissonOpFactory>& a_factory, int iphase, int ivol, Feature* feature, int ivar);
	void getEBLGAndQuadCFI(Vector<EBLevelGrid>  & ebLevelGrids, Vector<RefCountedPtr<EBQuadCFInterp> >& quadCFInterp, int iphase, int ivol, int ncomp =1);

	void extrapolateDataToBoundary();
	void initStencils();
	void getExtrapStencils(Vector<RefCountedPtr<BaseIndex  > >& a_destVoFs,
	                  Vector<RefCountedPtr<BaseStencil> >& a_stencils,
	                  const IntVectSet & a_cfivs,
	                  const DataIndex& a_dit, int iphase,
	                  int ivol, int ilev, Real a_dx);
	void solveFastSystem();

	void printOpMatrix();
	void printOpMatrix(int iphase, int ivol, int ilev, int ivar, const string& varName);
};

#endif
