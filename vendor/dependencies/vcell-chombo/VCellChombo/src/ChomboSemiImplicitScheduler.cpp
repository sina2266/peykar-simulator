#include <EBAMRDataOps.H>
#include <NeumannPoissonEBBC.H>
#include <DirichletPoissonEBBC.H>
#include <EBAMRPoissonOpFactory.H>

//#include <VCELL/ChomboLevelRedist.H>
#include <VCELL/ChomboSemiImplicitScheduler.h>
#include <VCELL/Variable.h>
#include <VCELL/VarContext.h>
#include <VCELL/ChomboGeometry.h>
#include <VCELL/ChomboIF.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/SimTool.h>
#include <VCELL/Feature.h>
#include <VCELL/VCellModel.h>
#include <VCELL/Membrane.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/FastSystemExpression.h>
#include <VCELL/ChomboDomainBC.h>
#include <VCELL/ChomboEBBC.h>
#include <VCELL/ConnectedComponent.h>
#include <VCELL/FastSystemExpression.h>

#include <time.h>
#include <sstream>
#include <fstream>
#include <string.h>
using std::stringstream;

static int numSmooth = 3;
static int numMGCycles = 1;
static int maxIter = 400;
//static Real tolerance = 1.0e-9;
static Real hang = 1.0e-15;
static Real normThresh = 1.0e-30;
static int maxCoarsen = -1;
static int numPreCondIters = 4;
static int relaxType = 2;

ChomboSemiImplicitScheduler::ChomboSemiImplicitScheduler(SimulationExpression* sim, ChomboSpec* chomboSpec)
	: ChomboScheduler(sim, chomboSpec)
{
	pout() << "************* Using Chombo SemiImplicit *****************" << endl;
	numGhostSource = IntVect::Zero;
}

ChomboSemiImplicitScheduler::~ChomboSemiImplicitScheduler()
{
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol++)	{
			for (int ilev = 0; ilev < numLevels; ilev++) {
				delete volSoln[iphase][ivol][ilev];
				delete volSolnWorkspace[iphase][ivol][ilev];
				delete volSolnOld[iphase][ivol][ilev];
				delete volSolnOldWorkspace[iphase][ivol][ilev];
				delete volSource[iphase][ivol][ilev];
				delete volSourceWorkspace[iphase][ivol][ilev];
				delete extrapStencils[iphase][ivol][ilev];
			}
			volSoln[iphase][ivol].clear();
			volSolnWorkspace[iphase][ivol].clear();
			volSolnOld[iphase][ivol].clear();
			volSolnOldWorkspace[iphase][ivol].clear();
			volSource[iphase][ivol].clear();
			volSourceWorkspace[iphase][ivol].clear();
			extrapStencils[iphase][ivol].clear();
		}
		volSoln[iphase].clear();
		volSolnWorkspace[iphase].clear();
		volSolnOld[iphase].clear();
		volSolnOldWorkspace[iphase].clear();
		volSource[iphase].clear();
		volSourceWorkspace[iphase].clear();
		extrapStencils[iphase].clear();
	}
	volSoln.clear();
	volSolnWorkspace.clear();
	volSolnOld.clear();
	volSolnOldWorkspace.clear();
	volSource.clear();
	volSourceWorkspace.clear();
	extrapStencils.clear();
	
	memSoln.clear();
	memSolnOld.clear();
}

void ChomboSemiImplicitScheduler::initValues()
{
	const char* methodName = "(ChomboSemiImplicitScheduler::initValues())";
	pout() << "Entry " << methodName << endl;
	setInitialConditions();

	pout() << "Initializing integrators and solvers" << endl;
	ebBEIntegratorList.resize(NUM_PHASES);
	ebMlgSolver.resize(NUM_PHASES);
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase) {
		ebBEIntegratorList[iphase].resize(phaseVolumeList[iphase].size());
		ebMlgSolver[iphase].resize(phaseVolumeList[iphase].size());
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol)
		{
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;

			int numDefinedVars = feature->getNumDefinedVariables();
			ebBEIntegratorList[iphase][ivol].resize(numDefinedVars);
			ebMlgSolver[iphase][ivol].resize(numDefinedVars);
		}
	}
	initStencils();
	pout() << "Exit " << methodName << endl;
}

