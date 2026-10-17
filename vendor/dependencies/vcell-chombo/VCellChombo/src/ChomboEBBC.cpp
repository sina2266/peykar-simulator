#include <VCELL/ChomboEBBC.h>
#include <VCELL/ChomboIF.h>
#include <VCELL/ChomboScheduler.h>
#include <VCELL/VCellModel.h>
#include <VCELL/SimTool.h>
#include <VCELL/Feature.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <VCELL/VolumeVarContextExpression.h>

#include <VCELL/ChomboSemiImplicitScheduler.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/ChomboGeometry.h>
#include <VCELL/ConnectedComponent.h>
#include <sstream>
using std::stringstream;

#include <VCELL/Variable.h>
#include <VCELL/Membrane.h>

ChomboEBBC::ChomboEBBC(ChomboSemiImplicitScheduler* scheduler, int a_iphase, int a_ivol, Feature* f,
				int a_ivar,
				const ProblemDomain& a_domain,
				const EBISLayout&    a_layout,
				const RealVect&      a_dx,
				const IntVect*       a_ghostCellsPhi,
				const IntVect*       a_ghostCellsRhs)
	: semiImpScheduler(scheduler),
		iphase(a_iphase),
		ivol(a_ivol),
		iFeature(f),
		ivar(a_ivar),
		domain(a_domain),
		layout(a_layout),
		dx(a_dx),
		ghostCellsPhi(*a_ghostCellsPhi),
		ghostCellsRhs(*a_ghostCellsRhs)
{
	var = iFeature->getDefinedVariable(ivar);

	if (iFeature->getEbBcType() == BOUNDARY_VALUE)
	{
		dirichletEbBc = new DirichletPoissonEBBC(domain, layout, dx, &ghostCellsPhi, &ghostCellsRhs);
		dirichletEbBc->setValue(0);
		// Using second order stencil for Dirichlet ebbc
		// current first order LS weight calculation produces wrong results for dx<0.002 - Feb 2015
    // sometimes the code reduces the order anyway
		dirichletEbBc->setOrder(2);
		neumannEbBc = 0;
		ebbc = dirichletEbBc;
	}
	else
	{
		dirichletEbBc = 0;
		neumannEbBc = new NeumannPoissonEBBC(domain, layout, dx);
		neumannEbBc->setValue(0);
		ebbc = neumannEbBc;
	}
}

ChomboEBBC::~ChomboEBBC()
{
	delete neumannEbBc;
	delete dirichletEbBc;
}

void ChomboEBBC::define(const LayoutData<IntVectSet>& a_cfivs, const Real& a_factor)
{
	ebbc->define(a_cfivs, a_factor);
}

LayoutData<BaseIVFAB<VoFStencil> >* ChomboEBBC::getFluxStencil(int ivar)
{
	return ebbc->getFluxStencil(ivar);
}

void ChomboEBBC::applyEBFlux(EBCellFAB&  a_lphi,
														const EBCellFAB&              a_phi,
														VoFIterator&                  a_vofit,
														const LayoutData<IntVectSet>& a_cfivs,
														const DataIndex&              a_dit,
														const RealVect&               a_probLo,
														const RealVect&               a_dx,
														const Real&                   a_factor,
														const bool&                   a_useHomogeneous,
														const Real&                   a_time)
{
	const EBISBox& ebisBox = a_phi.getEBISBox();
	const ProblemDomain& domain = ebisBox.getDomain();
	int ilev = semiImpScheduler->findLevel(domain);
	assert(ilev >= 0);
//	pout() << "Entry(ChomboEBBC::applyEBFlux) Variable " << var->getName() << " at level " << ilev << " in feature " << iFeature->getName() << endl;
  
	int totalVolumes = semiImpScheduler->phaseVolumeList[0].size() + semiImpScheduler->phaseVolumeList[1].size();
	Vector<ConnectedComponent*>& adjacentVolumes = semiImpScheduler->phaseVolumeList[iphase][ivol]->adjacentVolumes;
	// find the membrane
	for (int j = 0; j < adjacentVolumes.size(); j ++)
	{
		int jphase = adjacentVolumes[j]->phase;
		int jvol = adjacentVolumes[j]->volumeIndexInPhase;

		int currentMembraneID = 0;
		if (iphase == 0)
		{
			currentMembraneID = ivol * totalVolumes + jvol;
		} 
		else if (iphase == 1)
		{
			currentMembraneID = jvol * totalVolumes + ivol;
		}

		Feature* jFeature = adjacentVolumes[j]->feature;
		Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(iFeature, jFeature);
		for (a_vofit.reset(); a_vofit.ok(); ++a_vofit)
		{
			const VolIndex& vof = a_vofit();
			// get the type of boundary condition
			int membraneID = (*semiImpScheduler->irregularPointMembraneIDs[iphase][ivol][ilev])[a_dit](vof, 0);
			if (membraneID != currentMembraneID) {
				continue;
			}
	
			if (iFeature->getEbBcType(membrane) == BOUNDARY_VALUE)
			{								
				double diriValue = (*(semiImpScheduler->extrapValues)[iphase][ivol][ilev])[a_dit](vof, ivar);
				dirichletEbBc->setValue(diriValue);
				dirichletEbBc->applyEBFluxPoint(vof, a_lphi, a_phi, a_vofit,
								a_cfivs, a_dit, a_probLo, a_dx, a_factor, a_useHomogeneous,
												 a_time);
			}
			else
			{
				neumannEbBc->applyEBFluxPoint(vof, a_lphi, a_phi, a_vofit,
								a_cfivs, a_dit, a_probLo, a_dx, a_factor, a_useHomogeneous,
												 a_time);
			}
		}
	}
//	pout() << "Exist(ChomboEBBC::applyEBFlux)" << endl;
}

ChomboEBBCFactory::ChomboEBBCFactory(ChomboSemiImplicitScheduler* scheduler, int a_iphase, int a_ivol,
				Feature* f, int a_ivar)
	: semiImpScheduler(scheduler),
		iphase(a_iphase),
		ivol(a_ivol),
		iFeature(f),
		ivar(a_ivar)
{
}

ChomboEBBCFactory::~ChomboEBBCFactory()
{
}

ChomboEBBC* ChomboEBBCFactory::create(const ProblemDomain& a_domain,
											const EBISLayout&    a_layout,
											const RealVect&      a_dx,
											const IntVect*       a_ghostCellsPhi /*=0*/,
											const IntVect*       a_ghostCellsRhs /*=0*/)
{
  ChomboEBBC* fresh = new ChomboEBBC(semiImpScheduler, iphase, ivol, iFeature, ivar,
					a_domain, a_layout, a_dx, a_ghostCellsPhi, a_ghostCellsRhs);
  return fresh;
}