void ChomboSemiImplicitScheduler::iterate() {
	const char* methodName = "(ChomboSemiImplicitScheduler::iterate)";
	pout() << "Entry " << methodName << endl;

	static bool bFirstTime = true;
	pout()  << endl << "time = " << simulation->getTime_sec() << endl;
	EBAMRPoissonOp::setOperatorTime(simulation->getTime_sec());

	if (bFirstTime  /*|| (!m_params.m_constCoeff)*/)
	{
		defineSolver();
	}
	double dt = simulation->getDT_sec();
	updateSource();

	// loop for elliptic variables
	if (simulation->hasElliptic())
	{
		pout() << "Solving elliptic variables starting" << endl;
		for (int iphase = 0; iphase < NUM_PHASES; ++ iphase) {
			for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
				Feature* feature = phaseVolumeList[iphase][ivol]->feature;

				int numDefinedVars = feature->getNumDefinedVariables();

				for(int ivar = 0; ivar < numDefinedVars; ++ ivar) {
					Variable* var = feature->getDefinedVariable(ivar);
					if (!var->isElliptic())
					{
						continue;
					}
					pout() << "solving elliptic variable '" << var->getName() << "'" << endl;
					Interval zeroint(0,0);
					Interval ivarint(ivar, ivar);

					//solver is for a single variable.  copy solution and rhs to scratch space
					for(int ilev = 0; ilev < numLevels; ++ ilev)
					{
						volSoln[iphase][ivol][ilev]->copyTo(ivarint, *volSolnWorkspace[iphase][ivol][ilev], zeroint);
						volSource[iphase][ivol][ilev]->copyTo(ivarint, *volSourceWorkspace[iphase][ivol][ilev], zeroint);
					}

					ebMlgSolver[iphase][ivol][ivar]->solve(volSolnWorkspace[iphase][ivol], volSourceWorkspace[iphase][ivol], numLevels - 1, 0);
					EBAMRDataOps::assign(volSoln[iphase][ivol], volSolnWorkspace[iphase][ivol], ivarint, zeroint);
				}
			}
		}
		pout() << "Solving elliptic variables complete" << endl;
	}

	if (simulation->hasParabolic())
	{
		pout() << "Solving parabolic variables starting" << endl;
		bool zeroPhi = true;
		bool kappaWeighted = true;
		for (int iphase = 0; iphase < NUM_PHASES; ++ iphase) {
			for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
				Feature* feature = phaseVolumeList[iphase][ivol]->feature;

				int numDefinedVars = feature->getNumDefinedVariables();

				for(int ivar = 0; ivar < numDefinedVars; ++ ivar) {
					Variable* var = feature->getDefinedVariable(ivar);
					if (!var->isDiffusing() || var->isElliptic())
					{
						continue;
					}
					pout() << "solving parabolic variable '" << var->getName() << "'" << endl;

					Interval zeroint(0,0);
					Interval ivarint(ivar, ivar);

					//solver is for a single variable.  copy solution and rhs to scratch space
					for(int ilev = 0; ilev < numLevels; ++ ilev)
					{
						volSolnOld[iphase][ivol][ilev]->copyTo(ivarint, *volSolnOldWorkspace[iphase][ivol][ilev], zeroint);
						volSoln[iphase][ivol][ilev]->copyTo(ivarint, *volSolnWorkspace[iphase][ivol][ilev], zeroint);
						volSource[iphase][ivol][ilev]->copyTo(ivarint, *volSourceWorkspace[iphase][ivol][ilev], zeroint);

						for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit)
						{
							const Box& currBox = vectGrids[ilev][dit()];
							const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
							const EBGraph& currEBGraph = currEBISBox.getEBGraph();
							IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);

							// kappa weight old solution, so that we don't have to divide volFrac in membrane flux
							for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit)
							{
								const VolIndex& vof = vofit();
								(*volSolnOldWorkspace[iphase][ivol][ilev])[dit()](vof, 0) *= currEBISBox.volFrac(vof);
							}
						}
					}
					
					ebBEIntegratorList[iphase][ivol][ivar]->oneStep(volSolnWorkspace[iphase][ivol], volSolnOldWorkspace[iphase][ivol],
								volSourceWorkspace[iphase][ivol], dt, 0, numLevels - 1, zeroPhi, kappaWeighted);
					
					//---------------------------------
					// update solution of tiny volume points
					//--------------------------------
					if (chomboSpec->getSmallVolfracThreshold() > 0)
					{
						if (bHasTinyVols)
						{
							for(int ilev = 0; ilev < numLevels; ++ ilev)
							{
								int ibox = -1;
								for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit)
								{
									++ ibox;
									pout() << "(iphase, ivol, ilev)=(" << iphase << "," << ivol << "," << ilev << "), ibox=" << ibox << endl;
									const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
									for (int i = 0; i < irregTinyVolNeighbors[iphase][ivol][ilev][ibox].size(); ++ i)
									{
										VolIndex& tinyVof = irregTinyVolNeighbors[iphase][ivol][ilev][ibox][i];
										double maxFaceArea = 0;
										FaceIndex bestFace;
										VolIndex bestNeighborVof;
										for (int idir = 0; idir < SpaceDim; idir++)
										{
											for (SideIterator sit; sit.ok(); ++sit)
											{
												Vector<FaceIndex> faces = currEBISBox.getFaces(tinyVof, idir, sit());
												if (faces.size() > 1)
												{
													stringstream ss;
													ss << "multiple faces found @" << tinyVof << ", idir " << idir << ", sit " << sit() << endl;
													throw ss.str();
												}
												if (faces.size() == 1)
												{
													const FaceIndex& face = faces[0];
													double faceArea = currEBISBox.areaFrac(face);
													if (maxFaceArea < faceArea)
													{
														maxFaceArea = faceArea;
														bestFace = face;
														bestNeighborVof = face.getVoF(sit());
													}
												}
												else
												{
													pout() << "no faces found @" << tinyVof << ", idir " << idir << ", sit " << sit() << endl;
												}
											}
										}

										if (maxFaceArea > 0)
										{
											// found a better neighbor
											double solNewBefore = (*volSolnWorkspace[iphase][ivol][ilev])[dit()](tinyVof, 0);
											double solOld = (*volSolnOld[iphase][ivol][ilev])[dit()](tinyVof, ivar);
											double neighborOld = (*volSolnOld[iphase][ivol][ilev])[dit()](bestNeighborVof, ivar);
											double neighborNew = (*volSolnWorkspace[iphase][ivol][ilev])[dit()](bestNeighborVof, 0);
											double solNewAfter = neighborNew;  // use best neighbor
											(*volSolnWorkspace[iphase][ivol][ilev])[dit()](tinyVof, 0) = solNewAfter;
											pout() << "tiny volume@" << tinyVof << "(old=" << solOld << ", new=" << solNewBefore
													<< "), best neighbor@" << bestNeighborVof << "(old=" << neighborOld << ", new=" << neighborNew
												<< "), solution changed from " << solNewBefore << " to " << solNewAfter << endl;
										}
									}
								}
							}
						}  // end if bHasTinyVols
					}
					EBAMRDataOps::assign(volSoln[iphase][ivol], volSolnWorkspace[iphase][ivol], ivarint, zeroint);
				}
			}
		}
		pout() << "Solving parabolic variables complete" << endl;
	}

	if (simulation->hasFastSystem())
	{
		solveFastSystem();
	}

	bFirstTime = false;
	
	// extrapolate to get latest extrapolated values
	// copy new to old
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase) {
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;

			if (feature->getNumDefinedVariables() > 0)
			{
				EBAMRDataOps::assign(volSolnOld[iphase][ivol], volSoln[iphase][ivol]);
			}

			if (iphase == phase0 && feature->getMemVarIndexesInAdjacentMembranes().size() > 0) {
				for(int ilev = 0; ilev < numLevels; ++ ilev)
				{
					memSoln[ivol][ilev]->copyTo(*memSolnOld[ivol][ilev]);
				}
			}
		}
	}
	extrapolateDataToBoundary();

	//printOpMatrix();
	pout() << "Exit " << methodName << endl;
}

void ChomboSemiImplicitScheduler::setInitialConditions() {
	const char* methodName = "(ChomboSemiImplicitScheduler:: setInitialConditions)";
	pout() << "Entry " << methodName << endl;

	// t, x, y, z, VAR, VAR_INSIDE, VAR_OUTSIDE, field data, parameters
	numSymbols = simulation->getNumSymbols();
	vectValues = new double[numSymbols];
	memset(vectValues, 0, numSymbols * sizeof(double));

	numUnknowns = 0;
	volSoln.resize(NUM_PHASES);
	volSolnWorkspace.resize(NUM_PHASES);
	volSolnOld.resize(NUM_PHASES);
	volSolnOldWorkspace.resize(NUM_PHASES);
	volSource.resize(NUM_PHASES);
	volSourceWorkspace.resize(NUM_PHASES);

	extrapValues.resize(NUM_PHASES);

	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		int numVols = phaseVolumeList[iphase].size();

		volSoln[iphase].resize(numVols);
		volSolnWorkspace[iphase].resize(numVols);
		volSolnOld[iphase].resize(numVols);
		volSolnOldWorkspace[iphase].resize(numVols);
		volSource[iphase].resize(numVols);
		volSourceWorkspace[iphase].resize(numVols);
		extrapValues[iphase].resize(numVols);
		
		if (iphase == phase0) {
			memSoln.resize(numVols);
			memSolnOld.resize(numVols);
		}
		
		for (int ivol = 0; ivol < numVols; ivol++)
		{
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;

			int numDefinedVolVars = feature->getNumDefinedVariables();
			int numDefinedMemVars = feature->getMemVarIndexesInAdjacentMembranes().size();
			if (numDefinedVolVars  == 0 && numDefinedMemVars == 0)
			{
				continue;
			}

			pout() << "iphase:" << iphase << ", ivol:" << ivol << ", initializing volume solution level data" << endl;

			if (numDefinedVolVars > 0) {
				volSoln[iphase][ivol].resize(numLevels);
				volSolnWorkspace[iphase][ivol].resize(numLevels);
				volSolnOld[iphase][ivol].resize(numLevels);
				volSolnOldWorkspace[iphase][ivol].resize(numLevels);
				volSource[iphase][ivol].resize(numLevels);
				volSourceWorkspace[iphase][ivol].resize(numLevels);
				extrapValues[iphase][ivol].resize(numLevels);
			}
			
			if (iphase == phase0) {
				memSoln[ivol].resize(numLevels);
				memSolnOld[ivol].resize(numLevels);
			}
		
			for (int ilev = 0; ilev < numLevels; ilev ++) {
				RefCountedPtr< LayoutData<IntVectSet> > irrSet = RefCountedPtr<LayoutData<IntVectSet> >(new LayoutData<IntVectSet>(vectGrids[ilev]));

				if (numDefinedVolVars > 0) {
					EBCellFactory        ebCellFactory(vectEbis[iphase][ivol][ilev]);
					volSoln[iphase][ivol][ilev] = new LevelData<EBCellFAB>(vectGrids[ilev], numDefinedVolVars, numGhostSoln, ebCellFactory);
					volSolnWorkspace[iphase][ivol][ilev] = new LevelData<EBCellFAB>(vectGrids[ilev], 1, numGhostSoln, ebCellFactory);
					volSolnOld[iphase][ivol][ilev] = new LevelData<EBCellFAB>(vectGrids[ilev], numDefinedVolVars, numGhostSoln, ebCellFactory);
					volSolnOldWorkspace[iphase][ivol][ilev] = new LevelData<EBCellFAB>(vectGrids[ilev], 1, numGhostSoln, ebCellFactory);
					volSource[iphase][ivol][ilev] = new LevelData<EBCellFAB>(vectGrids[ilev], numDefinedVolVars, numGhostSource, ebCellFactory);
					volSourceWorkspace[iphase][ivol][ilev] = new LevelData<EBCellFAB>(vectGrids[ilev], 1, numGhostSource, ebCellFactory);
				}
				
				// set up initial condition
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit) {
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];

					if (numDefinedVolVars > 0) {
						EBCellFAB& solnEBCellFAB = (*volSoln[iphase][ivol][ilev])[dit()];
						FArrayBox& solnFab = solnEBCellFAB.getFArrayBox();

						const IntVect& solnSize = solnFab.size();
						const int* solnLo = solnFab.loVect();
						Real* solnDataPtr = solnFab.dataPtr();

						EBCellFAB& solnOldEBCellFAB = (*volSolnOld[iphase][ivol][ilev])[dit()];
						FArrayBox& solnOldFab = solnOldEBCellFAB.getFArrayBox();
						Real* solnOldDataPtr = solnOldFab .dataPtr();

#if CH_SPACEDIM==3
						for (int k = 0; k < solnSize[2]; k ++) {
#endif
							for (int j = 0; j < solnSize[1]; j ++) { // phi has ghost point
								for (int i = 0; i < solnSize[0]; i ++) {
									IntVect gridIndex(D_DECL(i + solnLo[0], j + solnLo[1], k + solnLo[2]));

									RealVect coord = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
									vectValues[0] = simulation->getTime_sec();
									vectValues[1] = coord[0];
									vectValues[2] = coord[1];
									vectValues[3] = SpaceDim < 3 ? 0.5 : coord[2];

									// evaluate initial condition because chombo evaluates and advances the solution
									// at the cell centers so that is where these functions should be evaluated.
									for (int ivar = 0; ivar < numDefinedVolVars; ivar ++) {
										Variable* volVar = feature->getDefinedVariable(ivar);
										VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)volVar->getVarContext();
										double ic = varContextExp->evaluateExpression(INITIAL_VALUE_EXP, vectValues);
										int solnLocalIndex = getChomboBoxLocalIndex(solnSize, ivar, D_DECL(i, j, k));
										solnDataPtr[solnLocalIndex] = ic;
										solnOldDataPtr[solnLocalIndex] = ic;
									}
								} // end i
							} // end j
#if CH_SPACEDIM==3
						} // end for k
#endif
					}

					const Box& currBox = vectGrids[ilev][dit()];
					(*irrSet)[dit()] = currEBISBox.getIrregIVS(currBox);
				} // end for DataIterator

				// initialize extrapValues
				BaseIVFactory<Real>  bivfabFactory(vectEbis[iphase][ivol][ilev], *irrSet);
				if (numDefinedVolVars > 0) {
					extrapValues[iphase][ivol][ilev] = RefCountedPtr<LevelData< BaseIVFAB<Real> > >(new LevelData< BaseIVFAB<Real> >(vectGrids[ilev], numDefinedVolVars, IntVect::Zero, bivfabFactory));
				}

				// initialize membrane variable, only do it when phase=0
				if (iphase == phase0 && numDefinedMemVars > 0)
				{
					pout() << "iphase:" << iphase << ", ivol:" << ivol << ", initializing membrane solution level data" << endl;
					memSoln[ivol][ilev] = RefCountedPtr<LevelData< BaseIVFAB<Real> > >(new LevelData< BaseIVFAB<Real> >(vectGrids[ilev], numDefinedMemVars, IntVect::Zero, bivfabFactory));
					memSolnOld[ivol][ilev] = RefCountedPtr<LevelData< BaseIVFAB<Real> > >(new LevelData< BaseIVFAB<Real> >(vectGrids[ilev], numDefinedMemVars, IntVect::Zero, bivfabFactory));
				}
				else if (numDefinedVolVars == 0)
				{
					continue;
				}
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit) {
					const Box& currBox = vectGrids[ilev][dit()];
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);

					for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit) {
						const VolIndex& vof = vofit();
						RealVect vol_center = EBArith::getVofLocation(vof, vectDxes[ilev], chomboGeometry->getDomainOrigin());
						const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);

						// fill vectValues with membrane centroid
						RealVect mem_coord = mem_centroid;
						mem_coord *= vectDxes[ilev];
						mem_coord += vol_center;
						
						memset(vectValues, 0, numSymbols * sizeof(double));
						vectValues[0] = simulation->getTime_sec();
						vectValues[1] = mem_coord[0];
						vectValues[2] = mem_coord[1];
						vectValues[3] = SpaceDim == 2 ? 0.5 : mem_coord[2];

						if (numDefinedVolVars > 0)
						{
							for (int iDefinedVar = 0; iDefinedVar < numDefinedVolVars; iDefinedVar ++)
							{
								Variable* volVar = feature->getDefinedVariable(iDefinedVar);
								VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)volVar->getVarContext();
								double ic = varContextExp->evaluateExpression(INITIAL_VALUE_EXP, vectValues);
								(*extrapValues[iphase][ivol][ilev])[dit()](vof, iDefinedVar) = ic;
							}
						}

						if (iphase == phase1 || numDefinedMemVars == 0)
						{
							continue;
						}

						int membraneID = (*irregularPointMembraneIDs[iphase][ivol][ilev])[dit()](vof, 0);
						if (membraneID < 0) {
							continue;
						}
						int	jvol = membraneID % numConnectedComponents;
						Feature* jFeature = phaseVolumeList[phase1][jvol]->feature;
						Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(feature, jFeature);
						for (int ivar = 0; ivar < numDefinedMemVars; ++ ivar) {
							int varIndex = feature->getMemVarIndexesInAdjacentMembranes()[ivar];
							Variable *memVar = (Variable*)simulation->getMemVariable(varIndex);
							if (membrane->isVariableDefined(memVar)) {
								MembraneVarContextExpression* varContextExp = (MembraneVarContextExpression*)memVar->getVarContext();
								double ic = varContextExp->evaluateExpression(INITIAL_VALUE_EXP, vectValues);	
								(*memSolnOld[ivol][ilev])[dit()](vof, ivar) = ic;
								(*memSoln[ivol][ilev])[dit()](vof, ivar) = ic;
							}
						}
					}
				} // end for DataIterator
			} // end for ilev
		} // end for ivol
	} // end for iphase
	pout() << "Exit " << methodName << endl;
}

//void ChomboSemiImplicitScheduler::createVariableCoeffOpFactory(RefCountedPtr<EBConductivityOpFactory>& a_factory, int a_ivol, int a_ivar)
//{
//  // Set up the no flux domain and embedded boundary conditions
//  RefCountedPtr<NeumannConductivityDomainBCFactory> domBC(new NeumannConductivityDomainBCFactory());
//  domBC->setValue(0.0);
//
//  //  RefCountedPtr<DirichletConductivityEBBCFactory>      ebBC(new DirichletConductivityEBBCFactory());
//  //  ebBC->setValue(0.0);
//  //  ebBC->setOrder(1);
//  RefCountedPtr<NeumannConductivityEBBCFactory>      ebBC(new NeumannConductivityEBBCFactory());
//  ebBC->setValue(0.);
//
//  Vector<EBLevelGrid>  eblg;
//  Vector<RefCountedPtr<EBQuadCFInterp> > quadCFI;
//  getEBLGAndQuadCFI(eblg, quadCFI, a_ivol);
//  Vector<RefCountedPtr<LevelData<EBCellFAB> > >           aco;
//  Vector<RefCountedPtr<LevelData<EBFluxFAB> > >           bco;
//  Vector<RefCountedPtr<LevelData<BaseIVFAB<Real> > > >    bcoIrreg;
//
//  defineVariableCoeffs(aco, bco, bcoIrreg, a_ivol, a_ivar);
//
//  //coefficients come in through the =coefficients.
//  Real unity = 1.0;
//  //  int relaxType = 0;
//  //  pout() << "using multicolored gauss seidel" << endl;
//  int relaxType= m_params.m_mgRelaxType;
//  if(relaxType == 1)
//    {
//      pout() << "using multi-colored gauss seidel relaxation" << endl;
//    }
//  else if(relaxType == 0)
//    {
//      pout() << "using point jacobi relaxation" << endl;
//    }
//  else if(relaxType == 2)
//    {
//      pout() << "using gsrb fast relaxation" << endl;
//    }
//  else
//    {
//      MayDay::Error("bogus relaxType for variable coefficients");
//    }
//
//  a_factory = RefCountedPtr<EBConductivityOpFactory>
//    (new EBConductivityOpFactory(eblg, quadCFI, unity, unity, aco, bco, bcoIrreg,
//                                 m_params.m_dx,  m_params.m_refRatio, domBC, ebBC,
//                                 m_params.m_numGhostSoln, m_params.m_numGhostSource, relaxType));
//  a_factory->setData(m_scalBou[a_ivol]);
//
//}

void ChomboSemiImplicitScheduler::getEBLGAndQuadCFI(Vector<EBLevelGrid>  & ebLevelGrids,
                  	  Vector<RefCountedPtr<EBQuadCFInterp> >& quadCFInterp,
                  	  int iphase, int ivol, int ncomp)
{
	ebLevelGrids.resize(numLevels);
	quadCFInterp.resize(numLevels);

  // Define the data holders and interpolators
	for (int ilev = 0; ilev < numLevels; ++ ilev) {
		ebLevelGrids[ilev].define(vectGrids[ilev], vectEbis[iphase][ivol][ilev], vectDomains[ilev]);

		if (ilev > 0) {
			int numVariables = ncomp;
			quadCFInterp[ilev] = RefCountedPtr<EBQuadCFInterp>(new EBQuadCFInterp(vectGrids[ilev], vectGrids[ilev-1],
					vectEbis[iphase][ivol][ilev], vectEbis[iphase][ivol][ilev-1],
			        vectDomains[ilev-1], vectRefRatios[ilev-1],
			        numVariables, *(ebLevelGrids[ilev].getCFIVS()),
					&(*phaseVolumeList[iphase][ivol]->volume)));
		}
	}
}

void ChomboSemiImplicitScheduler::createConstantCoeffOpFactory(RefCountedPtr<EBAMRPoissonOpFactory>& a_factory, int iphase, int ivol, Feature* feature, int ivar)
{
	Variable* var = feature->getDefinedVariable(ivar);
  // Set up the no flux domain and embedded boundary conditions
//    RefCountedPtr<NeumannPoissonDomainBCFactory> domainBCPtr(new NeumannPoissonDomainBCFactory());
//    RefCountedPtr<DirichletPoissonDomainBCFactory> domainBCPtr(new DirichletPoissonDomainBCFactory());
//    Real bcValue = getExpressionConstantValue(var, BOUNDARY_XM_EXP, chomboGeometry->getFeature(ifeature));
//    domainBCPtr->setValue(bcValue);
//    domainBCPtr->setValue(0);

	RefCountedPtr<ChomboDomainBCFactory> domainBCPtr(new ChomboDomainBCFactory(this, iphase, ivol, feature, var));
	
	RefCountedPtr<BaseEBBCFactory> ebBCPtr;
//	RefCountedPtr<DirichletPoissonEBBCFactory> ebBCPtr(new DirichletPoissonEBBCFactory());
//  ebBCPtr->setValue(1.0);
	if (SimTool::getInstance()->getModel()->getNumMembranes() == 0)
	{
		RefCountedPtr<NeumannPoissonEBBCFactory> neumannEBBC(new NeumannPoissonEBBCFactory());
		neumannEBBC->setValue(0.0);
		ebBCPtr = neumannEBBC;
	}
	else
	{
		ebBCPtr = RefCountedPtr<BaseEBBCFactory>(new ChomboEBBCFactory(this, iphase, ivol, feature, ivar));
	}

	Vector<EBLevelGrid>  eblg;
	Vector<RefCountedPtr<EBQuadCFInterp> > quadCFI;
	getEBLGAndQuadCFI(eblg, quadCFI, iphase, ivol);

	Real currTime = simulation->getTime_sec();
	Real alpha = 1.0;
	Real beta = getExpressionConstantValue(var, DIFF_RATE_EXP, phaseVolumeList[iphase][ivol]->feature);
	if (var->isElliptic())
	{
		// set alpha to 0 when elliptic
		alpha = 0;
		// change the sign of beta when elliptic
		beta = -beta;
	}
	a_factory = RefCountedPtr<EBAMRPoissonOpFactory>(new EBAMRPoissonOpFactory(eblg, vectRefRatios, quadCFI,
					  vectDxes[0], chomboGeometry->getDomainOrigin(),
					  numPreCondIters, relaxType,
					  domainBCPtr, ebBCPtr,
					  alpha, beta, currTime, numGhostSoln, numGhostSource, numLevels));
}

void ChomboSemiImplicitScheduler::defineSolver()
{
	const char* methodName = "(ChomboSemiImplicitScheduler::defineSolver())";
	pout() << "Entry " << methodName << endl;
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;

			int numDefinedVars = feature->getNumDefinedVariables();

			for(int ivar = 0; ivar < numDefinedVars; ivar++) {

				Variable* var = feature->getDefinedVariable(ivar);
				if (!var->isDiffusing())
				{
					continue;
				}
				// This is the multigrid solver used for backward Euler
				ebMlgSolver[iphase][ivol][ivar] = RefCountedPtr<AMRMultiGrid<LevelData<EBCellFAB> > > (new AMRMultiGrid<LevelData<EBCellFAB> >() );

				// Set the verbosity of the bottom solver for multigrid
				BiCGStabSolver<LevelData<EBCellFAB> >* bottomSolver = new BiCGStabSolver<LevelData<EBCellFAB> >();
				bottomSolver->m_verbosity = 0;

				RefCountedPtr<AMRLevelOpFactory<LevelData<EBCellFAB> > > operatorFactory;
//				if (!m_params.m_constCoeff) {
//					RefCountedPtr<EBConductivityOpFactory> opfact;
//					getVariableCoeffOpFactory(opfact, a_ivol, a_ivar);
//					operatorFactory = opfact;
//				} else {
					RefCountedPtr<EBAMRPoissonOpFactory> opfact;
					createConstantCoeffOpFactory(opfact, iphase, ivol, feature, ivar);
					operatorFactory = opfact;
//				}

				// Define the multigrid solver and set various parameters
				ebMlgSolver[iphase][ivol][ivar]->define(vectDomains[0], *operatorFactory, bottomSolver, numLevels);

				ebMlgSolver[iphase][ivol][ivar]->setSolverParameters(numSmooth, numSmooth, numSmooth,
				                      numMGCycles, maxIter, chomboSpec->getRelativeTolerance(), hang, normThresh);

				ebMlgSolver[iphase][ivol][ivar]->m_verbosity = 3;
				ebMlgSolver[iphase][ivol][ivar]->init(volSolnOldWorkspace[iphase][ivol], volSourceWorkspace[iphase][ivol], numLevels - 1, 0);

				if (!var->isElliptic() && var->isDiffusing())
				{
					// Create the backward Euler solver based on the multigrid solver
					ebBEIntegratorList[iphase][ivol][ivar] = RefCountedPtr<EBBackwardEuler> (
							new EBBackwardEuler(ebMlgSolver[iphase][ivol][ivar], *operatorFactory,
									vectDomains[0], vectRefRatios,
									numLevels));
				}
			}
		}
	}
	pout() << "Exit " << methodName << endl;
}

void ChomboSemiImplicitScheduler::getExtrapStencils(Vector<RefCountedPtr<BaseIndex  > >& a_destVoFs,
                  Vector<RefCountedPtr<BaseStencil> >& a_stencils,
                  const IntVectSet & a_cfivs,
                  const DataIndex& a_dit,
                  int iphase,
                  int ivol, int ilev, Real a_dx)
{
	const EBISBox& ebisBox = vectEbis[iphase][ivol][ilev][a_dit];
	const     Box&    grid = vectGrids[ilev][a_dit];
	IntVectSet ivs = ebisBox.getIrregIVS(grid);
	VoFIterator vofit(ivs, ebisBox.getEBGraph());
	const Vector<VolIndex>& vofs = vofit.getVector();
	a_destVoFs.resize(vofs.size());
	a_stencils.resize(vofs.size());
	IntVectSet& cfivs = (IntVectSet&)a_cfivs;
	for(int ivof = 0; ivof < vofs.size(); ivof++)
	{
		VoFStencil  extrapStenc;
		const VolIndex& volIndex = vofs[ivof];
		const IntVect& gridIndex = volIndex.gridIndex();
		//distance of extrapolation = dx*boundaryCentroid
		RealVect dist = ebisBox.bndryCentroid(volIndex);
		dist *= a_dx;

//		EBArith::getFirstOrderExtrapolationStencil(extrapStenc, dist,
//		                a_dx*RealVect::Unit,
//		                vofs[ivof], ebisBox, -1, &cfivs, 0);
		Vector<VoFStencil> pointStenc;
		Vector<Real> distance;
		bool dropOrder = false;
		EBArith::johanStencil(dropOrder, pointStenc, distance, vofs[ivof], ebisBox,  a_dx*RealVect::Unit, cfivs);
		if (!dropOrder)
		{
			CH_assert(distance.size() >= 2);
			CH_assert(pointStenc.size() >= 2);
			//value at boundary is extrapolated along a ray.
			//we have data at points (phi) and distance to those points(x)
			//phi_boundary = (x1 phi0 - x0 phi1)/(x1-x0)
			pointStenc[0] *=  distance[1];
			pointStenc[1] *= -distance[0];
			extrapStenc += pointStenc[0];  //now val = x1 phi0
			extrapStenc += pointStenc[1];  //now val = x1 phi0 - x0 phi1
			extrapStenc *= 1.0/(distance[1]-distance[0]);  //now val = (x1 phi0 - x0 phi1)/(x1-x0)
		}
		else
		{
			EBArith::getExtrapolationStencil(extrapStenc, dist, a_dx*RealVect::Unit, vofs[ivof], ebisBox, -1, &cfivs, 0);
		}

		a_destVoFs[ivof] = RefCountedPtr<BaseIndex  >(new   VolIndex(volIndex));
		a_stencils[ivof] = RefCountedPtr<BaseStencil>(new VoFStencil(extrapStenc));
	}
}

void ChomboSemiImplicitScheduler::initStencils()
{
	const char* methodName = "(ChomboSemiImplicitScheduler::initStencils())";
	pout() << "Entry " << methodName << endl;
	extrapStencils.resize(NUM_PHASES);
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		int numVols = phaseVolumeList[iphase].size();
		extrapStencils[iphase].resize(numVols);
		for (int ivol = 0; ivol < numVols; ivol ++) {
			extrapStencils[iphase][ivol].resize(numLevels);

			Feature* feature = phaseVolumeList[iphase][ivol]->feature;

			int numDefinedVariables = feature->getNumDefinedVariables();
			if (numDefinedVariables == 0) {
				continue;
			}

			Vector<EBLevelGrid>  eblg;
			Vector<RefCountedPtr<EBQuadCFInterp> > quadCFI;
			int inco = numDefinedVariables;
			getEBLGAndQuadCFI(eblg, quadCFI, iphase, ivol, inco);

			Real dxlev = vectDxes[0][0];
			for(int ilev = 0; ilev < numLevels; ilev++) {
				extrapStencils[iphase][ivol][ilev] = new LayoutData< RefCountedPtr< AggStencil< EBCellFAB, BaseIVFAB<Real> > > >(vectGrids[ilev]);
				DisjointBoxLayout& currGrids = vectGrids[ilev];
				for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)	{
					Vector< RefCountedPtr<BaseIndex>   > vofs;
					Vector< RefCountedPtr<BaseStencil> > stencils;

					getExtrapStencils(vofs, stencils, (*eblg[ilev].getCFIVS())[dit()], dit(), iphase, ivol, ilev,  dxlev);
					const EBCellFAB& srcData = (*volSolnOld[iphase][ivol][ilev])[dit()];
					const BaseIVFAB<Real>& dstData = (*extrapValues[iphase][ivol][ilev])[dit()];

					(*extrapStencils[iphase][ivol][ilev])[dit()] = RefCountedPtr< AggStencil < EBCellFAB, BaseIVFAB<Real> > >
					(new AggStencil < EBCellFAB, BaseIVFAB<Real> > (vofs, stencils, srcData, dstData));

				}
				if (ilev < numLevels - 1) {
					dxlev /= vectRefRatios[ilev];
				}
			}
		}
	}
	pout() << "Exit " << methodName << endl;
}

void ChomboSemiImplicitScheduler::extrapolateDataToBoundary() {
  const char* methodName = "(ChomboSemiImplicitScheduler::extrapolateDataToBoundary())";
	pout() << "Entry " << methodName << endl;
  for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		int numVols = phaseVolumeList[iphase].size();
		for (int ivol = 0; ivol < numVols; ivol ++) {
				Feature* feature = phaseVolumeList[iphase][ivol]->feature;

				int numDefinedVars = feature->getNumDefinedVariables();
				if (numDefinedVars == 0) {
					continue;
				}
				int isrc = 0;
				int idst = 0;
				int inco = numDefinedVars;
				Vector<EBLevelGrid> eblg;
				Vector<RefCountedPtr<EBQuadCFInterp> > quadCFI;
				getEBLGAndQuadCFI(eblg, quadCFI, iphase, ivol, inco);

				for(int ilev = 0; ilev < numLevels; ilev++) {
					DisjointBoxLayout& currGrids = vectGrids[ilev];

					if (ilev > 0) {
						Interval interv(0, inco-1);
						quadCFI[ilev]->interpolate((*volSolnOld[iphase][ivol][ilev]), (*volSolnOld[iphase][ivol][ilev-1]), interv);
					}
					(*volSolnOld[iphase][ivol][ilev]).exchange();
					for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)	{
					//the last arguments say to loop over  all the variables with the stencil
					//and the false is not just increment the solution but replace the value there.
					(*extrapStencils[iphase][ivol][ilev])[dit()]->apply((*extrapValues[iphase][ivol][ilev])[dit()],
															 (*volSolnOld[iphase][ivol][ilev])[dit()],
														 isrc, idst, inco, false);
				}
			}
		}
	}
	pout() << "Exit " << methodName << endl;
}

void ChomboSemiImplicitScheduler::updateSource() {
	const char* methodName = "(ChomboSemiImplicitScheduler::updateSource)";
	pout() << "Entry " << methodName << endl;

	int volSymbolOffset = 4;
	int numVolVars = simulation->getNumVolVariables();
	int numSymbolsPerVolVar = 1 + chomboGeometry->getNumSubdomains();
	int memSymbolOffset = volSymbolOffset + numSymbolsPerVolVar * numVolVars;
	double deltaT = simulation->getDT_sec();
	vectValues[0] = simulation->getTime_sec();
	
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
			Feature* iFeature = phaseVolumeList[iphase][ivol]->feature;

			int numDefinedVolVars = iFeature->getNumDefinedVariables();
			int numDefinedMemVars = iphase == phase0 ? iFeature->getMemVarIndexesInAdjacentMembranes().size() : 0;
			if (numDefinedVolVars == 0 && numDefinedMemVars == 0) {
				continue;
			}
			
			for (int i = 1; i < numSymbols; ++ i)
			{
				vectValues[i] = BASEFAB_REAL_SETVAL;
			}

			for(int ilev = 0; ilev < numLevels; ilev ++) {
				DisjointBoxLayout& currGrids = vectGrids[ilev];

				//this is used for scaling boundary area
				Real maxDxComponent = vectDxes[ilev][0];
				for(int idir = 1; idir < SpaceDim; idir ++) {
					maxDxComponent = max(maxDxComponent, vectDxes[ilev][idir]);
				}
				Real bndryAreaScale = pow(maxDxComponent, SpaceDim - 1) / vectDxes[ilev].product();

				for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)	{
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					const Box& currBox = vectGrids[ilev][dit()];
					
					if (numDefinedVolVars > 0) {
						EBCellFAB& solnOldEBCellFAB = (*volSolnOld[iphase][ivol][ilev])[dit()];
						FArrayBox& solnOldFab = solnOldEBCellFAB.getFArrayBox();
						IntVect solnOldSize = solnOldFab.size();
						const int *solnOldLo = solnOldFab.loVect();
						Real* solnOldDataPtr = solnOldFab .dataPtr();
						// used to solve Volume ODE
						Real* solnDataPtr = (*volSoln[iphase][ivol][ilev])[dit()].getFArrayBox().dataPtr();

						EBCellFAB& sourceEBCellFAB = (*volSource[iphase][ivol][ilev])[dit()];
						sourceEBCellFAB.setVal(0.);

						FArrayBox& sourceFab = sourceEBCellFAB.getFArrayBox();
						IntVect sourceSize = sourceFab.size();
						Real* sourceDataPtr = sourceFab.dataPtr();

#if CH_SPACEDIM==3
						for (int k = numGhostSoln[2]; k < solnOldSize[2] - numGhostSoln[2]; k ++) { // phi has ghost point
#endif
							for (int j = numGhostSoln[1]; j < solnOldSize[1] - numGhostSoln[1]; j ++) { // phi has ghost point
								for (int i = numGhostSoln[0]; i < solnOldSize[0] - numGhostSoln[0]; i ++) {

									IntVect gridIndex(D_DECL(i + solnOldLo[0], j + solnOldLo[1], k + solnOldLo[2]));
									if (currEBISBox.isCovered(gridIndex)) {
										continue;
									}
									RealVect coord = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
									
									vectValues[1] = coord[0];
									vectValues[2] = coord[1];
									vectValues[3] = SpaceDim < 3 ? 0.5 : coord[2];

									// fill the values
									int iDefinedVar = 0;
									for (int ivar = 0; ivar < numVolVars; ivar ++) {
										Variable* var = (Variable*)simulation->getVolVariable(ivar);
										if (!iFeature->isVariableDefined(var)) {
											continue;
										}
										int solnLocalIndex = getChomboBoxLocalIndex(solnOldSize, iDefinedVar, D_DECL(i, j, k));
										vectValues[volSymbolOffset + ivar * numSymbolsPerVolVar] = solnOldDataPtr[solnLocalIndex];
										iDefinedVar ++;
									}

									for (int ivar = 0; ivar < numDefinedVolVars; ivar ++) {
										Variable* var = iFeature->getDefinedVariable(ivar);
										VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)var->getVarContext();
										double eval = varContextExp->evaluateExpression(REACT_RATE_EXP, vectValues);
										if (var->isDiffusing())
										{
											int sourceIndex = getChomboBoxLocalIndex(sourceSize, ivar,
												D_DECL(i - numGhostSoln[0] + numGhostSource[0], j - numGhostSoln[1] + numGhostSource[1], k - numGhostSoln[2] + numGhostSource[2]));
											sourceDataPtr[sourceIndex] = eval;
										}
										else
										{
											int solnLocalIndex = getChomboBoxLocalIndex(solnOldSize, ivar, D_DECL(i, j, k));
											solnDataPtr[solnLocalIndex] = solnOldDataPtr[solnLocalIndex] + deltaT * eval;
										}
									} // end for ivar

								} // end for i
							} // end for j
#if CH_SPACEDIM==3					
						} // end for k
#endif
					} // if (numDefinedVolVars > 0)

					// update source with membrane flux or dirichlet values
					// solve membrane variables
					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
					Vector<ConnectedComponent*>& adjacentVolumes = phaseVolumeList[iphase][ivol]->adjacentVolumes;

					for (int j = 0; j < adjacentVolumes.size(); j ++) {
						int jphase = adjacentVolumes[j]->phase;
						int jvol = adjacentVolumes[j]->volumeIndexInPhase;

						int currentMembraneID = 0;
						if (iphase == phase0)
						{
							currentMembraneID = ivol * numConnectedComponents + jvol;
						} 
						else if (iphase == phase1)
						{
							currentMembraneID = jvol * numConnectedComponents + ivol;
						}

						Feature* jFeature = adjacentVolumes[j]->feature;
						Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(iFeature, jFeature);
						for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit)
						{
							const VolIndex& vof = vofit();
//							double volfrac = currEBISBox.volFrac(vof);
//							if (volfrac <= ChomboLevelRedist::TINY_VOL_FRAC)
//							{
//								pout() << "====phase " << iphase << ", @ " << vof << ", volfrac=" << volfrac
//												<< " <= " << ChomboLevelRedist::TINY_VOL_FRAC << ", skip membrane flux.====" << endl;
//							}
//							else
//							{
								int globalMemIndex = (*irregularPointMembraneIndex[iphase][ivol][ilev])[dit()](vof, 0);
								if (globalMemIndex == MEMBRANE_INDEX_IN_FINER_LEVEL)
								{
									continue;
								}

								int membraneID = (*irregularPointMembraneIDs[iphase][ivol][ilev])[dit()](vof, 0);
								if (membraneID != currentMembraneID)
								{
									continue;
								}
								const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);
								Real mem_areaFrac = currEBISBox.bndryArea(vof);
								RealVect vol_center = EBArith::getVofLocation(vof, vectDxes[ilev], chomboGeometry->getDomainOrigin());

								// fill vectValues
								RealVect mem_point = mem_centroid;
								mem_point *= vectDxes[ilev];
								mem_point += vol_center;

								memset(vectValues, 0, numSymbols * sizeof(double));
								vectValues[0] = simulation->getTime_sec();
								vectValues[1] = mem_point[0];
								vectValues[2] = mem_point[1];
								vectValues[3] = SpaceDim == 2 ? 0.5 : mem_point[2];

								{
									int iDefinedVar = 0, jDefinedVar = 0;
									// Fill volume variables to vectValues in iFeature and jFeature.
									for (int ivar = 0; ivar < numVolVars; ++ ivar)
									{
										Variable* var = (Variable*)simulation->getVolVariable(ivar);
										if (iFeature->isVariableDefined(var))
										{
											Real extrapVal = (*extrapValues[iphase][ivol][ilev])[dit()](vof, iDefinedVar);
											vectValues[volSymbolOffset + ivar * numSymbolsPerVolVar + 1 + iFeature->getIndex()] = extrapVal;
											++ iDefinedVar;
										}
										if (jFeature->isVariableDefined(var))
										{
											Real extrapVal = (*extrapValues[jphase][jvol][ilev])[dit()](vof, jDefinedVar);
											vectValues[volSymbolOffset + ivar * numSymbolsPerVolVar + 1 + jFeature->getIndex()] = extrapVal;
											++ jDefinedVar;
										}
									}
								}

								{
									// Fill membrane variables to vectValues in iFeature and jFeature.
									int ivolInPhase0 = iphase == phase0 ? ivol : jvol;
									Feature* iFeatureInPhase0 = iphase == phase0 ? iFeature : jFeature;
									for (int ivar = 0; ivar < iFeatureInPhase0->getMemVarIndexesInAdjacentMembranes().size(); ivar ++)
									{
										int varIndex =	iFeatureInPhase0->getMemVarIndexesInAdjacentMembranes()[ivar];
										Variable* var = (Variable*)simulation->getMemVariable(varIndex);
										if (membrane->isVariableDefined(var))
										{
											Real mv = (*memSolnOld[ivolInPhase0][ilev])[dit()](vof, ivar);
											vectValues[memSymbolOffset + varIndex] = mv;
										}
									}
								}

								if (numDefinedVolVars > 0)
								{
									EBCellFAB& sourceEBCellFAB = (*volSource[iphase][ivol][ilev])[dit()];
									for (int ivar = 0; ivar < numDefinedVolVars; ivar ++)
									{
										Variable* var = iFeature->getDefinedVariable(ivar);
										if (!var->isDiffusing())
										{
											continue;
										}

										// kappa weight reaction, so that we don't have divide volFrac in membrane flux
										Real volFrac = currEBISBox.volFrac(vof);
										sourceEBCellFAB(vof, ivar, vof.cellIndex()) *= volFrac;

										VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)var->getVarContext();
										double Jval = varContextExp->evaluateJumpCondition(membrane, vectValues);
										if (iFeature->getEbBcType(membrane) == BOUNDARY_VALUE)
										{
											(*extrapValues[iphase][ivol][ilev])[dit()](vof, ivar) = Jval;
										}
										else
										{
											double flux = Jval * mem_areaFrac * bndryAreaScale;
											sourceEBCellFAB(vof, ivar, vof.cellIndex()) += flux;
										}
									}
								}
//							} // end if (volfrac)

							// membrane variables, simply iterate
							if (iphase == phase0 && numDefinedMemVars > 0)
							{
								for (int ivar = 0; ivar < iFeature->getMemVarIndexesInAdjacentMembranes().size(); ivar ++)
								{
									int varIndex =	iFeature->getMemVarIndexesInAdjacentMembranes()[ivar];
									Variable* var = (Variable*)simulation->getMemVariable(varIndex);
									if (membrane->isVariableDefined(var))
									{
										MembraneVarContextExpression* varContextExp = (MembraneVarContextExpression*)var->getVarContext();
										double eval = varContextExp->evaluateExpression(REACT_RATE_EXP, vectValues);
										Real oldSol = (*memSolnOld[ivol][ilev])[dit()](vof, ivar);
										(*memSoln[ivol][ilev])[dit()](vof, ivar) = oldSol + deltaT * eval;
									}
								}
							}
						} // for (VoFIterator vofit(irregCells,currEBGraph);
					} // end for jvol
				} // end DataIter
			} // end ilev
		} // end ivol
	} // end iphase
	pout() << "Exit " << methodName << endl;
}

void ChomboSemiImplicitScheduler::printOpMatrix()
{
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++)
		{
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;

			int numDefinedVars = feature->getNumDefinedVariables();

			for(int ivar = 0; ivar < numDefinedVars; ++ ivar)
			{
				Variable* var = feature->getDefinedVariable(ivar);
				for(int ilev = 0; ilev < numLevels; ++ ilev)
				{
					printOpMatrix(iphase, ivol, ilev, ivar, var->getName());
				}
			}
		}
	}
}

void ChomboSemiImplicitScheduler::printOpMatrix(int iphase, int ivol, int ilev, int ivar, const string& varName)
{
	pout() << "variable=\"" << varName << "\";" << endl;
	pout() << "[iphase ivol ilev ivar]=[" << iphase << " " << ivol << " " << ilev << " " << ivar << " " << "]" << ";" << endl;
	const LevelData<EBCellFAB>& a_rhs = (*volSource[iphase][ivol][ilev]);
	const LevelData<EBCellFAB>& a_phi = (*volSoln[iphase][ivol][ilev]);
	EBAMRPoissonOp* op = dynamic_cast<EBAMRPoissonOp* >(ebMlgSolver[iphase][ivol][ivar]->getAMROperators()[ilev]);

  LevelData<EBCellFAB> column, phi01, rhs0;
  LevelData<BaseFab<int> > gids;

  IntVect idghostsPhi = a_phi.ghostVect();
  IntVect idghostsRhs = a_rhs.ghostVect();
  const DisjointBoxLayout &dbl = a_phi.disjointBoxLayout();
  const EBLevelGrid eblg = op->getEBLG();
  const EBCellFactory ebcellfact(eblg.getEBISL() );
  column.define(dbl, 1, idghostsRhs, ebcellfact);
  rhs0.define(dbl, 1, idghostsRhs, ebcellfact);
  phi01.define(dbl, 1, idghostsPhi, ebcellfact);
  gids.define(dbl, 1, idghostsRhs);

  EBLevelDataOps::setVal(rhs0,  0);
  EBLevelDataOps::setVal(phi01, 0);
  EBLevelDataOps::setVal(column,0);

  int data=0;
  for (DataIterator dit = a_rhs.dataIterator() ; dit.ok() ; ++dit )
	{
		const Box &box = dbl.get(dit());
		if (CH_SPACEDIM==3) data += box.size(0)*box.size(1)*box.size(2);
		else data += box.size(0)*box.size(1);
	}
  int gid0;
  int ierr;
  gid0 = 0;
  ierr = 0;
  int gid = gid0;

  //
  // all the indexes are incremented for MATLAB
  //
  int precision = 17;
  pout() << "volfrac = zeros(" << data << ");" << endl;
  pout() << "iv = zeros(" << data << "," << data << ");" << endl;
  for (DataIterator  dit = a_rhs.dataIterator() ; dit.ok() ; ++dit )
	{
		BaseFab<int> &gidsFab = gids[dit()];
		const Box& box = dbl.get(dit());
		BoxIterator bit(box);
		const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
		for (bit.begin(); bit.ok(); bit.next(), gid++ )
		{
			IntVect iv = bit();
			double volfrac = currEBISBox.volFrac(VolIndex(iv, 0));
			gidsFab(iv,0) = gid;
			pout() << std::setprecision(precision) << "iv(" << (gid+1) << ")=" << (iv + IntVect::Unit) << "; volfrac(" << (gid+1) << ")=" << volfrac << ";" << endl;
		}
	}

  IntVect lastIV = IntVect::Zero;
  pout() << "A = zeros(" << data << "," << data << ");" << endl;
  for (DataIterator dit = a_rhs.dataIterator(); dit.ok(); ++dit)
  {
		BaseFab<Real>& phi01FAB =  phi01[dit()].getSingleValuedFAB();
		BaseFab<Real>& columnFAB = column[dit()].getSingleValuedFAB();
		BaseFab<int> & gidsFab = gids[dit()];

		const Box& box = dbl.get(dit());
		BoxIterator bit(box);
		for (bit.begin(); bit.ok(); bit.next())
		{
			IntVect iv = bit();
			phi01FAB(lastIV,0) = 0;
			phi01FAB(iv,0) = 1;
			lastIV = iv;
			EBLevelDataOps::setVal(column,0);
			op->residual(column,phi01,rhs0); // results could be on all procs

			Real v;
			int  i,j;
			i = gidsFab(iv,0);
			for (DataIterator dit2 = a_rhs.dataIterator(); dit2.ok(); ++dit2)
			{
				const Box& box2 = dbl.get(dit2());
				const EBISBox& dit2EBISBox = vectEbis[iphase][ivol][ilev][dit2()];
				bool hasIrreg = false;
				for (BoxIterator bit2(box2); bit2.ok(); bit2.next())
				{
					IntVect iv2 = bit2();
					v = columnFAB(iv2,0);
					if (v != 0)
					{
						double volfrac = dit2EBISBox.volFrac(VolIndex(iv2, 0));
						if (volfrac < 1)
						{
							hasIrreg=true;
							break;
						}
					}
				}
				if (!hasIrreg)
				{
					continue;
				}

				for (BoxIterator bit2(box2); bit2.ok(); bit2.next())
				{
					IntVect iv2 = bit2();
					v = columnFAB(iv2,0);
					j = gidsFab(iv2,0);
					if (v != 0)
					{
						pout() << std::setprecision(precision) << "A(" << (j+1) << "," << (i+1) << ")=" << v << ";" << endl;
					}
				}
			}
		}
  }
}

void ChomboSemiImplicitScheduler::solveFastSystem()
{
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++)
		{
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;
			FastSystemExpression* fs = feature->getFastSystem();
			if (!fs)
			{
				continue;
			}
			fs->solve(this, iphase, ivol);
		}
	}
}
