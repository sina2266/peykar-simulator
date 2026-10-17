#include <VCELL/ChomboScheduler.h>
#include <BRMeshRefine.H>
#include <CH_HDF5.H>
#include <EBArith.H>
#include <AMRIO.H>

#include <EBISLayout.H>
#include <EBLevelGrid.H>
#include <GeometryShop.H>
#include <EBEllipticLoadBalance.H>
#include <EBCoarseAverage.H>
#include <EBAMRDataOps.H>
#include <EBLevelDataOps.H>
#include <MFIndexSpace.H>
#include <ComplementIF.H>
#include <UnionIF.H>
#include <PolyGeom.H>

#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <VCELL/ChomboIF.h>
#include <VCELL/ChomboGeometry.h>
#include <VCELL/ChomboGeometryShop.h>
#include <VCELL/ConnectedComponent.h>
#include <VCELL/ChomboSpec.h>
#include <VCELL/DataSet.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/SimTool.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/VarContext.h>
#include <VCELL/VCellModel.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <assert.h>
#include <sstream>
#include <hdf5.h>
#include <iomanip>

#ifdef CH_MPI
#include <mpi.h>
#endif

#ifdef HOFFSET
	 #undef HOFFSET
#endif
#define HOFFSET(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define HDF5_FILE_EXT ".hdf5"
#define MESH_HDF5_FILE_EXT ".mesh.hdf5"
#define IF_VAR_NAME_PREFIX "zzz_IF_"
// Holds a path built from BASE_FILE_NAME -- see FILE_NAME_BUFSIZE in SimTool.cpp.
#define FILE_NAME_BUFSIZE 4096
#define CHOMBO_OUTPUT_FILE_NAME_FORMAT "%s%06d_%s_vol%d"HDF5_FILE_EXT  // SimID_98312620_0_000000_outside_vol0.hdf5

static const int nestingRadius  = 2; //ghostPhi[0];  // should be the same as ghost phi size, but Terry used 2
static const int maxCoarsen = -1;
static const int numPreCondIters = 4;
static const int relaxType = 2;
static const int numGhostEBISLayout = 4;
static const double smallVolFrac = 1e-3;

const int ChomboScheduler::phase0 = 0;
const int ChomboScheduler::phase1 = 1;

enum MembraneInvalidNeighborIndex
{
	MEMBRANE_NEIGHBOR_UNKNOWN = -10,
	MEMBRANE_NEIGHBOR_NEXT_TO_WALL = -1,
};

ChomboScheduler::ChomboScheduler(SimulationExpression* sim, ChomboSpec* chomboSpec)
{
	simulation = sim;
	this->chomboSpec = chomboSpec;
	chomboGeometry = this->chomboSpec->getChomboGeometry();
	numLevels = chomboSpec->getNumLevels();
	int* ratios = chomboSpec->getRefRatios();
	for (int ilev = 0; ilev < numLevels; ++ ilev)
	{
		vectRefRatios.push_back(ratios[ilev]);
	}
	hdf5FileCount = 0;
	numGhostSoln = IntVect::Unit * 3;
	
#ifdef CH_MPI
	membraneIndexGhost = IntVect::Unit;
#else
	membraneIndexGhost = IntVect::Zero;
#endif

	memIndexOffset = 0;
	H5dont_atexit();
}

ChomboScheduler::~ChomboScheduler() {
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		delete geoIfs[iphase];
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++)
		{
			volSoln[iphase][ivol].clear();
			irregularPointMembraneIDs[iphase][ivol].clear();
			irregularPointMembraneIndex[iphase][ivol].clear();
			delete phaseVolumeList[iphase][ivol];

			for (int ilev = 0; ilev < numLevels; ilev ++)
			{
				int ibox = -1;
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
				{
					++ ibox;
//					for (int i = 0; i < irregTinyVolNeighbors[iphase][ivol][ilev][ibox].size(); ++ i)
//					{
//						delete irregTinyVolNeighbors[iphase][ivol][ilev][ibox][i];
//					}
					irregTinyVolNeighbors[iphase][ivol][ilev][ibox].clear();
				}
				irregTinyVolNeighbors[iphase][ivol][ilev].clear();
			}
			irregTinyVolNeighbors[iphase][ivol].clear();
		}
		phaseVolumeList[iphase].clear();
		irregularPointMembraneIDs[iphase].clear();
		irregularPointMembraneIndex[iphase].clear();

		irregTinyVolNeighbors[iphase].clear();
	}
	phaseVolumeList.clear();
	irregularPointMembraneIDs.clear();
	irregularPointMembraneIndex.clear();
	irregVolumeMembraneMap.clear();
}

bool ChomboScheduler::isInNextFinerLevel(int level, const IntVect& gridIndex) {
	if (level >= numLevels - 1) {
		return false;
	}

	int refRatio = vectRefRatios[level];
	IntVect gridIndexInNextFinerLevel = gridIndex * refRatio;

	int nextFinerLevel = level + 1;
	for (LayoutIterator lit = vectGrids[nextFinerLevel].layoutIterator(); lit.ok(); ++ lit) {
		const Box& box = vectGrids[nextFinerLevel].get(lit());
		if (box.contains(gridIndexInNextFinerLevel)) {
			return true;
		}
	}
	return false;
}

int ChomboScheduler::getVolumeIndex(const IntVect& size, const IntVect& ijk)
{
	int index = ijk[1] * size[0] + ijk[0];
#if CH_SPACEDIM == 3
	index += ijk[2] * size[0] * size[1];
#endif
	return index;
}

int ChomboScheduler::getChomboBoxLocalIndex(const IntVect& size, int ivar, const IntVect& ijk) {
	return getChomboBoxLocalIndex(size, ivar, D_DECL(ijk[0], ijk[1], ijk[2]));
}

int ChomboScheduler::getChomboBoxLocalIndex(const IntVect& size, int ivar, D_DECL(int i, int j, int k)) {
#if CH_SPACEDIM==2
	return ivar * size[0] * size[1] + j * size[0] + i;
#else
	return ivar * size[0] * size[1] * size[2] + k * size[0] * size[1] + j * size[0] + i;
#endif
}

#ifdef CH_MPI
void ChomboScheduler::exchangeFeaturesAndMembraneIndexOffset()
{
	const char* methodName = "(ChomboScheduler::exchangeFeaturesAndMembraneIndexOffset)";
	pout() << "Entry " << methodName << endl;

	int numProcs = SimTool::getInstance()->getCommSize();
	pout() << "numProcs=" << numProcs << endl;
	
	// sendBuffer: feature indexes of all volumes (numConnectedComponents), numMembranePoints on each proc, totalNumMembranePoints in all
	// so the total size is (numConnectedComponents + numProcs + 1)
	// but the first send, only send my own numMembranePoints, so it will only use (numConnectedComponents + 1)
	// later when the root broadcast, it would use all the capacity
	int* sendBuffer = new int[numConnectedComponents + numProcs + 1];
	int singleRecvSize = numConnectedComponents + 1;
	int receiveSize = numProcs * singleRecvSize;
	int *recvBuffer = NULL;
	if (SimTool::getInstance()->isRootRank())
	{
		 recvBuffer = new int[receiveSize];
	}

	// collect feature indexes to send
	for (int iphase = 0, volCnt = 0; iphase < NUM_PHASES; ++ iphase)
	{
		// Select one index-space
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol, ++ volCnt)
		{
			ConnectedComponent* cc = phaseVolumeList[iphase][ivol];
			sendBuffer[volCnt] = cc->feature == NULL ? -1 : cc->feature->getIndex();
		}
	}
	sendBuffer[numConnectedComponents] = numMembranePoints;

	// each processor puts its sendBuffer in recvBuffer in the order or rank.
	// so in recvBuffer, the data looks like
	// ---sendBuffer of processor 1---sendBuffer of processor 2---
	pout() << "gathering features and # membrane points from each processor" << endl;
	MPI_Gather(sendBuffer, singleRecvSize, MPI_INT, recvBuffer, singleRecvSize, MPI_INT, SimTool::rootRank,MPI_COMM_WORLD);

	if (SimTool::getInstance()->isRootRank())
	{
		pout() << "root rank collecting features" << endl;
		for (int volCnt = 0; volCnt < numConnectedComponents; ++ volCnt)
		{
			if (sendBuffer[volCnt] != -1)
			{
				continue;
			}
			for (int iproc = 0; iproc < numProcs; ++ iproc)
			{
				if (iproc == SimTool::getInstance()->getMyRank()) // skip my own data
				{
					continue;
				}
				int idx = iproc * singleRecvSize + volCnt;
				if (recvBuffer[idx] != -1)
				{
					sendBuffer[volCnt] = recvBuffer[idx];
					break;
				}
			} // end iproc
		} // end volCnt
		
		pout() << "root rank collect membraneIndexOffset" << endl;
		totalNumMembranePoints = 0;
		for (int iproc = 0; iproc < numProcs; ++ iproc)
		{
			int n = recvBuffer[iproc * singleRecvSize + numConnectedComponents];
			sendBuffer[numConnectedComponents + iproc] = totalNumMembranePoints;
			totalNumMembranePoints += n;
		}
		sendBuffer[numConnectedComponents + numProcs] = totalNumMembranePoints;
		pout() << "totalNumMembranePoints=" << totalNumMembranePoints << endl;
	}

	pout() << "broadcasting features and membraneIndexOffset " << endl;
	// broadcast
	MPI_Bcast(sendBuffer, numConnectedComponents + numProcs + 1, MPI_INT, SimTool::rootRank, MPI_COMM_WORLD);
	
	pout() << "populating features" << endl;
	for (int iphase = 0, volCnt = 0; iphase < NUM_PHASES; ++ iphase)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol, ++ volCnt)
		{
			ConnectedComponent* cc = phaseVolumeList[iphase][ivol];
			if (cc->feature != NULL)
			{
				continue;
			}

			if (sendBuffer[volCnt] == -1)
			{
				stringstream ss;
				ss << "iphase " << iphase << ", ivol " << ivol << ", feature not found.";
				throw ss.str();
			}
			else
			{
				cc->feature = SimTool::getInstance()->getModel()->getFeatureFromIndex(sendBuffer[volCnt]);
				cc->feature->setPhase(iphase);
				pout() << "iphase " << iphase << ", ivol " << ivol << ", assign feature " << cc->feature->getName() << endl;
			}
		} // end ivol
	}	// end iphase

	pout() << "** find memIndexOffset" << endl;
	memIndexOffset = sendBuffer[numConnectedComponents + SimTool::getInstance()->getMyRank()];
	totalNumMembranePoints = sendBuffer[numConnectedComponents + numProcs];
	pout() << "** my memIndexOffset is = " << memIndexOffset << ", totalNumMembranePoints=" << totalNumMembranePoints << endl;

	delete[] sendBuffer;
	delete[] recvBuffer;

	pout() << "** reset membrane index level data" << endl;

	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol)
		{
			for (int ilev = 0; ilev < numLevels; ilev ++)
			{
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit)
				{
					const Box& currBox = vectGrids[ilev][dit()];

					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
					for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++ vofit)
					{
						const VolIndex& vof = vofit();
						int localMemIndex = (*irregularPointMembraneIndex[iphase][ivol][ilev])[dit()](vof, 0);
						if (localMemIndex == MEMBRANE_INDEX_IN_FINER_LEVEL)
						{
							continue;
						}
						int globalMemIndex = localMemIndex + memIndexOffset;
						(*irregularPointMembraneIndex[iphase][ivol][ilev])[dit()](vof, 0) = globalMemIndex;
					} // end for VoFIterator
				} // end for DataIterator

				// exchange to get ghost points filled
				irregularPointMembraneIndex[iphase][ivol][ilev]->exchange();
			} // end for ilev
		} // end for ivol
	} // end for iphase
	
	pout() << "Exit " << methodName << endl;
}
#endif

void ChomboScheduler::computeFeatures()
{
	const char* methodName = "(ChomboScheduler::computeFeatures)";
	pout() << "Entry " << methodName << endl;
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		int numVols = phaseVolumeList[iphase].size();
		for (int ivol = 0; ivol < numVols; ++ ivol)
		{
			ConnectedComponent* cc = phaseVolumeList[iphase][ivol];
			bool bfoundMyFeature = false;
			for (int ilev = 0; ilev < numLevels; ++ ilev)
			{
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
				{
					const Box& currBox = vectGrids[ilev][dit()];
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					const IntVect& smallEnd = currBox.smallEnd();
					const IntVect& boxSize = currBox.size();
					pout() << "Searching for feature (regular points), iphase: " << iphase << ", ivol: " << ivol
										<< " , box smallEnd: " << smallEnd << " ; size: " << boxSize << endl;
#if CH_SPACEDIM==3
					for (int k = 0; k < boxSize[2]; ++ k)
					{
#endif
						for (int j = 0; j < boxSize[1]; ++ j)
						{
							for (int i = 0; i < boxSize[0]; ++ i)
							{
								IntVect gridIndex(D_DECL(i + smallEnd[0], j + smallEnd[1], k + smallEnd[2]));
								// check every regular point to make sure
								// our IF (R) functions are correct.
								if (!isInNextFinerLevel(ilev, gridIndex) && currEBISBox.isRegular(gridIndex))
								{
									RealVect a_point = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
									cc->feature = chomboGeometry->getFeature(a_point, true);
									cc->feature->setPhase(iphase);
									if (!bfoundMyFeature)
									{
										// only print once
										pout() << "Found feature (regular) for iphase " << iphase << ", ivol " << ivol
											<< ", Feature (name, index)=(" << cc->feature->getName() << "," << cc->feature->getIndex() << ")" << endl;
									}
									bfoundMyFeature = true;
								}
							} // end for i
						} // end for j
#if CH_SPACEDIM==3
					} // end for k
#endif
				} // end DataIterator
			} // end ilev


			// New way to find feature for each connected component (in parallel, this is computed only if it will be needed)
			// search irregular points if needed
			if (!bfoundMyFeature)
			{
				// if no regular point is found, use irregular points (centroids) to find feature,
				// but this may not be as reliable
				// so we will check the feature in all irregular points just in case
				for (int ilev = 0; ilev < numLevels; ilev ++)
				{
					for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
					{
						const Box& currBox = vectGrids[ilev][dit()];
						const IntVect& smallEnd = currBox.smallEnd();
						const IntVect& boxSize = currBox.size();
						pout() << "Searching for feature (irregular points) , iphase: " << iphase << ", ivol: " << ivol
											<< " , box smallEnd: " << smallEnd << " ; size: " << boxSize << endl;

						const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
						IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
						const EBGraph& currEBGraph = currEBISBox.getEBGraph();

						for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++ vofit)
						{
							const VolIndex& vof = vofit();
							const IntVect& gridIndex = vof.gridIndex();
							// find volume fraction for this vof, if too small then ignore
							double volfrac = currEBISBox.volFrac(vof);
							if (!isInNextFinerLevel(ilev, gridIndex) && (volfrac > smallVolFrac))
							{
								RealVect a_point = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
								//RealVect a_point = EBArith::getIVLocation(gridIndex, fineDx, chomboGeometry->getDomainOrigin());
								RealVect centroid = currEBISBox.centroid(vof);
								centroid *= vectDxes[ilev];
								a_point += centroid;
								Feature* fi = chomboGeometry->getFeature(a_point, false);
								if (cc->feature == NULL)
								{
									cc->feature = fi;
									pout() << "Found feature (irregular) for iphase " << iphase << ", ivol " << ivol
									<< ", Feature (name, index)=(" << cc->feature->getName() << "," << cc->feature->getIndex() << ")" << endl;
									cc->feature->setPhase(iphase);
									bfoundMyFeature = true;
								}
								else if (cc->feature != fi)
								{
									stringstream ss;
									ss << "Found inconsistent features for two irregular vof. Try refining mesh around " << a_point;
									throw ss.str();
								}
							}
						}
					} // end dit
				} // end ilev
			} // end if (!bfoundMyFeature)

#ifndef CH_MPI
			if (!bfoundMyFeature)
			{
				stringstream ss;
				ss <<	"did not find feature for iphase " << iphase << ", ivol " << ivol;
				throw ss.str();
			}
#endif
		} // end for ivol
	}// end for phase

	pout() << "Exit " << methodName << endl;
}

double ChomboScheduler::getExpressionConstantValue(Variable* var, ExpressionIndex expIndex, Feature* feature) {
	VolumeVarContextExpression* varContextExp =	(VolumeVarContextExpression*)var->getVarContext();
	return varContextExp->evaluateConstantExpression(expIndex);
}

void ChomboScheduler::generateMembraneIndexData()
{	
	const char* methodName = "(ChomboScheduler::generateMembraneIndexData)";
	pout() << "Entry " << methodName << endl;

	// compute membrane id
	// allocate storage and initialize to -1
	pout() << "Initializing membrane ID level data" << endl;
	irregularPointMembraneIDs.resize(NUM_PHASES);
	irregularPointMembraneIndex.resize(NUM_PHASES);
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++)
	{
		int numVols = phaseVolumeList[iphase].size();
		irregularPointMembraneIDs[iphase].resize(numVols);
		irregularPointMembraneIndex[iphase].resize(numVols);
		for (int ivol = 0; ivol < numVols; ++ ivol)
		{
			irregularPointMembraneIDs[iphase][ivol].resize(numLevels);
			irregularPointMembraneIndex[iphase][ivol].resize(numLevels);
			for (int ilev = 0; ilev < numLevels; ilev ++)
			{
				RefCountedPtr< LayoutData<IntVectSet> > irrSet = RefCountedPtr<LayoutData<IntVectSet> >(new LayoutData<IntVectSet>(vectGrids[ilev]));
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
				{
					const Box& currBox = vectGrids[ilev][dit()];
					Box boxWithGhost = currBox;
					boxWithGhost.grow(membraneIndexGhost);

					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					(*irrSet)[dit()] = currEBISBox.getIrregIVS(boxWithGhost);
				}
				BaseIVFactory<int>  bivfabFactory(vectEbis[iphase][ivol][ilev], *irrSet);
				irregularPointMembraneIDs[iphase][ivol][ilev] = RefCountedPtr<LevelData< BaseIVFAB<int> > >(new LevelData< BaseIVFAB<int> >(vectGrids[ilev], 1, IntVect::Zero, bivfabFactory));
				// add a new structure to store indexes for the membrane elements
				irregularPointMembraneIndex[iphase][ivol][ilev] = RefCountedPtr<LevelData< BaseIVFAB<int> > >(new LevelData< BaseIVFAB<int> >(vectGrids[ilev], 1, membraneIndexGhost, bivfabFactory));
				for (DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
				{
					(*irregularPointMembraneIDs[iphase][ivol][ilev])[dit()].setVal(-1);
					(*irregularPointMembraneIndex[iphase][ivol][ilev])[dit()].setVal(MEMBRANE_INDEX_INVALID);
				}
			} // end ilev
		} // end ivol
	} // end iphase

	// find membrane for each irregular point
	pout() << "Computing membrane ID , adjacent volumes, membrane local indexes"	<< endl;
	bool* bAdjacent = new bool[phaseVolumeList[phase1].size()];
	numMembranePoints = 0;
	for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ++ ivol)
	{
		memset(bAdjacent, 0, phaseVolumeList[phase1].size() * sizeof(bool));

		for (int ilev = 0; ilev < numLevels; ilev ++)
		{
			for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit)
			{
				const Box& currBox = vectGrids[ilev][dit()];

				const EBISBox& currEBISBox = vectEbis[phase0][ivol][ilev][dit()];
				const EBGraph& currEBGraph = currEBISBox.getEBGraph();
				IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
				for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++ vofit)
				{
					const VolIndex& vof = vofit();
					const IntVect& gridIndex = vof.gridIndex();

					int membraneID = -1;
					int jvol;
					for (jvol = 0; jvol < phaseVolumeList[phase1].size(); ++ jvol)
					{
						const IntVectSet& ivs = vectEbis[phase1][jvol][ilev][dit()].getIrregIVS(currBox);
						if (ivs.contains(gridIndex))
						{
							membraneID = ivol * numConnectedComponents + jvol;
							(*irregularPointMembraneIDs[phase0][ivol][ilev])[dit()](vof, 0) = membraneID;
							(*irregularPointMembraneIDs[phase1][jvol][ilev])[dit()](vof, 0) = membraneID;
							bAdjacent[jvol] = true;
							break;
						}
					}

					bool bInFinerLevel = isInNextFinerLevel(ilev, gridIndex);
					if (membraneID == -1)
					{
						// membrane not found
						if (bInFinerLevel)
						{
							// it is OK if the point is refined
							(*irregularPointMembraneIDs[phase0][ivol][ilev])[dit()](vof, 0) = -1;
							(*irregularPointMembraneIndex[phase0][ivol][ilev])[dit()](vof, 0) = MEMBRANE_INDEX_IN_FINER_LEVEL;
						}
						else
						{
							// throw exception if it's not refined
							RealVect a_point = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
							RealVect centroid = currEBISBox.centroid(vof);
							centroid *= vectDxes[ilev];
							a_point += centroid;
							double areaFrac = currEBISBox.bndryArea(vof);
							double volFrac = currEBISBox.volFrac(vof);
							stringstream ss;
							ss << "At point " << a_point << ", membrane cannot be resolved. Use finer mesh or refine near that point."
							   << "{phase:0, vol:" << ivol << ", lev:" << ilev
								<< ", box:" << currBox << ", vof:" << vof << ", coordinate:" << a_point
								<< ", volumeFraction:" << volFrac << ", areaFraction:" << areaFrac
								<< ", error:no matching point in the other phase}.";
							throw ss.str();
						}
					}
					else
					{
						int localMemIndex = -1;
						if (bInFinerLevel)
						{
							localMemIndex = MEMBRANE_INDEX_IN_FINER_LEVEL;
						}
						else
						{
							localMemIndex = (*irregularPointMembraneIndex[phase0][ivol][ilev])[dit()](vof, 0);
							if (localMemIndex >= 0)
							{
								RealVect a_point = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
								RealVect centroid = currEBISBox.centroid(vof);
								centroid *= vectDxes[ilev];
								a_point += centroid;
								double areaFrac = currEBISBox.bndryArea(vof);
								double volFrac = currEBISBox.volFrac(vof);
								stringstream ss;
								ss << "At point " << a_point << ", multi-valued point found. Use finer mesh or refine near that point."
							   << "{phase:0, vol:" << ivol << ", lev:" << ilev
								<< ", box:" << currBox << ", vof:" << vof << ", coordinate:" << a_point
								<< ", volumeFraction:" << volFrac << ", areaFraction:" << areaFrac
								<< ", error:mulit-valued point}.";
								throw ss.str();
							}
							 // in parallel, these indexes are not unique from processor to processor
							 // but it is probably not a problem as long as it is corrected by offset when needed
							localMemIndex = numMembranePoints ++;
						}
						(*irregularPointMembraneIndex[phase0][ivol][ilev])[dit()](vof, 0) = localMemIndex;
						(*irregularPointMembraneIndex[phase1][jvol][ilev])[dit()](vof, 0) = localMemIndex;

						if (localMemIndex == MEMBRANE_INDEX_IN_FINER_LEVEL)
						{
							const EBISBox& currEBISBox_phase1 = vectEbis[phase1][jvol][ilev][dit()];
							const EBGraph& currEBGraph_phase1 = currEBISBox_phase1.getEBGraph();
							// if this point is multi-valued in phase 1, we need to assign other cells.
							if (currEBISBox_phase1.numVoFs(gridIndex) > 1)
							{
								IntVectSet irregCells_phase1;
								irregCells_phase1 |= gridIndex;  // add the same and only point
								for (VoFIterator vofit_phase1(irregCells_phase1, currEBGraph_phase1); vofit_phase1.ok(); ++ vofit_phase1)
								{
									const VolIndex& vof_phase1 = vofit_phase1();
									pout() << "setting value of same point multi-valued in phase 1 to MEMBRANE_INDEX_IN_FINER_LEVEL "
									<< "{phase:1, vol:" << ivol << ", lev:" << ilev << ", vof:" << vof_phase1 << ", box:" << currBox << "}" << endl;
									(*irregularPointMembraneIndex[phase1][jvol][ilev])[dit()](vof_phase1, 0) = MEMBRANE_INDEX_IN_FINER_LEVEL;
								}
							}
						}
					} // end else
				} // end for VoFIterator
			} // end for DataIterator
		} // end for ilev

		for (int jvol = 0; jvol < phaseVolumeList[phase1].size(); ++ jvol)
		{
			if (bAdjacent[jvol])
			{
				phaseVolumeList[phase0][ivol]->adjacentVolumes.push_back(phaseVolumeList[phase1][jvol]);
				phaseVolumeList[phase1][jvol]->adjacentVolumes.push_back(phaseVolumeList[phase0][ivol]);
			}
		}
	} // end for ivol
	delete[] bAdjacent;
	pout() << "numMembranePoints=" << numMembranePoints << endl;

  // validate the membrane indexes in phase 1
	for (int ivol = 0; ivol < phaseVolumeList[phase1].size(); ++ ivol)
	{
		for (int ilev = 0; ilev < numLevels; ilev ++)
		{
			for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit)
			{
				const Box& currBox = vectGrids[ilev][dit()];

				const EBISBox& currEBISBox = vectEbis[phase1][ivol][ilev][dit()];
				const EBGraph& currEBGraph = currEBISBox.getEBGraph();
				IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
				for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++ vofit)
				{
					const VolIndex& vof = vofit();
					const IntVect& gridIndex = vof.gridIndex();
					if ((*irregularPointMembraneIndex[phase1][ivol][ilev])[dit()](vof, 0) == MEMBRANE_INDEX_INVALID)
					{
						if (isInNextFinerLevel(ilev, gridIndex))
						{
							(*irregularPointMembraneIndex[phase1][ivol][ilev])[dit()](vof, 0) = MEMBRANE_INDEX_IN_FINER_LEVEL;
						}
						else
						{
							RealVect a_point = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
							RealVect centroid = currEBISBox.centroid(vof);
							centroid *= vectDxes[ilev];
							a_point += centroid;
							double areaFrac = currEBISBox.bndryArea(vof);
							double volFrac = currEBISBox.volFrac(vof);
							stringstream ss;
							ss << "At point " << a_point << ", membrane cannot be resolved. Use finer mesh or refine near that point."
							   << "{phase:1, vol:" << ivol << ", lev:" << ilev
								<< ", box:" << currBox << ", vof:" << vof << ", coordinate:" << a_point
								<< ", volumeFraction:" << volFrac << ", areaFraction:" << areaFrac
								<< ", error:no matching point in the other phase}.";
							throw ss.str();
						}
					}
				}
			}
		}
	}
	pout() << "Exit " << methodName << endl;
}

void ChomboScheduler::generateTinyVolumeNeighbors()
{
	if (chomboSpec->getSmallVolfracThreshold() <= 0)
	{
		return;
	}

	static const char* methodName = "(ChomboScheduler::generateTinyVolumeNeighbors)";
	pout() << "Entry " << methodName << endl;

	irregTinyVolNeighbors.resize(NUM_PHASES);
	bHasTinyVols = false;
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		int numVols = phaseVolumeList[iphase].size();
		irregTinyVolNeighbors[iphase].resize(numVols);
		for (int ivol = 0; ivol < numVols; ++ ivol)
		{
			irregTinyVolNeighbors[iphase][ivol].resize(numLevels);
			for (int ilev = 0; ilev < numLevels; ilev ++)
			{
				// populate the irregular point (volIndex) -> membrane index map , based on membrane index level data
				int numBoxes = vectGrids[ilev].numBoxes(SimTool::getInstance()->getMyRank());
				pout() << "(iphase, ivol, ilev)=(" << iphase << "," << ivol << "," << ilev << "), numBoxes=" << numBoxes << endl;
				irregTinyVolNeighbors[iphase][ivol][ilev].resize(numBoxes);

				int ibox = -1;
				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
				{
					++ ibox;
					pout() << "(iphase, ivol, ilev)=(" << iphase << "," << ivol << "," << ilev << "), ibox=" << ibox << endl;
					const Box& currBox = vectGrids[ilev][dit()];
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
					for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit)
					{
						const VolIndex& vof = vofit();
						const IntVect& gridIndex = vof.gridIndex();

						double volFrac = currEBISBox.volFrac(vof);
						if (volFrac < chomboSpec->getSmallVolfracThreshold() && !isInNextFinerLevel(ilev, gridIndex))
						{
							bHasTinyVols = true;
							irregTinyVolNeighbors[iphase][ivol][ilev][ibox].push_back(vof);
							pout() << "(iphase, ivol, ilev, ibox)=(" << iphase << "," << ivol << "," << ilev << "," << ibox << "), tiny vol "
									<< vof << ", volFrac=" << volFrac << endl;
						}
					} // end for vof
				} // end for dit
			} // end for ilev
		} // end for ivol
	} // end for iphase

	pout() << "Exit " << methodName << ", hasTinyVols=" << bHasTinyVols << endl;
}

void ChomboScheduler::generateVolumeMembraneIndexMap()
{
	static const char* methodName = "(ChomboScheduler::generateVolumeMembraneIndexMap)";
	pout() << "Entry " << methodName << endl;

	irregVolumeMembraneMap.resize(numLevels);
	for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ++ ivol)
	{
		for (int ilev = 0; ilev < numLevels; ilev ++)
		{
			// populate the irregular point (volIndex) -> membrane index map , based on membrane index level data
			for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
			{
				const Box& currBox = vectGrids[ilev][dit()];
				Box boxWithGhost = currBox;
				boxWithGhost.grow(membraneIndexGhost);
				
				const EBISBox& currEBISBox = vectEbis[phase0][ivol][ilev][dit()];
				const EBGraph& currEBGraph = currEBISBox.getEBGraph();
				IntVectSet irregCells = currEBISBox.getIrregIVS(boxWithGhost); // irregular points in box with ghost
				for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit)
				{
					const VolIndex& vof = vofit();
					const IntVect& gridIndex = vof.gridIndex();
					// compute map entry
					int volIndex = getVolumeIndex(vectNxes[ilev], gridIndex);
					int globalMemIndex = (*irregularPointMembraneIndex[phase0][ivol][ilev])[dit()](vof, 0);
					map<int,int>::iterator iter = irregVolumeMembraneMap[ilev].find(volIndex);
					if (iter == irregVolumeMembraneMap[ilev].end())
					{
						irregVolumeMembraneMap[ilev][volIndex] = globalMemIndex;
					}
					else
					{
						int existingMemIndex = irregVolumeMembraneMap[ilev][volIndex];
						if (existingMemIndex == globalMemIndex)
						{
//							pout() << "Vol->Mem mapping @ " << gridIndex << " already set to  " << existingMemIndex
//											<< " and now another mapping set to the same again." << endl;
						}
						else
						{
							stringstream ss;
							ss << "Vol->Mem mapping @ " << gridIndex << " already set to  " << existingMemIndex
											<< " and now another mapping set to " << globalMemIndex << " again." << endl;
							throw ss.str();
						}
					}
				} // end for vof
			} // end for dit
		} // end for ilev
	} // end for ivol
	
	pout() << "Exit " << methodName << endl;
}

bool ChomboScheduler::tagROIs(Vector<IntVectSet>& tags)
{
	static const char* METHOD = "(ChomboScheduler::tagROIs)";
	pout() << "Entry " << METHOD << endl;

	bool bCellsTagged = false;
	// Membrane ROIs
	pout() << "Starting tagging membrane ROIs..." << endl;
	vector<ChomboRefinementRoi*>& membraneRefinementRois = chomboSpec->getMembraneRefinementRois();
	for (vector<ChomboRefinementRoi*>::iterator it = membraneRefinementRois.begin(); it != membraneRefinementRois.end(); ++ it)
	{
		ChomboRefinementRoi* roi = *it;
		int tagLevel = roi->getLevel() - 1;   // if ROI is at Level N, we tag points at N-1 level
		pout() << "Membrane ROI; " << roi->getRoi()->infix();
		IntVectSet memTags;
		// user might give 1.0 to refine all irregular points
		const double* c = roi->getConstantValue();
		if (c != NULL && *c != 0)
		{
			pout() << ", tagging all irregular points at level " << tagLevel << endl;
			for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
				for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
					// Get the depth of the tag level
					int depth = phaseVolumeList[iphase][ivol]->volume->getLevel(vectDomains[tagLevel]);

					memTags |= phaseVolumeList[iphase][ivol]->volume->irregCells(depth);
				}
			}
			memTags.grow(chomboSpec->getTagsGrow());
		}
		else
		{
			pout() << ", tagging irregular points at level " << tagLevel << endl;
			// not a constant, have to evaluate point by point
			for (int iphase = 0; iphase < NUM_PHASES; iphase ++)
			{
				for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++)
				{
					int depth = phaseVolumeList[iphase][ivol]->volume->getLevel(vectDomains[tagLevel]);
					IntVectSet tagsVol = phaseVolumeList[iphase][ivol]->volume->irregCells(depth);
					IVSIterator ivsit (tagsVol);
					for (ivsit.begin(); ivsit.ok(); ++ivsit)
					{
						IntVect gridIndex = ivsit();
						RealVect vol_center = EBArith::getIVLocation(gridIndex, vectDxes[tagLevel], chomboGeometry->getDomainOrigin());
						if (roi->getRoi()->evaluateVector(vol_center.dataPtr()))
						{
//							pout() << "tagging point " << gridIndex << " at level " << tagLevel << endl;
							memTags |= gridIndex;
						}
					}
				} // ivol
			} // iphase
		}
		tags[tagLevel] |= memTags;
		bCellsTagged |= !memTags.isEmpty();
	}

	// volume ROIs
	pout() << "Starting tagging volume ROIs..." << endl;
	vector<ChomboRefinementRoi*>& volumeRefinementRois = chomboSpec->getVolumeRefinementRois();
	for (vector<ChomboRefinementRoi*>::iterator it = volumeRefinementRois.begin(); it != volumeRefinementRois.end(); ++ it)
	{
		ChomboRefinementRoi* roi = *it;
		int tagLevel = roi->getLevel() - 1;
		pout() << "Volume ROI: " << roi->getRoi()->infix() << ", tagging points at level " << tagLevel << endl;

		IntVectSet volTags;
		// not a constant, have to evaluate point by point
		// right now we have to go through each point because
		// we don't have the grids for higher levels yet.
#if CH_SPACEDIM==3
		for (int k = 0; k < vectNxes[tagLevel][2]; k ++) {
#endif
			for (int j = 0; j < vectNxes[tagLevel][1]; j ++) {
				for (int i = 0; i < vectNxes[tagLevel][0]; i ++) {
					IntVect gridIndex(D_DECL(i, j, k));
					RealVect coord = EBArith::getIVLocation(gridIndex, vectDxes[tagLevel], chomboGeometry->getDomainOrigin());
					if (roi->getRoi()->evaluateVector(coord.dataPtr()))
					{
						//pout() << "tagging point " << gridIndex << " at level " << tagLevel << endl;
						volTags |= gridIndex;
					}
				}
			}
#if CH_SPACEDIM==3
		}
#endif
		tags[tagLevel] |= volTags;
		bCellsTagged |= !volTags.isEmpty();
	}
	
	pout() << "Exit " << METHOD << endl;
	return bCellsTagged;
}

void ChomboScheduler::generateMesh()
{
	static const char* thisMethod = "(ChomboScheduler::generateMesh)";
	pout() << "Entry " << thisMethod << endl;
	
	ProblemDomain coarsestDomain = vectDomains[0];

	pout() << "Fill EBISLayout for coarsest level" << endl;
	Vector<int> coarsestProcs;
	Vector<Box> coarsestBoxes;

	domainSplit(coarsestDomain, coarsestBoxes, chomboSpec->getMaxBoxSize(), chomboSpec->getBlockFactor());
	mortonOrdering(coarsestBoxes);
	LoadBalance(coarsestProcs, coarsestBoxes);

	vectGrids.resize(numLevels);
	vectGrids[0] = DisjointBoxLayout(coarsestBoxes, coarsestProcs, coarsestDomain);

	vectEbis.resize(NUM_PHASES);
	for(int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		vectEbis[iphase].resize(phaseVolumeList[iphase].size());
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
			vectEbis[iphase][ivol].resize(numLevels);
			phaseVolumeList[iphase][ivol]->volume->fillEBISLayout(vectEbis[iphase][ivol][0],
										vectGrids[0],
										coarsestDomain,
										numGhostEBISLayout);
		}
	}

	if (numLevels > 1) {
		pout() << "Starting mesh refinement (BRMeshRefine)" << endl;
		// If there is more than one level, the finer levels need to created by "BRMeshRefine"
		BRMeshRefine meshRefine(coarsestDomain, vectRefRatios, chomboSpec->getFillRatio(), chomboSpec->getBlockFactor(),
						nestingRadius, chomboSpec->getMaxBoxSize());

		// Tags for creating the finer levels
		Vector<IntVectSet> tags(numLevels);
		bool bCellsTagged = tagROIs(tags);
		for (int i = 0; i < tags.size() - 1; ++ i) // finest level shouldn't have any points
		{
			pout() << " Number of tagged points at level " << i << " is " << tags[i].numPts() << endl;
		}

#ifndef CH_MPI
		if (!bCellsTagged)
		{
			throw "No cells tagged for refinement";
		}
#endif

		pout() << "Re-griding and fill EBISLayout for each level" << endl;

		Vector<Vector<Box> > oldBoxes(numLevels);
		Vector<Vector<Box> > newBoxes;
		// Need the boxes on the coarsest level and the tags on the second to
		// finest level to make all the boxes on all the levels
		oldBoxes[0]= coarsestBoxes;
		// Make all the boxes on all the levels
		meshRefine.regrid(newBoxes, tags, 0, numLevels - 1, oldBoxes);

		ProblemDomain domainLev = coarsestDomain;
		for(int ilev = 1; ilev < numLevels; ++ ilev)
		{
			Vector<int> newProcs;
			domainLev.refine(vectRefRatios[ilev-1]);
			mortonOrdering(newBoxes[ilev]);
			LoadBalance(newProcs, newBoxes[ilev]);

			vectGrids[ilev] = DisjointBoxLayout(newBoxes[ilev], newProcs, domainLev);
			//generate ebislayout
			for(int iphase = 0; iphase < NUM_PHASES; ++ iphase) {
				for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol) {
					vectEbis[iphase][ivol].resize(numLevels);
					phaseVolumeList[iphase][ivol]->volume->fillEBISLayout(vectEbis[iphase][ivol][ilev],
												vectGrids[ilev],
												domainLev,
												numGhostEBISLayout);
				}
			}
		}
	}
	pout() << "Exit " << thisMethod << endl;
}

void ChomboScheduler::generatePhasesAndVolumes()
{
	static const char* thisMethod = "(ChomboScheduler::generatePhasesAndVolumes)";
	pout() << "Entry " << thisMethod << endl;

	pout() << "EB Mesh processing: MFIndexSpace" << endl;
	int finestLevel = numLevels - 1;
	ProblemDomain finestDomain = vectDomains[finestLevel];
	RealVect fineDx = vectDxes[finestLevel];

	Vector<GeometryService*> geometries(NUM_PHASES, NULL);
	Vector<BaseIF*> phase0List;
	for (int i = 0; i < chomboGeometry->getNumSubdomains(); ++ i) {
		ChomboIF* chomboIF = chomboGeometry->getChomboIF(i);
		if (chomboIF->getPhaseIndex() == 0) {
			phase0List.push_back(chomboIF);
		}
	}
	geoIfs[phase0] = new UnionIF(phase0List);
	GeometryShop *workshop = new GeometryShop(*(geoIfs[phase0]), 0, fineDx);
	workshop->m_phase = 0;
	geometries[0] = workshop;

	geoIfs[phase1] = new ComplementIF(*(geoIfs[phase0]));
	workshop = new GeometryShop(*(geoIfs[phase1]), 0, fineDx);
	workshop->m_phase = 1;
	geometries[1] = workshop;

	const RealVect& domainOrigin = chomboGeometry->getDomainOrigin();
	MFIndexSpace mfIndexSpace;
	mfIndexSpace.define(finestDomain.domainBox(), domainOrigin, fineDx[0], geometries, chomboSpec->getMaxBoxSize()/* ,maxCoarsenings */);

	phaseVolumeList.resize(NUM_PHASES);

	pout() << "Collecting Connected Components from MFIndexSpace" << endl;
	for (int iphase = 0; iphase < NUM_PHASES; iphase ++)
	{
		// Select one index-space
		Chombo_EBIS::alias((EBIndexSpace*) mfIndexSpace.EBIS(iphase));
		EBIndexSpace* ebisPtr = Chombo_EBIS::instance();
		Vector<RefCountedPtr<EBIndexSpace> > volumes = ebisPtr->connectedComponents();
		for (int ivol = 0; ivol < volumes.size(); ++ ivol)
		{
			RefCountedPtr<EBIndexSpace> volume = volumes[ivol];
			ConnectedComponent* cc = new ConnectedComponent;
			cc->feature = NULL;
			cc->phase = iphase;
			cc->volumeIndexInPhase = ivol;
			cc->volume = volume;
			phaseVolumeList[iphase].push_back(cc);
		} // end for ivol
	}// end for iphase
	numConnectedComponents = phaseVolumeList[phase0].size() + phaseVolumeList[phase1].size();
	pout() << "Exit " << thisMethod << ", numConnectedComponents=" << numConnectedComponents << endl;
}

void ChomboScheduler::initializeGrids()
{
	if (SpaceDim != chomboGeometry->getDimension())
	{
		stringstream ss;
		ss << "Wrong executable (" << SpaceDim << "d) for this simulation (" << chomboGeometry->getDimension() << "d)" << endl;
		throw ss.str();
	}

	const char* methodName = "(ChomboScheduler::initializeGrids)";
	pout() << "Entry " << methodName << endl;

	IntVect coarsestNx = chomboGeometry->getMeshSize();
	RealVect coarsestDx = chomboGeometry->getDomainSize();

	IntVect lo = IntVect::Zero;
	IntVect hi = coarsestNx - IntVect::Unit;
	Box crseDomBox(lo, hi);
	ProblemDomain coarsestDomain(crseDomBox);

	// compute Dx=Size/Nx at the first level
	for (int idir = 0; idir < SpaceDim; idir ++) {
		coarsestDx[idir] /= coarsestNx[idir];
		coarsestDomain.setPeriodic(idir, false);
	}

	// setup Domains and Dxes for all levels;
	vectDomains.resize(numLevels);
	vectDomains[0] = coarsestDomain;
	vectDxes.resize(numLevels);
	vectDxes[0] = coarsestDx;
	vectNxes.resize(numLevels);
	vectNxes[0] = coarsestNx;
	for(int ilev = 1; ilev < numLevels; ++ ilev) {
		int prevLev = ilev - 1;
		vectDxes[ilev] = vectDxes[prevLev] / vectRefRatios[prevLev];
		vectNxes[ilev] = vectNxes[prevLev] * vectRefRatios[prevLev];
		vectDomains[ilev] = refine(vectDomains[prevLev], vectRefRatios[prevLev]);
	}

	generatePhasesAndVolumes();
	generateMesh();
	if (chomboSpec->getSmallVolfracThreshold() > 0)
	{
		generateTinyVolumeNeighbors();
	}
	computeFeatures();
  generateMembraneIndexData();

#ifdef CH_MPI
	// has to call this after everything
	// needs numMembranePoints to find memIndexOffset
	exchangeFeaturesAndMembraneIndexOffset();
#endif

	generateVolumeMembraneIndexMap();

	pout() << "Features summary" << endl;
	for(int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol)
		{
			ConnectedComponent* cc = phaseVolumeList[iphase][ivol];
			pout() << "iphase " << iphase << ", ivol " << ivol << ", feature " << cc->feature->getName() << endl;
		}
	}

	// compute/exchange structure sizes
	computeStructureSizes();

	// print summary
	pout() << "Printing summary" << endl;
	for(int iphase = 0; iphase < NUM_PHASES; iphase ++) {
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
			for(int ilev = 0; ilev < numLevels; ilev++) {
				long totalIrregLevel   = 0.0;
				long totalPtsLevel   = 0.0;

				for (DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++dit) {
					const Box&  currBox      = vectGrids[ilev][dit()];

					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
					pout() << "phase " << iphase << ":volume " << ivol << ":level " << ilev << ":" << " Box " << currBox << endl;
					totalPtsLevel += currBox.numPts();
					totalIrregLevel += irregCells.numPts();
				}

				std::ios::fmtflags origFlags = cout.flags();
				int origWidth = cout.width();
				int origPrecision = cout.precision();

				pout() << "phase " << iphase << ":volume " << ivol << ":level " << ilev << ":" << endl;
				pout() << setiosflags(ios::right);
				pout() << "  Total computation cells: " << setw(10) << totalPtsLevel << endl;
				pout() << "  Total irregular cells:   " << setw(10) << totalIrregLevel << endl;
				pout() << endl;

				pout().flags(origFlags);
				pout().width(origWidth);
				pout().precision(origPrecision);
			}
		} // ivol
	} // iphase
	pout() << "Exit " << methodName << endl;
}

void ChomboScheduler::computeStructureSizes()
{
	const char* methodName = "(ChomboScheduler::computeStructureSizes)";
	pout() << "Entry " << methodName << endl;
	int cfRefRatio = 1;
	for(int ilev = 0; ilev < numLevels - 1; ++ ilev)
	{
		cfRefRatio *= vectRefRatios[ilev];
	}
	double finestLevelUnitV = vectDxes[numLevels - 1].product();
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol)
		{
			double sumVolFrac = 0;
			int refratio = cfRefRatio;
			int numPoints = 0;

			for(int ilev = 0; ilev < numLevels; ++ ilev)
			{
				int numRepeats = pow(refratio,SpaceDim);

				for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
				{
					const Box& currBox = vectGrids[ilev][dit()];
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];

					const IntVect& bigEnd = currBox.bigEnd();
					const IntVect& smallEnd = currBox.smallEnd();

#if CH_SPACEDIM==3
					for (int k = smallEnd[2]; k <= bigEnd[2]; ++ k)
					{
#endif
						for (int j = smallEnd[1]; j <= bigEnd[1]; ++ j)
						{
							for (int i = smallEnd[0]; i <= bigEnd[0]; ++ i)
							{
								IntVect gridIndex(D_DECL(i, j, k));
								if (currEBISBox.isCovered(gridIndex))
								{
									continue;
								}
								++ numPoints;

								if (isInNextFinerLevel(ilev, gridIndex))
								{
									continue;
								}

								double volFrac = 1.0;
								if (currEBISBox.isIrregular(gridIndex))
								{
									volFrac = currEBISBox.volFrac(VolIndex(gridIndex, 0));
								}
								volFrac *= numRepeats;
								sumVolFrac += volFrac;
							} // i
						} // j
#if CH_SPACEDIM==3
					} // k
#endif
					if (iphase == phase0)
					{
						//this is used for scaling boundary area
						Real maxDxComponent = vectDxes[ilev][0];
						for(int idir = 1; idir < SpaceDim; ++ idir)
						{
							maxDxComponent = max(maxDxComponent, vectDxes[ilev][idir]);
						}
						double levelUnitS = pow(maxDxComponent,SpaceDim -1);

						Feature* iFeature = phaseVolumeList[iphase][ivol]->feature;
						const EBGraph& currEBGraph = currEBISBox.getEBGraph();
						IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
						Vector<ConnectedComponent*>& adjacentVolumes = phaseVolumeList[iphase][ivol]->adjacentVolumes;

						for (int j = 0; j < adjacentVolumes.size(); ++ j)
						{
							int jphase = adjacentVolumes[j]->phase;
							int jvol = adjacentVolumes[j]->volumeIndexInPhase;

							int currentMembraneID = ivol * numConnectedComponents + jvol;
							Feature* jFeature = phaseVolumeList[jphase][jvol]->feature;
							if (iFeature == jFeature)
							{
								stringstream ss;
								ss << "2 adjacent volumes are in the same feature " << iFeature->getName()
												<< ", [iphase=" << iphase << ", ivol=" << ivol << "]"
												<< ",[jphase=" << jphase << ", jvol=" << jvol << "]. "
												<< "Mesh is too coarse to resolve. Refine the mesh.";
								throw ss.str();
							}
							Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(iFeature, jFeature);
							for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit)
							{
								const VolIndex& vof = vofit();
								int membraneID = (*irregularPointMembraneIDs[iphase][ivol][ilev])[dit()](vof, 0);
								if (membraneID != currentMembraneID)
								{
									continue;
								}
								int globalMemIndex = (*irregularPointMembraneIndex[iphase][ivol][ilev])[dit()](vof, 0);
								if (globalMemIndex == MEMBRANE_INDEX_IN_FINER_LEVEL)
								{
									continue;
								}
								double areaFrac = currEBISBox.bndryArea(vof);
								membrane->addSize(areaFrac * levelUnitS);
								membrane->addNumPoints(1);
							}
						}
					}
				} // dit
				refratio /= vectRefRatios[ilev];
			} // level
			double s = sumVolFrac * finestLevelUnitV;
			phaseVolumeList[iphase][ivol]->feature->addSize(s);
			phaseVolumeList[iphase][ivol]->feature->addNumPoints(numPoints);
		} // ivol
	} // iphase

#ifdef CH_MPI
	// need to collect the total sizes
	VCellModel* model = SimTool::getInstance()->getModel();
	int count = model->getNumFeatures() * 2 + model->getNumMembranes() * 2;
	double* sendBuffer = new double[count];
	double* recvBuffer = new double[count];
	int bi = 0;
	for (int i = 0; i < model->getNumFeatures(); ++ i)
	{
		Feature* f = model->getFeatureFromIndex(i);
		sendBuffer[bi ++] = f->getNumPoints();
		sendBuffer[bi ++] = f->getSize();
	}
	for (int i = 0; i < model->getNumMembranes(); ++ i)
	{
		Membrane* m = model->getMembraneFromIndex(i);
		sendBuffer[bi ++] = m->getNumPoints();
		sendBuffer[bi ++] = m->getSize();
	}
	// all reduce - sum
	MPI_Allreduce(sendBuffer, recvBuffer, count, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

	bi = 0;
	for (int i = 0; i < model->getNumFeatures(); ++ i)
	{
		Feature* f = model->getFeatureFromIndex(i);
		f->setNumPoints((int)recvBuffer[bi ++]);
		f->setSize(recvBuffer[bi ++]);
	}
	for (int i = 0; i < model->getNumMembranes(); ++ i)
	{
		Membrane* m = model->getMembraneFromIndex(i);
		m->setNumPoints((int)recvBuffer[bi ++]);
		m->setSize(recvBuffer[bi ++]);
	}
	delete[] sendBuffer;
	delete[] recvBuffer;
#endif
	pout() << "Exit " << methodName << endl;
}

void ChomboScheduler::updateSolutionFromLevelData()
{
	const char* methodName = "(ChomboScheduler::updateSolutionFromLevelData)";
	pout() << "Entry " << methodName << endl;

	for (int i = 0; i < simulation->getNumVariables(); ++ i)
	{
		Variable* var = simulation->getVariable(i);
		var->reset(chomboSpec->isSaveVCellOutput());
	}

#ifndef CH_MPI
	if (chomboSpec->isSaveVCellOutput())
	{
		// volume and IF values are written out
		// for NON MPI VCell output
		populateVolumeSolution();
		populateImplicitFunctions();
	}
#endif

	// membrane solution and extrapolated values are written out
	// for both MPI and NON MPI and for both VCell and Chombo output
	populateMembraneSolution();
	populateExtrapolatedValues();

	computeTotal();

	for (int i = 0; i < simulation->getNumVariables(); ++ i)
	{
		Variable* var = simulation->getVariable(i);
		var->computeFinalStatistics();
	}

	pout() << "Exit " << methodName << endl;
}

#ifndef CH_MPI
void ChomboScheduler::populateVolumeSolution()
{
	if (!chomboSpec->isSaveVCellOutput())
	{
		return;
	}
	static const char* methodName = "(ChomboScheduler::populateVolumeSolution)";
	pout() << "Entry " << methodName << endl;

	for (int iphase = 0; iphase < NUM_PHASES; iphase ++)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++)
		{
			populateVolumeSolution(iphase, ivol);
		} // end for ivol
	} // end for iphase

	pout() << "Exit " << methodName << endl;
}

void ChomboScheduler::populateVolumeSolution(int iphase, int ivol)
{
	if (!chomboSpec->isSaveVCellOutput())
	{
		return;
	}

	static const char* methodName = "(ChomboScheduler::populateVolumeSolution(iphase, ivol))";
	pout() << "Entry " << methodName << endl;

	Feature* feature = phaseVolumeList[iphase][ivol]->feature;
	int numDefinedVolVars = feature->getNumDefinedVariables();
	if (numDefinedVolVars == 0)
	{
		pout() << " no variables defined in feature " << feature->getName() << endl;
		return;
	}
	
	int cfRefRatio = 1;
	int viewLevel = chomboSpec->getViewLevel();
	for(int ilev = 0; ilev < viewLevel; ++ ilev)
	{
		cfRefRatio *= vectRefRatios[ilev];
	}
	double viewLevelUnitV = vectDxes[viewLevel].product();
	
	// average fine to coarse
	Interval interv(0, numDefinedVolVars - 1);
	for(int ilev = numLevels - 1; ilev > viewLevel; ilev --)
	{
		int coarseLevel = ilev - 1;
		EBCoarseAverage averageOp(vectGrids[ilev], vectGrids[coarseLevel], vectEbis[iphase][ivol][ilev], vectEbis[iphase][ivol][coarseLevel],
			vectDomains[coarseLevel], vectRefRatios[coarseLevel], numDefinedVolVars, (const EBIndexSpace *)phaseVolumeList[iphase][ivol]->volume);
		averageOp.average(*volSoln[iphase][ivol][coarseLevel], *volSoln[iphase][ivol][ilev], interv);
	}

	for (int ivar = 0; ivar < numDefinedVolVars; ivar ++)
	{
		Variable* var = feature->getDefinedVariable(ivar);
		double* varCurr = var->getCurr();
		double* errorCurr = 0;
		double* relErrCurr = 0;
		bool bComputeError = false;
		Variable* errorVar = var->getExactErrorVariable();
		if (errorVar != NULL)
		{
			bComputeError = true;
			errorCurr = errorVar->getCurr();
			relErrCurr = var->getRelativeErrorVariable()->getCurr();
		}

		int refratio = cfRefRatio;
		// copy phi to var, repeat values for coarse levels
		for(int ilev = 0; ilev <= viewLevel; ilev ++)
		{
			int numRepeats = pow(refratio,SpaceDim);
			for(DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
			{
				const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];

				EBCellFAB& solnEBCellFAB = (*volSoln[iphase][ivol][ilev])[dit()];
				FArrayBox& solnFab = solnEBCellFAB.getFArrayBox();
				const IntVect& solnSize = solnFab.size();
				const int* solnLo = solnFab.loVect();
				Real* solnDataPtr = solnFab.dataPtr();

#if CH_SPACEDIM==3
				for (int k = numGhostSoln[2]; k < solnSize[2] - numGhostSoln[2]; k ++)
				{
					int koffset = k + solnLo[2];
#endif
					for (int j = numGhostSoln[1]; j < solnSize[1] - numGhostSoln[1]; j ++)
					{
						int joffset = j + solnLo[1];
						for (int i = numGhostSoln[0]; i < solnSize[0] - numGhostSoln[0]; i ++)
						{
							int ioffset = i + solnLo[0];

							IntVect gridIndex(D_DECL(ioffset, joffset, koffset));
							if (currEBISBox.isCovered(gridIndex) || ilev < viewLevel && isInNextFinerLevel(ilev, gridIndex))
							{
								continue;
							}

							double volFrac = 1.0;
							if (currEBISBox.isIrregular(gridIndex))
							{
								volFrac = currEBISBox.volFrac(VolIndex(gridIndex, 0));
							}
							volFrac *= numRepeats;
							double sol = solnDataPtr[getChomboBoxLocalIndex(solnSize, ivar, D_DECL(i, j, k))];

							var->min = std::min<double>(var->min, sol);
							var->max = std::max<double>(var->max, sol);

							var->addTotalVCell(sol * volFrac * viewLevelUnitV);
							double error = 0;
							double relErr = 0;
							if (bComputeError)
							{
								RealVect coord = EBArith::getIVLocation(gridIndex, vectDxes[ilev], chomboGeometry->getDomainOrigin());
								memset(vectValues, 0, numSymbols * sizeof(double));
								vectValues[0] = simulation->getTime_sec();
								vectValues[1] = coord[0];
								vectValues[2] = coord[1];
								vectValues[3] = SpaceDim < 3 ? 0.5 : coord[2];
								double exact = var->getVarContext()->evaluateExpression(EXACT_EXP, vectValues);
								error = exact - sol;
								relErr = exact == 0 ? 0 : std::abs(error/exact);

								if (volFrac  > smallVolFrac)
								{
									var->updateMaxError(abs(error));
								}
								double l2 = error * error * volFrac;
								double exactl2 = exact * exact * volFrac;
								var->addL2Error(l2);
								var->addL2Exact(exactl2);
							}
#if CH_SPACEDIM==3
							for (int kk = 0; kk < refratio; kk ++)
							{
#endif
								for (int jj = 0; jj < refratio; jj ++)
								{
									for (int ii = 0; ii < refratio; ii ++)
									{
										IntVect viewLevelGridIndex(D_DECL(ioffset * refratio + ii, joffset * refratio + jj, koffset * refratio + kk));
										int volIndex = getVolumeIndex(vectNxes[viewLevel], viewLevelGridIndex);
										varCurr[volIndex] = sol;
										if (bComputeError)
										{
											errorCurr[volIndex] = error;
											relErrCurr[volIndex] = relErr;
										}
									} // end for ii
								} // end for jj
#if CH_SPACEDIM==3
							} // end for kk
#endif
						} // end for i
					} // end for j
#if CH_SPACEDIM==3
				} // end for k
#endif
			} // end for (DataIterator
			refratio /= vectRefRatios[ilev];
		} // end for ilev
	} // for (int ivar)

	pout() << "Exit " << methodName << endl;
}

void ChomboScheduler::populateImplicitFunctions()
{
	if (!chomboSpec->isSaveVCellOutput())
	{
		return;
	}

	const char* methodName = "(ChomboScheduler::populateImplicitFunctions)";
	pout() << "Entry " << methodName << endl;
	
	int viewLevel = chomboSpec->getViewLevel();
	
	// first time to save IF as volume variable
	static bool bIFVariableUpdated = false;
	if (!bIFVariableUpdated)
	{
		// create IF variables
		int numSubdomains = chomboGeometry->getNumSubdomains();
		for (int d = 0; d < numSubdomains; ++ d)
		{
			ChomboIF* chomboIf = chomboGeometry->getChomboIF(d);
			Feature* feature = chomboIf->getFeature();
			VolumeVariable* var = feature->getIFVariable();
			if (var == NULL)
			{
				string varname = string(IF_VAR_NAME_PREFIX) + feature->getName();
				var = new VolumeVariable(varname, NULL, vectNxes[viewLevel].product(), 0);
				feature->setIFVariable(var);
			}
		}
		const Box& viewLevelBox = vectDomains[viewLevel].domainBox();
		const IntVect& bigEnd = viewLevelBox.bigEnd();
		const IntVect& smallEnd = viewLevelBox.smallEnd();
#if CH_SPACEDIM == 3
		for (int k = smallEnd[2]; k <= bigEnd[2]; k ++) {
#endif
			for (int j = smallEnd[1]; j <= bigEnd[1]; j ++) {
				for (int i = smallEnd[0]; i <= bigEnd[0]; i ++) {
					IntVect viewLevelGridIndex(D_DECL(i, j, k));
					int volIndex = getVolumeIndex(vectNxes[viewLevel], viewLevelGridIndex);
					RealVect a_point = EBArith::getIVLocation(viewLevelGridIndex, vectDxes[viewLevel], chomboGeometry->getDomainOrigin());
					for (int d = 0; d < numSubdomains; ++ d)
					{
						ChomboIF* chomboIf = chomboGeometry->getChomboIF(d);
						Feature* feature = chomboIf->getFeature();
						VolumeVariable* var = feature->getIFVariable();
						var->getCurr()[volIndex] = chomboIf->value(a_point);
					}
				}
			}
#if CH_SPACEDIM == 3
		}
#endif
		bIFVariableUpdated = true;
	}
	pout() << "Exit " << methodName << endl;
}
#endif

void ChomboScheduler::writeData(char* filename, bool convertChomboData) {
	const char* methodName = "(ChomboScheduler::writeData)";
	pout() << "Entry " << methodName << endl;

	if (convertChomboData)
	{
#ifdef CH_MPI
		throw "Chombo data conversion is not supported in MPI Parallel";
#else
		updateSolutionFromChomboOutputFile();  // read from file into vcell variables
#endif
	}
	else
	{
		updateSolutionFromLevelData();  // read from level data into vcell variables
	}

#ifndef CH_MPI
	if (chomboSpec->isSaveVCellOutput())
	{
		DataSet::write(simulation, filename);
	}
#endif
	
	if (chomboSpec->isSaveChomboOutput())
	{
		string firstHdf5File;
		for (int iphase = 0; iphase < NUM_PHASES; iphase ++) {
			for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++) {
				Feature* feature = phaseVolumeList[iphase][ivol]->feature;
				if (feature == NULL || feature->getNumDefinedVariables() == 0)
				{
					pout() << methodName << " feature not found or no variables defined in feature " << (feature == NULL ? "" : feature->getName()) << endl;
					continue;
				}
				char hdf5FileName[FILE_NAME_BUFSIZE];
				sprintf(hdf5FileName, CHOMBO_OUTPUT_FILE_NAME_FORMAT, SimTool::getInstance()->getBaseFileName(), simulation->getCurrIteration(), feature->getName().c_str(), ivol);
				pout() << methodName << " writeEBHDF5, [iphase, ivol]=[" << iphase << "," << ivol << "] to " << hdf5FileName << endl;

				if (firstHdf5File.empty())
				{
					firstHdf5File = hdf5FileName;
				}
				
				Vector<string> names(feature->getNumDefinedVariables());
				for (int ivar = 0; ivar < feature->getNumDefinedVariables(); ivar ++) {
					Variable* var = feature->getDefinedVariable(ivar);
					names[ivar] = var->getName();
				}

				bool replaceCovered = false;
				Vector<Real> dummy;
				IntVect zeroGhost = IntVect::Zero; // not writing ghost
								 
				writeEBHDF5(string(hdf5FileName), vectGrids, volSoln[iphase][ivol], names,
					 vectDomains[0].domainBox(), vectDxes[0][0], simulation->getDT_sec(), simulation->getTime_sec(),
					 vectRefRatios, numLevels, replaceCovered, dummy, zeroGhost);
			} // ivol
		} // iphase

#ifdef CH_MPI
		MPI_Barrier(MPI_COMM_WORLD);
#endif

		pout() << methodName << "firstHdf5File=" << firstHdf5File << endl;
		if (!firstHdf5File.empty())
		{
			hid_t file_access = H5P_DEFAULT;
#ifdef CH_MPI
			file_access = H5Pcreate(H5P_FILE_ACCESS);
			H5Pset_fapl_mpio(file_access,  MPI_COMM_WORLD, MPI_INFO_NULL);
#endif
	
			hid_t h5SimFile =  H5Fopen(firstHdf5File.c_str(), H5F_ACC_RDWR, file_access);
			pout() << methodName << " writing membrane solution to first hdf5 file " << firstHdf5File << endl;
			DataSet::writeMembraneSolution(simulation, h5SimFile
#ifdef CH_MPI
			, memIndexOffset, totalNumMembranePoints
#endif
			);
			
			pout() << methodName << " writing extrapolated valuesto first hdf5 file " << firstHdf5File << endl;
			DataSet::writeExtrapolatedValues(simulation, h5SimFile
#ifdef CH_MPI
			, memIndexOffset, totalNumMembranePoints
#endif
			);
			H5Fclose(h5SimFile);
		}
	}

	hdf5FileCount ++;
	pout() << "Exit " << methodName << endl;
}

#define MESH_GROUP "/mesh"
#define MESH_ATTR_DIMENSION "dimension"
#define MESH_ATTR_ORIGIN "origin"
#define MESH_ATTR_NUM_LEVELS "numLevels"
#define MESH_ATTR_REFINE_RATIOS "refineRatios"
#define MESH_ATTR_VIEW_LEVEL "viewLevel"
#define MESH_ATTR_EXTENT "extent"
#define MESH_ATTR_NX "Nx"
#define MESH_ATTR_DX "Dx"
#define BOXES_GROUP MESH_GROUP"/boxes"
#define BOXES_LEVEL_DATASET_PREFIX BOXES_GROUP"/level_"
#define MEMBRANE_ELEMENTS_DATASET MESH_GROUP"/membrane elements"
#define STRUCTURES_DATASET MESH_GROUP"/structures"
#define FETUREPHASEVOLS_DATASET MESH_GROUP"/featurephasevols"
#define MEMBRANEIDS_DATASET MESH_GROUP"/membraneids"
#define VERTICES_DATASET MESH_GROUP"/vertices"
#if CH_SPACEDIM == 2
#define SEGMENTS_DATASET MESH_GROUP"/segments"
#else
#define SURFACE_DATASET MESH_GROUP"/surface triangles"
#define SLICE_VIEW_DATASET MESH_GROUP"/slice view"
#endif

hid_t ChomboScheduler::createIntVectDataType()
{
	hid_t intVectType = H5Tcreate(H5T_COMPOUND, sizeof(IntVect));
	D_TERM(H5Tinsert(intVectType, "i", HOFFSET(IntVect, dataPtr()[0]), H5T_NATIVE_INT);,
				 H5Tinsert(intVectType, "j", HOFFSET(IntVect, dataPtr()[1]), H5T_NATIVE_INT);,
				 H5Tinsert(intVectType, "k", HOFFSET(IntVect, dataPtr()[2]), H5T_NATIVE_INT);)
	return intVectType;
}

hid_t ChomboScheduler::createRealVectDataType()
{
	hid_t realVectType = H5Tcreate(H5T_COMPOUND, sizeof(RealVect));
	D_TERM(H5Tinsert(realVectType, "x", HOFFSET(RealVect, dataPtr()[0]), H5T_NATIVE_DOUBLE);,
				 H5Tinsert(realVectType, "y", HOFFSET(RealVect, dataPtr()[1]), H5T_NATIVE_DOUBLE);,
				 H5Tinsert(realVectType, "z", HOFFSET(RealVect, dataPtr()[2]), H5T_NATIVE_DOUBLE);)
	return realVectType;
}

struct MeshBox
{
	IntVect lo, hi;
	static hid_t createH5Type()
	{
		hid_t boxType = H5Tcreate(H5T_COMPOUND, sizeof(MeshBox));
		D_TERM(H5Tinsert(boxType, "lo_i", HOFFSET(MeshBox, lo[0]), H5T_NATIVE_INT);,
					 H5Tinsert(boxType, "lo_j", HOFFSET(MeshBox, lo[1]), H5T_NATIVE_INT);,
					 H5Tinsert(boxType, "lo_k", HOFFSET(MeshBox, lo[2]), H5T_NATIVE_INT);)
		D_TERM(H5Tinsert(boxType, "hi_i", HOFFSET(MeshBox, hi[0]), H5T_NATIVE_INT);,
					 H5Tinsert(boxType, "hi_j", HOFFSET(MeshBox, hi[1]), H5T_NATIVE_INT);,
					 H5Tinsert(boxType, "hi_k", HOFFSET(MeshBox, hi[2]), H5T_NATIVE_INT);)
		return boxType;
	}
};

struct MembraneElementMetrics
{
	int index;
	int level;
	IntVect gridIndex;
	RealVect coord, normal;
	double volumeFraction,areaFraction;
	int membraneId;
	unsigned short cornerPhaseMask;

	static hid_t createH5Type()
	{
		hid_t metricsType = H5Tcreate(H5T_COMPOUND, sizeof(MembraneElementMetrics));
		H5Tinsert(metricsType, "index", HOFFSET(MembraneElementMetrics, index), H5T_NATIVE_INT);
		H5Tinsert(metricsType, "level", HOFFSET(MembraneElementMetrics, level), H5T_NATIVE_INT);
		D_TERM(H5Tinsert(metricsType, "i", HOFFSET(MembraneElementMetrics, gridIndex[0]), H5T_NATIVE_INT);,
					 H5Tinsert(metricsType, "j", HOFFSET(MembraneElementMetrics, gridIndex[1]), H5T_NATIVE_INT);,
					 H5Tinsert(metricsType, "k", HOFFSET(MembraneElementMetrics, gridIndex[2]), H5T_NATIVE_INT);)
		D_TERM(H5Tinsert(metricsType, "x", HOFFSET(MembraneElementMetrics, coord[0]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(metricsType, "y", HOFFSET(MembraneElementMetrics, coord[1]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(metricsType, "z", HOFFSET(MembraneElementMetrics, coord[2]), H5T_NATIVE_DOUBLE);)
		D_TERM(H5Tinsert(metricsType, "normalX", HOFFSET(MembraneElementMetrics, normal[0]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(metricsType, "normalY", HOFFSET(MembraneElementMetrics, normal[1]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(metricsType, "normalZ", HOFFSET(MembraneElementMetrics, normal[2]), H5T_NATIVE_DOUBLE);)
		H5Tinsert(metricsType, "volumeFraction", HOFFSET(MembraneElementMetrics, volumeFraction), H5T_NATIVE_DOUBLE);
		H5Tinsert(metricsType, "areaFraction", HOFFSET(MembraneElementMetrics, areaFraction), H5T_NATIVE_DOUBLE);
		H5Tinsert(metricsType, "membraneId", HOFFSET(MembraneElementMetrics, membraneId), H5T_NATIVE_INT);
		H5Tinsert(metricsType, "cornerPhaseMask", HOFFSET(MembraneElementMetrics, cornerPhaseMask), H5T_NATIVE_USHORT);
		return metricsType;
	}
};


struct StructureMetrics
{
	char name[128];
	char type[128];
	double size;
	int numPoints;

	static hid_t createH5Type()
	{
		hid_t metricsType = H5Tcreate(H5T_COMPOUND, sizeof(StructureMetrics));
		hid_t strType = H5Tcreate(H5T_STRING, sizeof(char) * 128);
		H5Tinsert(metricsType, "name", HOFFSET(StructureMetrics, name), strType);
		H5Tinsert(metricsType, "type", HOFFSET(StructureMetrics, type), strType);
		H5Tinsert(metricsType, "size", HOFFSET(StructureMetrics, size), H5T_NATIVE_DOUBLE);
		H5Tinsert(metricsType, "numPoints", HOFFSET(StructureMetrics, numPoints), H5T_NATIVE_INT);
		H5Tclose(strType);
		return metricsType;
	}
};

struct FeaturePhaseVol
{
	char feature[128];
	int iphase;
	int ivol;

	static hid_t createH5Type()
	{
		hid_t metricsType = H5Tcreate(H5T_COMPOUND, sizeof(FeaturePhaseVol));
		hid_t strType = H5Tcreate(H5T_STRING, sizeof(char) * 128);
		H5Tinsert(metricsType, "feature", HOFFSET(FeaturePhaseVol, feature), strType);
		H5Tinsert(metricsType, "iphase", HOFFSET(FeaturePhaseVol, iphase), H5T_NATIVE_INT);
		H5Tinsert(metricsType, "ivol", HOFFSET(FeaturePhaseVol, ivol), H5T_NATIVE_INT);
		H5Tclose(strType);
		return metricsType;
	}
};

struct MembraneId
{
	int id;
	char membrane[128];

	static hid_t createH5Type()
	{
		hid_t metricsType = H5Tcreate(H5T_COMPOUND, sizeof(MembraneId));
		hid_t strType = H5Tcreate(H5T_STRING, sizeof(char) * 128);
		H5Tinsert(metricsType, "id", HOFFSET(MembraneId, id), H5T_NATIVE_INT);
		H5Tinsert(metricsType, "membrane", HOFFSET(MembraneId, membrane), strType);
		H5Tclose(strType);
		return metricsType;
	}
};

struct Vertex
{
	RealVect coords;

	static hid_t createH5Type()
	{
		hid_t vertexType = H5Tcreate(H5T_COMPOUND, sizeof(Vertex));
		D_TERM(H5Tinsert(vertexType, "x", HOFFSET(Vertex, coords[0]), H5T_NATIVE_DOUBLE);,
					H5Tinsert(vertexType, "y", HOFFSET(Vertex, coords[1]), H5T_NATIVE_DOUBLE);,
					H5Tinsert(vertexType, "z", HOFFSET(Vertex, coords[2]), H5T_NATIVE_DOUBLE);)
		return vertexType;
	}
};

#if CH_SPACEDIM == 2
struct Segment
{
	int index;
	int vertexIndexes[2];
	int neighborIndexes[2];
	Segment()
	{
		vertexIndexes[0] = vertexIndexes[1] = -1;
		neighborIndexes[0] = neighborIndexes[1] = MEMBRANE_NEIGHBOR_UNKNOWN;
	}

	static hid_t createH5Type()
	{
		hid_t segmentType = H5Tcreate(H5T_COMPOUND, sizeof(Segment));
		H5Tinsert(segmentType, "index", HOFFSET(Segment, index), H5T_NATIVE_INT);
		H5Tinsert(segmentType, "vertex_1", HOFFSET(Segment, vertexIndexes[0]), H5T_NATIVE_INT);
		H5Tinsert(segmentType, "vertex_2", HOFFSET(Segment, vertexIndexes[1]), H5T_NATIVE_INT);
		H5Tinsert(segmentType, "neighbor_1", HOFFSET(Segment, neighborIndexes[0]), H5T_NATIVE_INT);
		H5Tinsert(segmentType, "neighbor_2", HOFFSET(Segment, neighborIndexes[1]), H5T_NATIVE_INT);
		return segmentType;
	}
};
#else
struct Triangle
{
	int memIndex;
	int face;
	int neighborIndex;
	RealVect triVertices[3];

	static hid_t createH5Type()
	{
		hid_t triangleType = H5Tcreate(H5T_COMPOUND, sizeof(Triangle));
		H5Tinsert(triangleType, "index", HOFFSET(Triangle, memIndex), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "face", HOFFSET(Triangle, face), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "neighborIndex", HOFFSET(Triangle, neighborIndex), H5T_NATIVE_INT);
		D_TERM(H5Tinsert(triangleType, "x0", HOFFSET(Triangle, triVertices[0][0]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(triangleType, "y0", HOFFSET(Triangle, triVertices[0][1]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(triangleType, "z0", HOFFSET(Triangle, triVertices[0][2]), H5T_NATIVE_DOUBLE);)
		D_TERM(H5Tinsert(triangleType, "x1", HOFFSET(Triangle, triVertices[1][0]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(triangleType, "y1", HOFFSET(Triangle, triVertices[1][1]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(triangleType, "z1", HOFFSET(Triangle, triVertices[1][2]), H5T_NATIVE_DOUBLE);)
		D_TERM(H5Tinsert(triangleType, "x2", HOFFSET(Triangle, triVertices[2][0]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(triangleType, "y2", HOFFSET(Triangle, triVertices[2][1]), H5T_NATIVE_DOUBLE);,
					 H5Tinsert(triangleType, "z2", HOFFSET(Triangle, triVertices[2][2]), H5T_NATIVE_DOUBLE);)
		return triangleType;
	}
};

struct SurfaceTriangle
{
	int memIndex;
	int face;
	int neighborIndexes[3];
	int triVertices[3];
	bool oriented;
	SurfaceTriangle()
	{
		triVertices[0] = triVertices[1] = triVertices[2] = -1;
		neighborIndexes[0] = neighborIndexes[1] = neighborIndexes[2] = MEMBRANE_NEIGHBOR_UNKNOWN;
		oriented = false;
	}

	static hid_t createH5Type()
	{
		hid_t triangleType = H5Tcreate(H5T_COMPOUND, sizeof(Triangle));
		H5Tinsert(triangleType, "membrane index", HOFFSET(SurfaceTriangle, memIndex), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "face", HOFFSET(SurfaceTriangle, face), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "neighbor 0", HOFFSET(SurfaceTriangle, neighborIndexes[0]), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "neighbor 1", HOFFSET(SurfaceTriangle, neighborIndexes[1]), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "neighbor 2", HOFFSET(SurfaceTriangle, neighborIndexes[2]), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "vertex 0", HOFFSET(SurfaceTriangle, triVertices[0]), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "vertex 1", HOFFSET(SurfaceTriangle, triVertices[1]), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "vertex 2", HOFFSET(SurfaceTriangle, triVertices[2]), H5T_NATIVE_INT);
		H5Tinsert(triangleType, "oriented", HOFFSET(SurfaceTriangle, oriented), H5T_NATIVE_HBOOL);
		return triangleType;
	}
};

struct SliceView
{
	int index;
	double crossPoints[3][4];
	int vertices[3][2];

	static hid_t createH5Type()
	{
		hid_t sliceViewType = H5Tcreate(H5T_COMPOUND, sizeof(SliceView));
		H5Tinsert(sliceViewType, "index", HOFFSET(SliceView, index), H5T_NATIVE_INT);
		H5Tinsert(sliceViewType, "x_y1", HOFFSET(SliceView, crossPoints[0][0]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "x_z1", HOFFSET(SliceView, crossPoints[0][1]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "x_y2", HOFFSET(SliceView, crossPoints[0][2]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "x_z2", HOFFSET(SliceView, crossPoints[0][3]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "y_x1", HOFFSET(SliceView, crossPoints[1][0]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "y_z1", HOFFSET(SliceView, crossPoints[1][1]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "y_x2", HOFFSET(SliceView, crossPoints[1][2]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "y_z2", HOFFSET(SliceView, crossPoints[1][3]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "z_x1", HOFFSET(SliceView, crossPoints[2][0]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "z_y1", HOFFSET(SliceView, crossPoints[2][1]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "z_x2", HOFFSET(SliceView, crossPoints[2][2]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "z_y2", HOFFSET(SliceView, crossPoints[2][3]), H5T_NATIVE_DOUBLE);
		H5Tinsert(sliceViewType, "x_v1", HOFFSET(SliceView, vertices[0][0]), H5T_NATIVE_INT);
		H5Tinsert(sliceViewType, "x_v2", HOFFSET(SliceView, vertices[0][1]), H5T_NATIVE_INT);
		H5Tinsert(sliceViewType, "y_v1", HOFFSET(SliceView, vertices[1][0]), H5T_NATIVE_INT);
		H5Tinsert(sliceViewType, "y_v2", HOFFSET(SliceView, vertices[1][1]), H5T_NATIVE_INT);
		H5Tinsert(sliceViewType, "z_v1", HOFFSET(SliceView, vertices[2][0]), H5T_NATIVE_INT);
		H5Tinsert(sliceViewType, "z_v2", HOFFSET(SliceView, vertices[2][1]), H5T_NATIVE_INT);
		return sliceViewType;
	}
};
#endif

#if CH_SPACEDIM == 2
int ChomboScheduler::findNeighborMembraneIndex2D(int iphase, int ilev, const IntVect& gridIndex,
				int iedge, const RealVect& cp, const RealVect& cpcoords, int& neighborEdge)
{
//	const char* methodName = "(ChomboScheduler::findNeighborMembraneIndex2D)";
//	pout() << "Entry " << methodName << ", gridIndex=" << gridIndex << ", iedge=" << iedge << endl;

	bool bHasNeighbor = false;
	IntVect diff = IntVect::Zero;
	int idir = iedge / Side::NUMSIDES;
	int hilo = iedge % Side::NUMSIDES;

	bool bNextToWall = false;
	if (hilo == Side::Lo)
	{
		bNextToWall = gridIndex[idir] == 0;
		diff[idir] = -1;
	}
	else if (hilo == Side::Hi)
	{
		 bNextToWall = gridIndex[idir] == vectNxes[ilev][idir] - 1;
		 diff[idir] = +1;
	}

	static double cornerTol = 1.e-12;
	int neighborLocalMemIndex = MEMBRANE_NEIGHBOR_UNKNOWN;
	int otherDir = (idir + 1) % 2;
	if (bNextToWall)
	{
		neighborLocalMemIndex = MEMBRANE_NEIGHBOR_NEXT_TO_WALL;
	}
	else
	{
		bHasNeighbor = true;

		// cross point exactly on the corner
		if (abs(cp[otherDir]) >= (0.5-cornerTol))
		{
			double e = 0.25;
			RealVect nearByPointOffsets[4] =
			{
				RealVect(0, -e * cp[1] * vectDxes[ilev][1]),
				RealVect(0, e * cp[1] * vectDxes[ilev][1]),
				RealVect(-e * cp[0] * vectDxes[ilev][0], 0),
				RealVect(e * cp[0] * vectDxes[ilev][0], 0),
			};
			bool F[4];
			for (int i = 0; i < 4; ++ i)
			{
				F[i] = geoIfs[iphase]->value(cpcoords + nearByPointOffsets[i]) < 0;
			}
			if (F[0] == F[2])
			{
				bHasNeighbor = false;
//				stringstream ss;
//				ss << "cross-point calculation: membrane is tangent to an element face. "
//								"It may help if domain origin is shifted by a small number. F=["
//					<< F[0] << "," << F[1] << "," << F[2] << "," << F[3] << "], [cp, cpcoords]=["
//					<< cp << "," << cpcoords << "]";
//				throw ss.str();
			}
			else if ((F[1] == F[2]) && (F[0] == F[3]))
			{
				// diagonal
				diff[idir]     = cp[idir] < 0 ? -1 : +1;
				diff[otherDir] = cp[otherDir] < 0 ? -1 : +1;
				// define a different neighborEdge here:
				if (cp[otherDir] > 0)
				{
					neighborEdge = 2 * otherDir;
				}
				else
				{
					neighborEdge = 2 * otherDir + 1;
				}
			}
			else if (F[1] != F[2])
			{
				// y direction
				diff[0] = 0;
				diff[1] = cp[1] < 0 ? -1 : +1;
				if (idir == 0)
				{
					neighborEdge = iedge;
				}
			}
			else if (F[0] != F[3])
			{
				// x direction
				diff[0] = cp[0] < 0 ? -1 : +1;
				diff[1] = 0;
				if (idir == 1)
				{
					neighborEdge = iedge;
				}
			}
			else
			{
				bHasNeighbor = false;
			}
		}
	}

	// check again if neighbor could be next to the wall
	if (bHasNeighbor)
	{
		for (int idir = 0; idir < SpaceDim; ++ idir)
		{
			if (gridIndex[idir] == 0 && diff[idir] == -1
						||	gridIndex[idir] == vectNxes[ilev][idir] - 1 && diff[idir] == +1 )
			{
				bHasNeighbor = false;
				neighborLocalMemIndex = MEMBRANE_NEIGHBOR_NEXT_TO_WALL;
				break;
			}
		}
	}
	if (bHasNeighbor)
	{
		IntVect neighborGridIndex = gridIndex + diff;
		int volIndex = getVolumeIndex(vectNxes[ilev], neighborGridIndex);
		map<int,int>::iterator iter = irregVolumeMembraneMap[ilev].find(volIndex);
		if (iter == irregVolumeMembraneMap[ilev].end())
		{
			stringstream ss;
			ss << "Error finding neighbor membrane element: Volume element " << volIndex << " in level " << ilev << " is not an irregular point";
			//throw ss.str();
		}
		else if (iter->second == MEMBRANE_INDEX_IN_FINER_LEVEL)
		{
			int rr = vectRefRatios[ilev];
			IntVect fineGridIndex = gridIndex * rr;
			if (diff[idir] != 0)
			{
				// most likely case
				if (hilo == Side::Hi)
				{
					fineGridIndex[idir] = fineGridIndex[idir] + rr;
				}
				else
				{
					fineGridIndex[idir] = fineGridIndex[idir] - 1;
				}
			}
			else
			{
				// unusual case when eb becomes tangent at corner and neighbor is in otherdir direction:
				if (hilo == Side::Hi)
				{
					fineGridIndex[idir] = fineGridIndex[idir] + rr - 1;
				}
				else
				{
					fineGridIndex[idir] = fineGridIndex[idir];
				}
			}
						//
			double cp_tmp = (cp[otherDir] + 0.5) * rr;
			if (diff[otherDir] == 0)
			{
				fineGridIndex[otherDir] = fineGridIndex[otherDir] + std::min<int>((rr - 1), int(cp_tmp));
			}
			else
			{
				// case when neighbor is diagonal
				if (diff[otherDir] == -1)
				{
					fineGridIndex[otherDir] = fineGridIndex[otherDir] - 1;
				}
				else if (diff[otherDir] == 1)
				{
					fineGridIndex[otherDir] = fineGridIndex[otherDir] + rr ;
				}
			}

			int nextLevel = ilev + 1;
			int fineVolIndex = getVolumeIndex(vectNxes[nextLevel], fineGridIndex);
			map<int, int>::iterator fiter = irregVolumeMembraneMap[nextLevel].find(fineVolIndex);
			if (fiter == irregVolumeMembraneMap[nextLevel].end())
			{
				stringstream ss;
				ss << " Volume element " << fineVolIndex << " in finer level is not irregular (testing neighbor)";
				if (cp_tmp - int(cp_tmp) > 0.5)
				{
					// try next element up (diagonal in refined level?)
					fineGridIndex[otherDir] = fineGridIndex[otherDir] + 1;
				}
				else
				{
					// try next element down (diagonal in refined level? )
					fineGridIndex[otherDir] = fineGridIndex[otherDir] - 1;
				}
				fineVolIndex = getVolumeIndex(vectNxes[nextLevel], fineGridIndex);
				map<int,int>::iterator fiter = irregVolumeMembraneMap[nextLevel].find(fineVolIndex);
				if (fiter == irregVolumeMembraneMap[nextLevel].end())
				{
					stringstream ss;
					ss << "warning: Volume element " << fineVolIndex << " in finer level is ALSO not irregular ?";
				}
				else
				{
					neighborLocalMemIndex = fiter->second;
				}
			}
			else
			{
				neighborLocalMemIndex = fiter->second;
			}
		}
		else
		{
			neighborLocalMemIndex = iter->second;
		}
	}
//	pout() << "Exit " << methodName << endl;
	
	return neighborLocalMemIndex;
}
#else
bool ChomboScheduler::assignEdgeVertArray(int ilev, IntVect& nGridIndex, bool isCorner, int otherEdge, int vertexIndex, int (*edgeVertArray)[21])
{
//	const char* methodName = "(ChomboScheduler::assignEdgeVertArray)";
//	pout() << "Entry " << methodName << endl;

	bool bKeepGoing = true;
	if ((nGridIndex[0] < vectNxes[ilev][0]) && (nGridIndex[1] < vectNxes[ilev][1]) && (nGridIndex[2] < vectNxes[ilev][2])) // if not  on wall
	{
		int volIndex = getVolumeIndex(vectNxes[ilev], nGridIndex);
		map<int,int>::iterator iter = irregVolumeMembraneMap[ilev].find(volIndex);
		// check if neighbor has a valid membrane element index and fill vertex for neighbor
		if (iter == irregVolumeMembraneMap[ilev].end())
		{
			// did not find membrane index; check if it is a corner point; if not, issue a warning message for now
			if (!isCorner)
			{
				pout() << "volIndex"<< volIndex<<" in level"<< ilev << "should be irregular, should have a membrane element" << endl;
				// keep going for now because there may be other reason why the index is not set
			}
		}
		else
		{
			if  (iter->second != MEMBRANE_INDEX_IN_FINER_LEVEL)
			{
				int neighborGlobalMemIndex = iter->second; // neighbor's membrane element index
				int neighborLocalMemIndex = neighborGlobalMemIndex - memIndexOffset;
				if (neighborLocalMemIndex >= 0 && neighborLocalMemIndex < numMembranePoints)
				{
					// only assign values in my range
					edgeVertArray[neighborLocalMemIndex][otherEdge] = vertexIndex;
				}
			}
			//bKeepGoing = false; // neighbor in finer level, continue
		}
	}
//	pout() << "Exit " << methodName << endl;
	return bKeepGoing;
}

IntVect ChomboScheduler::orientVertices(RealVect* vertices, RealVect& outNormal)
{
	IntVect newOrder(0, 1, 2);
	// check the orientation (later)
	RealVect a = vertices[0]-vertices[2];
	RealVect b = vertices[1]-vertices[2];
	//compute cross product of a and b and make it unit size
  RealVect newNormal = PolyGeom::cross(a, b);
	newNormal /= newNormal.vectorLength(); // normalize

	// compute dot product with normal to see if they are pointing in the same direction
	double Dtest = newNormal.dotProduct(outNormal);
    //	same as	double Dtest = norm(PolyGeom::dot(newNormal, outNormal));
	if (Dtest<0) {
		// if pointing different than the normal, we need to reorder the points
		newOrder = IntVect(1, 0, 2);
	}
	return newOrder;
}
#endif

void ChomboScheduler::writeMembraneFiles()
{
	const char* methodName = "(ChomboScheduler::writeMembraneFiles)";
	pout() << "Entry " << methodName << endl;
	
	MembraneElementMetrics* metricsData = new MembraneElementMetrics[numMembranePoints];
#if CH_SPACEDIM == 2
	int vertexCount = 0;
	Segment* segmentList = new Segment[numMembranePoints];
	int* edgeVertices = new int[numMembranePoints * 4];
	std::fill(edgeVertices, edgeVertices + numMembranePoints * 4, -1);
	Vertex* vertexList = new Vertex[numMembranePoints * 2];
#else
	int triangleCount = 0;
	// old triangles
	Triangle* surfaceData = new Triangle[numMembranePoints * 6];
	// new triangles
	Vertex* vertexList = new Vertex[numMembranePoints * 8];

	const RealVect& origin = getChomboGeometry()->getDomainOrigin();
	Real minOrigin = std::min<Real>(std::min<Real>(origin[0], origin[1]), origin[2]);
	Real sliceCrossPointDefaultValue = minOrigin - 1;
	SliceView* sliceViewData = new SliceView[numMembranePoints];

	int triangleVertexCount = -1;
	SurfaceTriangle* membraneTriangles = new SurfaceTriangle[numMembranePoints * 6];
	// number all the edges in a cube from 0 to 11 (6 faces, 4 edges per face, each edge index is repeated twice)
	// the order is not important as long as it is used consistently
	int edgeIndex[6][4] = { {0, 2, 4, 8},  {1, 3, 5, 9} , {0, 1, 6, 10} , {2, 3, 7, 11} , {4, 5, 6, 7} , {8, 9, 10, 11} };
	// array for cross point indexes - they become vertices of the surface triangles
	// first 12 entries [0]:[11] are indexes for cross pts at edges of the volume element (for edges that have a cross point)
	// entry 13 [12] is index for membrane centroid
	// last 8 entries [13]:[21] are 0 or 1 - 1 if crosspoint is in a corner, 0 if not
	int (*tempVertIndexesArray)[21] = new int[numMembranePoints][21];
	
	int numEdges = 12;
	int memCentroidOffset = numEdges;       // 12
	int cornerOffset = memCentroidOffset+1; // 13 - make sure (length - cornerOffset) equals 8

	for (int imem = 0; imem < numMembranePoints; ++ imem)
	{
		for (int j = 0; j < cornerOffset; ++ j)
		{
			tempVertIndexesArray[imem][j] = MEMBRANE_INDEX_INVALID;
		}
		for (int j = cornerOffset; j < 21; ++ j)
		{
			tempVertIndexesArray[imem][j] = 0;
		}
	}
#endif
	
	// only look at phase 0 for membranes
	pout() << methodName << " computing segments/triangles and vertices" << endl;
	for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ++ ivol)
	{
		for (int ilev = 0; ilev < numLevels; ++ ilev)
		{
			ChomboGeometryShop chomboGeoShop(geoIfs[phase0], vectDxes[ilev]);

			DisjointBoxLayout& currGrids = vectGrids[ilev];

			for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++ dit)
			{
				const EBISBox& currEBISBox = vectEbis[phase0][ivol][ilev][dit()];
				const Box& currBox = vectGrids[ilev][dit()];

				const EBGraph& currEBGraph = currEBISBox.getEBGraph();
				IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);
				for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++ vofit)
				{
					const VolIndex& vof = vofit();
					int globalMemIndex = (*irregularPointMembraneIndex[phase0][ivol][ilev])[dit()](vof, 0);
					if (globalMemIndex == MEMBRANE_INDEX_IN_FINER_LEVEL)
					{
						continue;
					}
					
					const IntVect& gridIndex = vof.gridIndex();
					RealVect vol_center = EBArith::getVofLocation(vof, vectDxes[ilev], chomboGeometry->getDomainOrigin());
					const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);
					RealVect mem_point = mem_centroid;
					mem_point *= vectDxes[ilev];
					mem_point += vol_center;

					int localMemIndex = globalMemIndex - memIndexOffset;
					metricsData[localMemIndex].index = globalMemIndex;
					metricsData[localMemIndex].level = ilev;
					metricsData[localMemIndex].membraneId = (*irregularPointMembraneIDs[phase0][ivol][ilev])[dit()](vof, 0);;
					metricsData[localMemIndex].gridIndex = gridIndex;
					metricsData[localMemIndex].coord = mem_point;
					metricsData[localMemIndex].normal = currEBISBox.normal(vof);
					metricsData[localMemIndex].areaFraction = currEBISBox.bndryArea(vof);
					metricsData[localMemIndex].volumeFraction = currEBISBox.volFrac(vof);
					metricsData[localMemIndex].cornerPhaseMask = 0;

					// compute corner phase mask
					RealVect dP[2] = {-0.5 * vectDxes[ilev], 0.5 * vectDxes[ilev]};
					RealVect P = vol_center;

					int cindex = 0;
#if CH_SPACEDIM == 3
					for (int k = 0; k < 2; ++ k)
					{
						P[2] = vol_center[2] + dP[k][2];
#endif
						for (int j = 0; j < 2 ; ++ j)
						{
							P[1] = vol_center[1] + dP[j][1];
							for (int i = 0; i < 2 ; ++ i)
							{
								P[0] = vol_center[0] + dP[i][0];
								if (geoIfs[phase0]->value(P) > 0)
								{
									metricsData[localMemIndex].cornerPhaseMask |= (0x1 << cindex);
								}
								++ cindex;
							}
						}
#if CH_SPACEDIM == 3
					}
#endif

#if CH_SPACEDIM == 2
					segmentList[localMemIndex].index = globalMemIndex;
					edgeMo edges[4];

					bool faceCovered;
					bool faceRegular;
					bool faceDontKnow;

					//get edgeType and intersection points
					chomboGeoShop.edgeData2D(edges,
										 faceCovered,
										 faceRegular,
										 faceDontKnow,
										 vectDxes[ilev][0],
										 vectDxes[ilev],
										 vof.gridIndex(),
										 vectDomains[ilev],
										 chomboGeometry->getDomainOrigin());
					int crossedEdgeCount = -1;
					int iedge = -1;
					for (int idir = 0; idir < SpaceDim; ++ idir)
					{
						for (int lohi = Side::Lo; lohi < Side::NUMSIDES;  ++ lohi)
						{
							++ iedge;
							if (edgeVertices[localMemIndex * 4 + iedge] >= 0)
							{
								++ crossedEdgeCount;
								continue;
							}
							bool irreg = edges[iedge].dontKnow();
							if (irreg)
							{
								++ crossedEdgeCount;
								if (crossedEdgeCount < 2)
								{
									RealVect cp = (edges[iedge].getIntersectLo()) ? edges[iedge].getLo() : edges[iedge].getHi();
									RealVect cross_point = cp;
									cross_point *= vectDxes[ilev];
									cross_point += vol_center;

									int neighborEdge = (iedge ^ 1);
									int neighborGlobalMemIndex  = findNeighborMembraneIndex2D(phase0, ilev, gridIndex, iedge, cp, cross_point, neighborEdge);
									if (neighborGlobalMemIndex == MEMBRANE_NEIGHBOR_UNKNOWN)
									{
										pout() << "did not find neighbor at edge " << neighborEdge << " for membrane point (not next to wall) globalMemIndex=" << globalMemIndex << ", gridIndex=" << gridIndex << endl;
									}
									else
									{
										// determine if vertex is first or second
										// compute Q (as membrane enters the volume element from outside
										// find a point to the right of cross point, and compute fQ=implF(Q);
										RealVect Vp = mem_point - cross_point;
										RealVect Vtan(-metricsData[localMemIndex].normal[1], metricsData[localMemIndex].normal[0]);
										double Dtest = Vp.dotProduct(Vtan);
										int orderV, orderN;
										if (Dtest > 0)
										{
											orderV = 0;
											orderN = 1;
										}
										else
										{
											orderV = 1;
											orderN = 0;
										}

										// set neighbor vertex and neighbor
										segmentList[localMemIndex].vertexIndexes[orderV] = vertexCount;
										segmentList[localMemIndex].neighborIndexes[orderV] = neighborGlobalMemIndex;
										edgeVertices[localMemIndex * 4 + iedge] = vertexCount;
										if (neighborGlobalMemIndex != MEMBRANE_NEIGHBOR_NEXT_TO_WALL)
										{
											int neighborLocalMemIndex = neighborGlobalMemIndex - memIndexOffset;
											// neighbor is also in my domain
											if (neighborLocalMemIndex >= 0 && neighborLocalMemIndex < numMembranePoints)
											{
												segmentList[neighborLocalMemIndex].vertexIndexes[orderN] = vertexCount;
												segmentList[neighborLocalMemIndex].neighborIndexes[orderN] = globalMemIndex;
												edgeVertices[neighborLocalMemIndex * 4 + neighborEdge] = vertexCount;
											}
											else
											{
												pout() << " for membrane index " << globalMemIndex << ", neighbor " << neighborGlobalMemIndex << " is not in my domain" << endl;
											}
										}
										vertexList[vertexCount].coords = cross_point;
										++ vertexCount;
									}
								}
							}
						}
					}
#else
					int faceCount = -1;
					int sliceCrossPointCount[SpaceDim];
					sliceViewData[localMemIndex].index = globalMemIndex;
					for (int dir = 0; dir < SpaceDim; ++ dir)
					{
						sliceCrossPointCount[dir] = 0;
						for (int i = 0; i < 4; ++ i)
						{
							sliceViewData[localMemIndex].crossPoints[dir][i] = sliceCrossPointDefaultValue;
						}
						for (int i = 0; i < 2; ++ i)
						{
							sliceViewData[localMemIndex].vertices[dir][i] = MEMBRANE_INDEX_INVALID;
						}
					}

					/////////
					// add membrane point (centroid) to the list of triangle vertices and set the entry for
					// this vertex in the tempVertIndexesArray
					triangleVertexCount ++;
					vertexList[triangleVertexCount].coords = mem_point;
					tempVertIndexesArray[localMemIndex][memCentroidOffset] = triangleVertexCount;
					
					for (int face = 0; face < SpaceDim; ++ face)
					{
						for (int hiLoFace = 0; hiLoFace < Side::NUMSIDES; ++ hiLoFace)
						{
							++ faceCount;
							edgeMo edges[4];
							bool faceCovered;
							bool faceRegular;
							bool faceDontKnow;

							chomboGeoShop.edgeData3D(edges,
												 faceCovered,
												 faceRegular,
												 faceDontKnow,
												 hiLoFace,
												 face,
												 vectDxes[ilev][0],
												 vectDxes[ilev],
												 vof.gridIndex(),
												 vectDomains[ilev],
												 chomboGeometry->getDomainOrigin());
							int crossedEdgeCount = 0;
							for (int iedge = 0; iedge < 4; ++ iedge)
							{
								bool irreg = edges[iedge].dontKnow();
								if (!irreg)
								{
									continue;
								}
								++ crossedEdgeCount;
								RealVect cp = (edges[iedge].getIntersectLo()) ? edges[iedge].getLo() : edges[iedge].getHi();
								// get the real coordinate
								RealVect cross_point = cp;
								cross_point *= vectDxes[ilev];
								cross_point += vol_center;
								if (crossedEdgeCount < 3)
								{
									surfaceData[triangleCount].triVertices[crossedEdgeCount] = cross_point;
								}

								// note, I will just store this crosspoint even if >= 3

								//////////////////////////////////////////
								//////////////////////////////////////////
								// new code inside of edge loop
								////////////////////////////////////////
								////////////////////////////////////////
								int edgeNumber = edgeIndex[faceCount][iedge]; // find the number for this edge in a cube
								//  the crosspoint may have already been assigned by a neighbor (foundVert may be valid or invalid at this point)
								int foundVert = tempVertIndexesArray[localMemIndex][edgeNumber];
								if (foundVert != MEMBRANE_INDEX_INVALID)  // vertex already set, continue;
								{
									continue;
								}
								//
								// Note to Fei:
								// replace the following 19 lines with a (boolean) function:
								// isCorner = determineCornerStatus(cp,isHiCorner,cornerIndex);
								// if (isCorner) {
								//  	tempVertIndexesArray[memIndex][cornerOffset + cornerIndex] = 1;
								// }
								bool isCorner = false;
								bool isHiCorner = false;
								if ((abs(cp[0]) == 0.5) && (abs(cp[1]) == 0.5) && (abs(cp[2]) == 0.5))
								{
									isCorner = true;
									isHiCorner = ((cp[0] == 0.5) && (cp[1] == 0.5) && (cp[2] == 0.5));
									// compute the column for storing this vertex in tempEdgeVert array
									int cornerIndex = 0;
									int coldir = 1;
									for (int dir = 0; dir < SpaceDim; ++ dir)
									{
										if (cp[dir] >= 0)
										{
											cornerIndex += coldir;
											coldir = coldir*2;
										}
									}
									// set to 1 the column for this corner in the temp vertex array
									tempVertIndexesArray[localMemIndex][cornerOffset + cornerIndex] = 1;
								}
								//
								// end of function call determineCornerStatus
								// since this crosspoint has not been set, create a new vertex and set its index in tempVertIndexesArray
								if ((!isCorner) || isHiCorner)
								{
									++ triangleVertexCount;
									// new vertex: triangleVertexCount is the current index
									vertexList[triangleVertexCount].coords = cross_point;
									tempVertIndexesArray[localMemIndex][edgeNumber] = triangleVertexCount;
								}
								// only interested in hi sides and edges. otherwise we are done
								if ((hiLoFace != Side::Hi) || ((iedge!=1) && (iedge!=3)))
								{
									continue;
								}
						
								// if face is hi and edge is hi, share this vertex with lo neighbors that share
								// a face or an edge, or the corner - there are 7 possible neighbors (if in same level)
								// NOTE: if neighbor is not in the same refinement level, for now do nothing
								if (gridIndex[face] + 1 < vectNxes[ilev][face]) // if not next to wall (this is also checked in assignEdgeVertArray)
								{
									// (1) (2) (3): neighbors that share a face:
									//          face x+, neighbor i+1,j,k; face y+, neighbor i,j+1,k; face z+, neighbor i,j,k+1
									//          otherEdge (for neighbor sharing edge) is the same edge in face x- y- or z-
									IntVect nGridIndex = gridIndex;
									++ nGridIndex[face];
									int otherEdge = edgeIndex[faceCount - 1][iedge];
									bool bKeepGoing = assignEdgeVertArray(ilev, nGridIndex, isCorner, otherEdge, triangleVertexCount, tempVertIndexesArray);
									if (!bKeepGoing) continue;
									// end (1) (2) (3)

									int faceEdgeMask = 6 * face + iedge;
									// (4) (5) (6): neighbors that share an edge
									// (4) 	7			// if ((face == 1) && (iedge == 1))
									// (5)	15			// if ((face == 2) && (iedge == 3))
									if ((faceEdgeMask == 7) || (faceEdgeMask == 15))
									{
										int otherdir = iedge - face;
										// (4) face y+ and edge is the intersection between x+ and y+,
										// neighbor i+1,j+1,k (gridindex + [1 1 0]) find membrane index and edge number of the neighbor
										// (5) face z+, edge is the intersection between y+ and z+,
										// neighbor i,j+1,k+1 (gridindex + [0 1 1]) find membrane index and edge number of the neighbor
										nGridIndex = gridIndex;
										++ nGridIndex[face];
										++ nGridIndex[otherdir];

										if (faceEdgeMask == 7)
										{
											otherEdge = edgeIndex[0][0];
										} 
										else
										{
											otherEdge = edgeIndex[4][2];
										}
										bool bKeepGoing = assignEdgeVertArray(ilev, nGridIndex, isCorner, otherEdge, triangleVertexCount, tempVertIndexesArray);
										if (!bKeepGoing) continue;
									} // end (4) (5)

									// (6) 
									else if (faceEdgeMask == 13)
									{
										int otherdir = 0;
										// edge is the intersection between x+ and z+, for neighbor i+1,j,k+1 and edge number of the neighbor
										nGridIndex = gridIndex;
										++ nGridIndex[face];
										++ nGridIndex[otherdir];

										int otherEdge = edgeIndex[2][0];
										bool bKeepGoing = assignEdgeVertArray(ilev, nGridIndex, isCorner, otherEdge, triangleVertexCount, tempVertIndexesArray);
										if (!bKeepGoing) continue;
									}// end (6)
								} // end if (gridIndex[face] + 1 < vectNxes[ilev][face])
						
								if (isHiCorner)
								{
									// neighbor (7) find membrane index for neighbor i+1,j+1,k+1 and corner number of the neighbor
									IntVect nGridIndex = gridIndex + IntVect(1, 1, 1);

									int cornerIndex = 0;
									int otherEdge = cornerOffset + cornerIndex;
									bool bKeepGoing = assignEdgeVertArray(ilev, nGridIndex, isCorner, otherEdge, 1, tempVertIndexesArray);
									if (!bKeepGoing) continue;
								} // end isHiCorner (7)

							} // end for (iedge)

							/////////////////////////////////////////
							/////////////////////////////////////////
							/// new code outside of the loop
							/////////////////////////////////////////
							/////////////////////////////////////////
							if (crossedEdgeCount >= 3)
							{
								// multi valued point, continue, why?
								stringstream ss;
								ss << "Point " << gridIndex << " has " << crossedEdgeCount << " cross edge points; it is a multi-valued cell.";
								//				<< "Mesh is too coarse to resolve. Use finer mesh or mesh refinement.";
								pout() << ss.str() << endl;
							}
							else if (crossedEdgeCount > 0)
							{
								// initialize new triangle (eventually these will replace surfaceData)
								membraneTriangles[triangleCount].memIndex = globalMemIndex;
								membraneTriangles[triangleCount].face = faceCount;
								////////////////////////////////////////////
								////////////////////////////////////////////
								//// old code outside of edge loop
								///////////////////////////////////////////
								///////////////////////////////////////////

								surfaceData[triangleCount].memIndex = globalMemIndex;
								surfaceData[triangleCount].neighborIndex = MEMBRANE_NEIGHBOR_UNKNOWN;
								surfaceData[triangleCount].face = faceCount;
								surfaceData[triangleCount].triVertices[0] = mem_point;
								for(int dir = 0; dir < SpaceDim; ++ dir)
								{
									RealVect crossPoint;
									bool oneFaceCross = computeOneFaceCross(dir, face, hiLoFace == 0 ? -1 : 1, vectDxes[ilev],
													vol_center, surfaceData[triangleCount].triVertices[1],
													surfaceData[triangleCount].triVertices[2], crossPoint);
									if (oneFaceCross)
									{
										sliceViewData[localMemIndex].crossPoints[dir][sliceCrossPointCount[dir] * 2] = crossPoint[0];
										sliceViewData[localMemIndex].crossPoints[dir][sliceCrossPointCount[dir] * 2 + 1] = crossPoint[1];
										++ sliceCrossPointCount[dir];
									}
								}
								++ triangleCount;
							} // end if (crossedEdgeCount >= 3) , else
							
						} // end for (int hiLoFace
					} // end for (int face
#endif
				} // end for vofit
			} // end for dit
		} // end for ilev
	} // end 	for ivol

	// find membrane neighbors for points having corner neighbors.
#if CH_SPACEDIM == 2
	pout() << methodName << " release edgeVertices" << endl;
	delete[] edgeVertices;
	writeMeshHdf5(metricsData, vertexCount, vertexList, segmentList);
	pout() << methodName << " release segmentList" << endl;
	delete[] segmentList;
#else
	int numTriangles = triangleCount;
	for (int tri = 0; tri < numTriangles; ++ tri)
	{
//		pout() << "tri=" << tri << endl;
		int globalMemIndex = membraneTriangles[tri].memIndex;
		int localMemIndex = globalMemIndex - memIndexOffset;
		int centroidIndex = tempVertIndexesArray[localMemIndex][memCentroidOffset];
		if (centroidIndex < 0)
		{
			stringstream ss;
			ss << " for membrane element " << globalMemIndex << ", no vertex was stored for its centroid.";
			throw ss.str();
		}
		int triface = membraneTriangles[tri].face;
		int triverts[4];
		int vcount = 0;
		for (int j = 0; j < 4; ++ j)
		{
			int edgeNumber = edgeIndex[triface][j];
			int foundVert = tempVertIndexesArray[localMemIndex][edgeNumber];
			if (foundVert != MEMBRANE_INDEX_INVALID)
			{
				if (vcount < 2)
				{
					triverts[vcount] = foundVert;  // set vertex index
				}
				vcount++;
			}
		}

		if (vcount == 2)
		{
			// 1. order vertices consistently counter-clockwise
			RealVect pts[3] = {vertexList[triverts[0]].coords, vertexList[triverts[1]].coords, vertexList[centroidIndex].coords};
			RealVect outNormal = metricsData[localMemIndex].normal;
			IntVect newOrder = orientVertices(pts, outNormal);
			membraneTriangles[tri].oriented = true;
			// 2. assign vertex indeces
			for (int i = 0; i < 3; ++ i) {
				if (newOrder[i] == 2) {
					membraneTriangles[tri].triVertices[i] = centroidIndex;
				} else {
					membraneTriangles[tri].triVertices[i] = triverts[newOrder[i]];
				}
			}
		}
		else
		{
			// vcount != 2
			// cases with corners or cases with more than 2 cross points
			// construct a dummy triangle
			for (int i = 0; i < SpaceDim; ++ i) {
				membraneTriangles[tri].triVertices[i] = centroidIndex;
			}
		}
	}// end tri

	delete[] tempVertIndexesArray;

	++ triangleVertexCount;  // triangleVertexCount was the index of the last vertex
	writeMeshHdf5(metricsData, triangleVertexCount, vertexList, triangleCount, surfaceData, sliceViewData);
	delete[] surfaceData;
	delete[] sliceViewData;
#endif

	pout() << methodName << " release metricsData" << endl;
	delete[] metricsData;
	pout() << methodName << " release vertexList" << endl;
	delete[] vertexList;

	pout() << "Exit " << methodName << endl;
}

#if CH_SPACEDIM == 2
void ChomboScheduler::writeMeshHdf5(MembraneElementMetrics* metricsData, int vertexCount, Vertex* vertexList, Segment* segmentList)
#else
void ChomboScheduler::writeMeshHdf5(MembraneElementMetrics* metricsData, int vertexCount, Vertex* vertexList, int triangleCount, Triangle* surfaceData, SliceView* sliceViewData)
#endif
{
	const char* methodName = "(ChomboScheduler::writeMeshHdf5)";
	pout() << "Entry " << methodName << endl;
	
	hid_t file_access = H5P_DEFAULT;
#ifdef CH_MPI
	file_access = H5Pcreate(H5P_FILE_ACCESS);
	H5Pset_fapl_mpio(file_access,  MPI_COMM_WORLD, MPI_INFO_NULL);
#endif

	char fileName[FILE_NAME_BUFSIZE];
	sprintf(fileName, "%s%s", SimTool::getInstance()->getBaseFileName(), MESH_HDF5_FILE_EXT);
	pout() << "creating file " << fileName << endl;
	hid_t h5MeshFile = H5Fcreate(fileName, H5F_ACC_TRUNC, H5P_DEFAULT, file_access);

	pout() << "creating group " << MESH_GROUP << endl;
	hid_t meshGroup = H5Gcreate(h5MeshFile, MESH_GROUP, H5P_DEFAULT);

	pout() << "writing attributes " << endl;
	// attributes
	{
	// dimension
	hid_t scalarDataSpace = H5Screate(H5S_SCALAR); // shared among all attributes

	hid_t attribute = H5Acreate(meshGroup, MESH_ATTR_DIMENSION, H5T_NATIVE_INT, scalarDataSpace, H5P_DEFAULT);
	int dimension = chomboGeometry->getDimension();
	pout() << "writing attribute " << MESH_ATTR_DIMENSION << "=" << dimension << endl;
	H5Awrite(attribute, H5T_NATIVE_INT, &dimension);
	H5Aclose(attribute);

	// origin
	pout() << "writing attribute " << MESH_ATTR_ORIGIN << "=" << chomboGeometry->getDomainOrigin() << endl;
	hid_t realVectType = createRealVectDataType();
	attribute = H5Acreate(meshGroup, MESH_ATTR_ORIGIN, realVectType, scalarDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, realVectType, chomboGeometry->getDomainOrigin().dataPtr());
	H5Aclose(attribute);

	// extent
	pout() << "writing attribute " << MESH_ATTR_EXTENT << "=" << chomboGeometry->getDomainSize() << endl;
	attribute = H5Acreate(meshGroup, MESH_ATTR_EXTENT, realVectType, scalarDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, realVectType, chomboGeometry->getDomainSize().dataPtr());
	H5Aclose(attribute);

	// mesh size
	int viewLevel = chomboSpec->getViewLevel();
	pout() << "writing attribute " << MESH_ATTR_NX << "=" << vectNxes[viewLevel] << endl;
	hid_t intVectType = createIntVectDataType();
	attribute = H5Acreate(meshGroup, MESH_ATTR_NX, intVectType, scalarDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, intVectType, vectNxes[viewLevel].dataPtr());
	H5Aclose(attribute);

	// grid size
	pout() << "writing attribute " << MESH_ATTR_DX << "=" << vectDxes[viewLevel] << endl;
	attribute = H5Acreate(meshGroup, MESH_ATTR_DX, realVectType, scalarDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, realVectType, vectDxes[viewLevel].dataPtr());
	H5Aclose(attribute);

	// number of levels
	pout() << "writing attribute " << MESH_ATTR_NUM_LEVELS << "=" << numLevels << endl;
	attribute = H5Acreate(meshGroup, MESH_ATTR_NUM_LEVELS, H5T_NATIVE_INT, scalarDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, H5T_NATIVE_INT, &numLevels);
	H5Aclose(attribute);

	// view level
	pout() << "writing attribute " << MESH_ATTR_VIEW_LEVEL << "=" << viewLevel << endl;
	attribute = H5Acreate(meshGroup, MESH_ATTR_VIEW_LEVEL, H5T_NATIVE_INT, scalarDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, H5T_NATIVE_INT, &viewLevel);
	H5Aclose(attribute);
	H5Sclose(scalarDataSpace);

	// refine ratios
	pout() << "writing attribute " << MESH_ATTR_REFINE_RATIOS << "=" << vectRefRatios << endl;
	int* ratios = new int[vectRefRatios.size()];
	for (int i = 0; i < vectRefRatios.size(); ++ i)
	{
		ratios[i] = vectRefRatios[i];
	}
	int rank = 1;
	hsize_t dim[1] = {vectRefRatios.size()};
	hid_t intArrayDataSpace = H5Screate_simple(rank, dim, NULL); // shared among all attributes
	attribute = H5Acreate(meshGroup, MESH_ATTR_REFINE_RATIOS, H5T_NATIVE_INT, intArrayDataSpace, H5P_DEFAULT);
	H5Awrite(attribute, H5T_NATIVE_INT, ratios);
	H5Aclose(attribute);
	H5Sclose(intArrayDataSpace);
	delete[] ratios;
	}

	// boxes
	{
	pout() << "writing group " << BOXES_GROUP << endl;
	hid_t boxesGroup = H5Gcreate(h5MeshFile, BOXES_GROUP, H5P_DEFAULT);
	hid_t boxType = MeshBox::createH5Type();
	int rank = 1;
	
	for (int ilev = 0; ilev < numLevels; ++ ilev)
	{
		Vector<Box> vectBoxes = vectGrids[ilev].boxArray();
		int numBoxes = vectBoxes.size();
		hsize_t dim[] = {numBoxes};   /* Dataspace dimensions */
		hid_t fileSpace = H5Screate_simple (rank, dim, NULL);
		char levelBoxDataSetName[20];
		sprintf(levelBoxDataSetName, "%s%d", BOXES_LEVEL_DATASET_PREFIX, ilev);
		hid_t levelBoxDataSet = H5Dcreate (h5MeshFile, levelBoxDataSetName, boxType, fileSpace, H5P_DEFAULT);
		// let root write
		if (SimTool::getInstance()->isRootRank())
		{
			MeshBox* levelBoxData = new MeshBox[numBoxes];
			for (int i = 0; i < numBoxes; ++ i)
			{
				levelBoxData[i].lo = vectBoxes[i].smallEnd();
				levelBoxData[i].hi = vectBoxes[i].bigEnd();
			}
			pout() << "writing dataset " << levelBoxDataSetName << endl;
			H5Dwrite(levelBoxDataSet, boxType, H5S_ALL, H5S_ALL, H5P_DEFAULT, levelBoxData);
			delete[] levelBoxData;
		}
		H5Dclose(levelBoxDataSet);
		H5Sclose(fileSpace);
	}
	H5Tclose(boxType);
	H5Gclose(boxesGroup);
	}

	// structures
	{
	pout() << "creating dataset " << STRUCTURES_DATASET << endl;
	hid_t sType = StructureMetrics::createH5Type();
	VCellModel* model = SimTool::getInstance()->getModel();
	int numStructures = model->getNumFeatures() + model->getNumMembranes();
	
	hsize_t dim[] = {numStructures};   /* Dataspace dimensions */
	int rank = 1;  // number of dimensions
	hid_t fileSpace = H5Screate_simple (rank, dim, NULL);
	hid_t structureDataset = H5Dcreate(h5MeshFile, STRUCTURES_DATASET, sType, fileSpace, H5P_DEFAULT);
  // let root write
	if (SimTool::getInstance()->isRootRank())
	{
		pout() << "writing dataset " << STRUCTURES_DATASET << endl;
		StructureMetrics* structureData = new StructureMetrics[numStructures];
		int cnt = 0;
		for (int i = 0; i < model->getNumFeatures(); ++ i)
		{
			Feature* f = model->getFeatureFromIndex(i);
			strcpy(structureData[cnt].name, f->getName().c_str());
			strcpy(structureData[cnt].type, "feature");
			structureData[cnt].size = f->getSize();
			structureData[cnt].numPoints = f->getNumPoints();
			++ cnt;
		}
		for (int i = 0; i < model->getNumMembranes(); ++ i)
		{
			Membrane* m = model->getMembraneFromIndex(i);
			strcpy(structureData[cnt].name, m->getName().c_str());
			strcpy(structureData[cnt].type, "membrane");
			structureData[cnt].size = m->getSize();
			structureData[cnt].numPoints = m->getNumPoints();
			++ cnt;
		}
		H5Dwrite(structureDataset, sType, H5S_ALL, H5S_ALL, H5P_DEFAULT, structureData);
		delete[] structureData;
	}
	H5Dclose(structureDataset);
	H5Sclose(fileSpace);
	H5Tclose(sType);
	}

	// featurephasevols
	{
	pout() << "creating dataset " << FETUREPHASEVOLS_DATASET << endl;
	hid_t sType = FeaturePhaseVol::createH5Type();

	int ts = phaseVolumeList[phase0].size() + phaseVolumeList[phase1].size();
	hsize_t dim[] = {ts};   /* Dataspace dimensions */
	int rank = 1;  // number of dimensions
	hid_t fileSpace = H5Screate_simple (rank, dim, NULL);
	hid_t ds = H5Dcreate(h5MeshFile, FETUREPHASEVOLS_DATASET, sType, fileSpace, H5P_DEFAULT);
  // let root write
	if (SimTool::getInstance()->isRootRank())
	{
		pout() << "writing dataset " << FETUREPHASEVOLS_DATASET << endl;
		FeaturePhaseVol* data = new FeaturePhaseVol[ts];
		int cnt = 0;
		for (int iphase = 0; iphase < NUM_PHASES; iphase ++)
		{
			for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++)
			{
				Feature* f = phaseVolumeList[iphase][ivol]->feature;
				strcpy(data[cnt].feature, f->getName().c_str());
				data[cnt].iphase = iphase;
				data[cnt].ivol = ivol;
				++ cnt;
			}
		}
		H5Dwrite(ds, sType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
		delete[] data;
	}
	H5Dclose(ds);
	H5Sclose(fileSpace);
	H5Tclose(sType);
	}
	
	// membraneids
	{
	pout() << "creating dataset " << MEMBRANEIDS_DATASET << endl;
	hid_t sType = MembraneId::createH5Type();

	int ts = 0;
	for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ++ ivol)
	{
		ts += phaseVolumeList[phase0][ivol]->adjacentVolumes.size();
	}
	hsize_t dim[] = {ts};   /* Dataspace dimensions */
	int rank = 1;  // number of dimensions
	hid_t fileSpace = H5Screate_simple (rank, dim, NULL);
	hid_t ds = H5Dcreate(h5MeshFile, MEMBRANEIDS_DATASET, sType, fileSpace, H5P_DEFAULT);
  // let root write
	if (SimTool::getInstance()->isRootRank())
	{
		pout() << "writing dataset " << MEMBRANEIDS_DATASET << endl;
		MembraneId* data = new MembraneId[ts];
		int cnt = 0;
		for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol ++)
		{
			Feature* iFeature = phaseVolumeList[phase0][ivol]->feature;
			Vector<ConnectedComponent*>& adjacentVolumes = phaseVolumeList[phase0][ivol]->adjacentVolumes;
			for (int j = 0; j < adjacentVolumes.size(); j ++) {
				int jphase = adjacentVolumes[j]->phase;
				int jvol = adjacentVolumes[j]->volumeIndexInPhase;
				data[cnt].id = ivol * numConnectedComponents + jvol;
				Feature* jFeature = adjacentVolumes[j]->feature;
				Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(iFeature, jFeature);
				strcpy(data[cnt].membrane, membrane->getName().c_str());
				++ cnt;
			}
		}
		H5Dwrite(ds, sType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
		delete[] data;
	}
	H5Dclose(ds);
	H5Sclose(fileSpace);
	H5Tclose(sType);
	}

	// membrane metrics
	{
	pout() << "writing dataset " << MEMBRANE_ELEMENTS_DATASET << ", count=" << numMembranePoints << endl;
	hid_t metricsType = MembraneElementMetrics::createH5Type();
	int rank = 1;
	// memory dataspace dimensions
	hsize_t dim[] = {numMembranePoints};
	hid_t memSpace = H5Screate_simple(rank, dim, NULL);

#ifdef CH_MPI
	// file dataspace dimensions
	dim[0] = totalNumMembranePoints;
	hid_t fileSpace = H5Screate_simple(rank, dim, NULL);
	// select offset in file space
	hsize_t start[] = {memIndexOffset};
	hsize_t count[] = {numMembranePoints};
	herr_t err = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, NULL, count, NULL);
	if (err < 0)
	{
		stringstream ss;
		ss << "failed to select position to write " << MEMBRANE_ELEMENTS_DATASET;
		throw ss.str();
	}
#else
	hid_t fileSpace = memSpace;
	memSpace = H5S_ALL; // same as file space in serial
#endif
	hid_t metricsDataset = H5Dcreate(h5MeshFile, MEMBRANE_ELEMENTS_DATASET, metricsType, fileSpace, H5P_DEFAULT);
	H5Dwrite(metricsDataset, metricsType, memSpace, fileSpace, H5P_DEFAULT, metricsData);
	H5Dclose(metricsDataset);
#ifdef CH_MPI
	H5Sclose(memSpace);
#endif
	H5Sclose(fileSpace);
	H5Tclose(metricsType);
	}

	// vertices
#ifdef CH_MPI
	int vertexIndexOffset = 0; // this will be used below by segments which has vertex indices
#endif
	
	{
	pout() << "writing dataset " << VERTICES_DATASET << ", count=" << vertexCount << endl;
	hid_t vertexType = Vertex::createH5Type();
	int rank = 1;
	// memory space
	hsize_t dim[] = {vertexCount};
	hid_t memSpace = H5Screate_simple(rank, dim, NULL);
#ifdef CH_MPI
	// exchange vertex offset and total
	int numProcs = SimTool::getInstance()->getCommSize();
	int* sendBuffer = new int[numProcs + 1];  // only send my vertextCount, receive all offsets and total
	sendBuffer[0] =  vertexCount;
	int receiveSize = numProcs;
	int *recvBuffer = NULL;
	if (SimTool::getInstance()->isRootRank())
	{
		 recvBuffer = new int[receiveSize];
	}

	int totalNumVertices = vertexCount;

	// each processor puts its sendBuffer in recvBuffer in the order or rank.
	// so in recvBuffer, the data looks like
	// ---sendBuffer of processor 1---sendBuffer of processor 2---
	pout() << "gathering # vertices from each processor" << endl;
	MPI_Gather(sendBuffer, 1, MPI_INT, recvBuffer, 1, MPI_INT, SimTool::rootRank, MPI_COMM_WORLD);
	if (SimTool::getInstance()->isRootRank())
	{
		pout() << "root rank collect vertextIndexOffset" << endl;
		totalNumVertices = 0;
		for (int iproc = 0; iproc < numProcs; ++ iproc)
		{
			int n = recvBuffer[iproc];
			sendBuffer[iproc] = totalNumVertices;
			totalNumVertices += n;
		}
		sendBuffer[numProcs] = totalNumVertices;
		pout() << "totalNumVertices=" << totalNumVertices << endl;
	}

	pout() << "broadcast vertexIndexOffsets and total " << endl;
	// broadcast
	MPI_Bcast(sendBuffer, numProcs + 1, MPI_INT, SimTool::rootRank, MPI_COMM_WORLD);

	pout() << "** find vertexIndexOffset" << endl;
	vertexIndexOffset = sendBuffer[SimTool::getInstance()->getMyRank()];
	totalNumVertices = sendBuffer[numProcs];
	pout() << "** my vertexIndexOffset is = " << vertexIndexOffset << ", totalNumVertices=" << totalNumVertices << endl;

	delete[] sendBuffer;
	delete[] recvBuffer;

	// file dataspace dimensions
	dim[0] = totalNumVertices;
	hid_t fileSpace = H5Screate_simple(rank, dim, NULL);
	// select offset in file space
	hsize_t start[] = {vertexIndexOffset};
	hsize_t count[] = {vertexCount};
	herr_t err = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, NULL, count, NULL);
	if (err < 0)
	{
		stringstream ss;
		ss << "failed to select position to write " << VERTICES_DATASET;
		throw ss.str();
	}
#else
	hid_t fileSpace = memSpace;
	memSpace = H5S_ALL; // same as file space in serial
#endif
	hid_t verticesDataset = H5Dcreate (h5MeshFile, VERTICES_DATASET, vertexType, fileSpace, H5P_DEFAULT);
	H5Dwrite(verticesDataset, vertexType, memSpace, fileSpace, H5P_DEFAULT, vertexList);
	H5Dclose(verticesDataset);
#ifdef CH_MPI
	H5Sclose(memSpace);
#endif
	H5Sclose(fileSpace);
	H5Tclose(vertexType);
	}

#if CH_SPACEDIM == 2
	// segments
	{
	pout() << "writing dataset " << SEGMENTS_DATASET << ", count=" << numMembranePoints << endl;
	hid_t segmentType = Segment::createH5Type();
	int rank = 1;
	// memory dataspace dimensions
	hsize_t dim[] = {numMembranePoints};
	hid_t memSpace = H5Screate_simple(rank, dim, NULL);

#ifdef CH_MPI
	// adjust vertexIndex in segments
	for (int i = 0; i < numMembranePoints; ++ i)
	{
		segmentList[i].vertexIndexes[0] += vertexIndexOffset;
		segmentList[i].vertexIndexes[1] += vertexIndexOffset;
	}
	// file dataspace dimensions
	dim[0] = totalNumMembranePoints;
	hid_t fileSpace = H5Screate_simple(rank, dim, NULL);
	// select offset in file space
	hsize_t start[] = {memIndexOffset};
	hsize_t count[] = {numMembranePoints};
	herr_t err = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, NULL, count, NULL);
	if (err < 0)
	{
		stringstream ss;
		ss << "failed to select position to write " << SEGMENTS_DATASET;
		throw ss.str();
	}
#else
	hid_t fileSpace = memSpace;
	memSpace = H5S_ALL;  // same as file space in serial
#endif
	
	hid_t segmentsDataset = H5Dcreate (h5MeshFile, SEGMENTS_DATASET, segmentType, fileSpace, H5P_DEFAULT);
	H5Dwrite(segmentsDataset, segmentType, memSpace, fileSpace, H5P_DEFAULT, segmentList);
	H5Dclose(segmentsDataset);
#ifdef CH_MPI
	H5Sclose(memSpace);
#endif
	H5Sclose(fileSpace);
	H5Tclose(segmentType);
	}

#else
	// cross points
	{
	// vertices
	pout() << "writing dataset " << SURFACE_DATASET << endl;
	hid_t triangleType = Triangle::createH5Type();
	hsize_t dim[] = {triangleCount};   /* Dataspace dimensions */
	int rank = 1;
	hid_t memSpace = H5Screate_simple(rank, dim, NULL);
#ifdef CH_MPI
	int triangleOffset = 0;
	// exchange vertex offset and total
	int numProcs = SimTool::getInstance()->getCommSize();
	int* sendBuffer = new int[numProcs + 1];  // only send my vertextCount, receive all offsets and total
	sendBuffer[0] =  triangleCount;
	int receiveSize = numProcs;
	int *recvBuffer = NULL;
	if (SimTool::getInstance()->isRootRank())
	{
		 recvBuffer = new int[receiveSize];
	}

	int totalNumTriangles = triangleCount;

	// each processor puts its sendBuffer in recvBuffer in the order or rank.
	// so in recvBuffer, the data looks like
	// ---sendBuffer of processor 1---sendBuffer of processor 2---
	pout() << "gathering # triangles from each processor" << endl;
	MPI_Gather(sendBuffer, 1, MPI_INT, recvBuffer, 1, MPI_INT, SimTool::rootRank, MPI_COMM_WORLD);
	if (SimTool::getInstance()->isRootRank())
	{
		pout() << "root rank collect vertextIndexOffset" << endl;
		totalNumTriangles = 0;
		for (int iproc = 0; iproc < numProcs; ++ iproc)
		{
			int n = recvBuffer[iproc];
			sendBuffer[iproc] = totalNumTriangles;
			totalNumTriangles += n;
		}
		sendBuffer[numProcs] = totalNumTriangles;
		pout() << "totalNumTriangles=" << totalNumTriangles << endl;
	}

	pout() << "broadcast triangleOffsets and total " << endl;
	// broadcast
	MPI_Bcast(sendBuffer, numProcs + 1, MPI_INT, SimTool::rootRank, MPI_COMM_WORLD);

	pout() << "** find triangleOffset" << endl;
	triangleOffset = sendBuffer[SimTool::getInstance()->getMyRank()];
	totalNumTriangles = sendBuffer[numProcs];
	pout() << "** my triangleOffset is = " << triangleOffset << ", totalNumTriangles=" << totalNumTriangles << endl;

	delete[] sendBuffer;
	delete[] recvBuffer;

	// file dataspace dimensions
	dim[0] = totalNumTriangles;
	hid_t fileSpace = H5Screate_simple(rank, dim, NULL);
	// select offset in file space
	hsize_t start[] = {triangleOffset};
	hsize_t count[] = {triangleCount};
	herr_t err = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, NULL, count, NULL);
	if (err < 0)
	{
		stringstream ss;
		ss << "failed to select position to write " << VERTICES_DATASET;
		throw ss.str();
	}
#else
	hid_t fileSpace = memSpace;
	memSpace = H5S_ALL; // same as file space in serial
#endif
	hid_t surfaceDataset = H5Dcreate (h5MeshFile, SURFACE_DATASET, triangleType, fileSpace, H5P_DEFAULT);
	H5Dwrite(surfaceDataset, triangleType, memSpace, fileSpace, H5P_DEFAULT, surfaceData);
	H5Dclose(surfaceDataset);
#ifdef CH_MPI
	H5Sclose(memSpace);
#endif
	H5Sclose(fileSpace);
	H5Tclose(triangleType);
	}

	// slice view
	{
	pout() << "writing dataset " << SLICE_VIEW_DATASET << endl;
	hid_t sliceViewType = SliceView::createH5Type();
	hsize_t dim[] = {numMembranePoints};   /* Dataspace dimensions */
	int rank = 1;
	hid_t memSpace = H5Screate_simple(rank, dim, NULL);
	
#ifdef CH_MPI
	// file dataspace dimensions
	dim[0] = totalNumMembranePoints;
	hid_t fileSpace = H5Screate_simple(rank, dim, NULL);
	// select offset in file space
	hsize_t start[] = {memIndexOffset};
	hsize_t count[] = {numMembranePoints};
	herr_t err = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, NULL, count, NULL);
	if (err < 0)
	{
		stringstream ss;
		ss << "failed to select position to write " << SLICE_VIEW_DATASET;
		throw ss.str();
	}
#else
	hid_t fileSpace = memSpace;
	memSpace = H5S_ALL;  // same as file space in serial
#endif

	hid_t sliceViewDataset = H5Dcreate(h5MeshFile, SLICE_VIEW_DATASET, sliceViewType, fileSpace, H5P_DEFAULT);
	H5Dwrite(sliceViewDataset, sliceViewType, memSpace, fileSpace, H5P_DEFAULT, sliceViewData);
	H5Dclose(sliceViewDataset);
#ifdef CH_MPI
	H5Sclose(memSpace);
#endif
	H5Sclose(fileSpace);
	H5Tclose(sliceViewType);
	}
#endif

	pout() << "closing group " << MESH_GROUP << " and file." << endl;
	H5Gclose(meshGroup);
	H5Fclose(h5MeshFile);
	
#ifdef CH_MPI
	H5Pclose(file_access);
#endif
	
	pout() << "Exit " << methodName << endl;
}

bool ChomboScheduler::computeOneFaceCross(int dir, int face, int hiLoFace, RealVect& H, RealVect& v0,
				RealVect& v1, RealVect& v2, RealVect& crossPoint)
{
	if (dir == face)
	{
		return false;
	}
	static int slicedirs[3][2] = {{1, 2}, {0, 2}, {0, 1}};

	int jlohi, jcross;
	int planedir0, planedir1;
	planedir0 = dir;
	if (face == slicedirs[dir][0])    // y- or y+
	{
		planedir1 = slicedirs[dir][1];   // not y direction,
		jlohi = 0;
		jcross = 1;
	}
	else                         // z- or z+
	{
		planedir1 = slicedirs[dir][0];   // not z direction,
		jlohi = 1;
		jcross = 0;
	}

	RealVect DP = v2 - v1;
	Real thisDP0 = DP[planedir0];
	Real thisDP1 = DP[planedir1];

	if (abs(thisDP0) > 1.e-8 * abs(thisDP1))
	{
		// if slope too large, return a number out of range
		Real xc = v0[planedir0];
		Real yc = v0[planedir1];
		Real p10 = v1[planedir0];
		Real p11 = v1[planedir1];
		Real a = thisDP1 / thisDP0;
		Real value = a * (xc - p10) + p11;
		if (abs(value - yc) <= H[planedir1]/2)
		{
			crossPoint[jlohi]  = v0[face] + hiLoFace * H[face]/2 ;
			crossPoint[jcross] = value;
			return true;
		}
	}
	return false;
}

int ChomboScheduler::findLevel(const ProblemDomain& domain)
{
	if (numLevels == 1)
	{
		return 0;
	}
	for (int i = 0; i < numLevels; ++ i)
	{
		if (vectDomains[i].domainBox() == domain.domainBox())
		{
			return i;
		}
	}
	return -1;
}

void ChomboScheduler::populateExtrapolatedValues()
{
	const char* methodName = "(ChomboScheduler::populateExtrapolatedValues)";
	pout() << "Entry " << methodName << endl;

	for (int iphase = 0; iphase < NUM_PHASES; iphase ++)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ivol ++)
		{
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;
			int numDefinedVolVars = feature->getNumDefinedVariables();
			if (numDefinedVolVars == 0)
			{
				pout() << methodName << "no variables defined in feature " << feature->getName() << endl;
				continue;
			}
			for (int ilev = 0; ilev < numLevels; ++ ilev)
			{
				DisjointBoxLayout& currGrids = vectGrids[ilev];
				for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)
				{
					const EBISBox& currEBISBox = vectEbis[iphase][ivol][ilev][dit()];
					const Box& currBox = vectGrids[ilev][dit()];

					const EBGraph& currEBGraph = currEBISBox.getEBGraph();
					IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);

					for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit)
					{
						const VolIndex& vof = vofit();
						int globalMemIndex = (*irregularPointMembraneIndex[iphase][ivol][ilev])[dit()](vof, 0);
						if (globalMemIndex == MEMBRANE_INDEX_IN_FINER_LEVEL)
						{
							continue;
						}
						int localMemIndex = globalMemIndex - memIndexOffset;

						for (int iDefinedVar = 0; iDefinedVar < numDefinedVolVars; iDefinedVar ++)
						{
							VolumeVariable* var = (VolumeVariable*)feature->getDefinedVariable(iDefinedVar);
							Real extrapVal = (*extrapValues[iphase][ivol][ilev])[dit()](vof, iDefinedVar);
							var->getExtrapolated()[localMemIndex] = extrapVal;
						}
					}
				} // end dit()
			} // end ilev
		} // end ivol
	} // end iphase

	pout() << "Exit " << methodName << endl;
}

void ChomboScheduler::populateMembraneSolution()
{
	const char* methodName = "(ChomboScheduler::populateMembraneSolution)";
	pout() << "Entry " << methodName << endl;

	// membrane variables
	int numMembraneVars = simulation->getNumMemVariables();
	
	for (int ivol = 0; ivol < phaseVolumeList[phase0].size(); ivol ++) {
		Feature* iFeature = phaseVolumeList[phase0][ivol]->feature;

		for (int ilev = 0; ilev < numLevels; ++ ilev)
		{
			// unit surface area in this level
			Real maxDxComponent = vectDxes[ilev][0];
			for(int idir = 1; idir < SpaceDim; idir ++) {
				maxDxComponent = max(maxDxComponent, vectDxes[ilev][idir]);
			}
			double levelUnitS = pow(maxDxComponent,SpaceDim -1);

			DisjointBoxLayout& currGrids = vectGrids[ilev];
			for(DataIterator dit = currGrids.dataIterator(); dit.ok(); ++dit)	{
				const EBISBox& currEBISBox = vectEbis[phase0][ivol][ilev][dit()];
				const Box& currBox = vectGrids[ilev][dit()];

				const EBGraph& currEBGraph = currEBISBox.getEBGraph();
				IntVectSet irregCells = currEBISBox.getIrregIVS(currBox);

				for (VoFIterator vofit(irregCells,currEBGraph); vofit.ok(); ++vofit) {
					const VolIndex& vof = vofit();
					int globalMemIndex = (*irregularPointMembraneIndex[phase0][ivol][ilev])[dit()](vof, 0);
					if (globalMemIndex == MEMBRANE_INDEX_IN_FINER_LEVEL)
					{
						continue;
					}
					int localMemIndex = globalMemIndex - memIndexOffset;

					int membraneID = (*irregularPointMembraneIDs[phase0][ivol][ilev])[dit()](vof, 0);
					if (membraneID < 0)
					{
						continue;
					}
					int jvol = membraneID % numConnectedComponents;

					double areaFrac = currEBISBox.bndryArea(vof);
					Feature* jFeature = phaseVolumeList[phase1][jvol]->feature;
					Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(iFeature, jFeature);
					for (int memVarIdx = 0; memVarIdx < numMembraneVars; ++ memVarIdx)
					{
						Variable* var = (Variable*)simulation->getMemVariable(memVarIdx);
						for (int ivar = 0; ivar < iFeature->getMemVarIndexesInAdjacentMembranes().size(); ++ ivar)
						{
							int varIndex =	iFeature->getMemVarIndexesInAdjacentMembranes()[ivar];
							if (varIndex == memVarIdx)
							{
								if (membrane->isVariableDefined(var))
								{
									double* varCurr = var->getCurr();
									double sol = (*memSoln[ivol][ilev])[dit()](vof, ivar);
									varCurr[localMemIndex] = sol;
#ifndef CH_MPI
									var->min = std::min<double>(var->min, sol);
									var->max = std::max<double>(var->max, sol);

									var->addTotal(sol * areaFrac * levelUnitS);

									Variable* errorVar = var->getExactErrorVariable();
									if (errorVar != NULL)
									{
										RealVect vol_center = EBArith::getVofLocation(vof, vectDxes[ilev], chomboGeometry->getDomainOrigin());
										const RealVect& mem_centroid = currEBISBox.bndryCentroid(vof);
										RealVect coord = mem_centroid;
										coord *= vectDxes[ilev];
										coord += vol_center;
										memset(vectValues, 0, numSymbols * sizeof(double));
										vectValues[0] = simulation->getTime_sec();
										vectValues[1] = coord[0];
										vectValues[2] = coord[1];
										vectValues[3] = SpaceDim < 3 ? 0.5 : coord[2];
										double exact = var->getVarContext()->evaluateExpression(EXACT_EXP, vectValues);
										double* errorCurr = errorVar->getCurr();
										double error = sol - exact;
										errorCurr[localMemIndex] = error;
										Variable* relErrVar = var->getRelativeErrorVariable();
										double* relErrCurr = relErrVar->getCurr();
										relErrCurr[localMemIndex] = exact = 0 ? 0 : std::abs(error/exact);
										var->updateMaxError(abs(error));

										double l2 = error * error * areaFrac;
										double l2exact = exact * exact * areaFrac;
										var->addL2Error(l2);
										var->addL2Exact(l2exact);
									}
#endif
								}
								break;
							}
						}
					}
				} // for (VoFIterator vofit(irregCells,currEBGraph);
			} // end DataIter
		} // end ilev
	} // end ivol
	
	pout() << "Exit " << methodName << endl;
}

#ifndef CH_MPI
void ChomboScheduler::updateSolutionFromChomboOutputFile()
{
	static const char* METHOD = "(ChomboScheduler::updateSolutionFromChomboOutputFile)";
	pout() << "Entry " << METHOD << endl;

	pout() << endl << "time = " << simulation->getTime_sec() << endl;

	for (int i = 0; i < simulation->getNumVariables(); ++ i)
	{
		Variable* var = simulation->getVariable(i);
		var->reset(chomboSpec->isSaveVCellOutput());
	}
	
	int firstFilePhase = -1, firstFileVol = -1;
	
	volSoln.resize(NUM_PHASES);
	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		int numVols = phaseVolumeList[iphase].size();
		volSoln[iphase].resize(numVols);

		for (int ivol = 0; ivol < numVols; ++ ivol)
		{
			Feature* feature = phaseVolumeList[iphase][ivol]->feature;
			int ncomp = feature->getNumDefinedVariables();
			if (ncomp == 0)
			{
				pout() << METHOD << "no variables defined in feature " << feature->getName() << endl;
				continue;
			}

			if (firstFilePhase == -1)
			{
				firstFilePhase = iphase;
				firstFileVol = ivol;
			}
			
			char hdf5FileName[FILE_NAME_BUFSIZE];
			sprintf(hdf5FileName, CHOMBO_OUTPUT_FILE_NAME_FORMAT, SimTool::getInstance()->getBaseFileName(), simulation->getCurrIteration(), feature->getName().c_str(), ivol);
			pout() << METHOD << " readEBHDF5, [iphase, ivol]=[" << iphase << "," << ivol << "] from " << hdf5FileName << endl;

			Vector<LevelData<FArrayBox>* > chomboData;
			Box domain;
			int eek = ReadAMRHierarchyHDF5(hdf5FileName,
                     vectGrids,
                     chomboData,
                     domain,
                     vectRefRatios,
                     numLevels);
			if (eek != 0)
      {
        throw "ReadAMRHierarchyHDF5 failed";
      }

			// refill EBIS layout because of new grids
			for (int ilev = 0; ilev < numLevels; ++ ilev)
			{
				phaseVolumeList[iphase][ivol]->volume->fillEBISLayout(vectEbis[iphase][ivol][ilev],
										vectGrids[ilev],
										vectDomains[ilev],
										numGhostEBISLayout);
			}

			volSoln[iphase][ivol].resize(numLevels);

			for (int ilev = 0; ilev < numLevels; ilev ++)
			{
				EBCellFactory ebCellFactory(vectEbis[iphase][ivol][ilev]);
				delete volSoln[iphase][ivol][ilev];
				volSoln[iphase][ivol][ilev] = new LevelData<EBCellFAB>(vectGrids[ilev], ncomp, numGhostSoln, ebCellFactory);

				LevelData<EBCellFAB>& ebcfSoln = *volSoln[iphase][ivol][ilev];
				LevelData<FArrayBox>& fabData = *chomboData[ilev];

				for (DataIterator dit = vectGrids[ilev].dataIterator(); dit.ok(); ++ dit)
				{
					FArrayBox& currentFab = fabData[dit()];
					// copy single valued data
					ebcfSoln[dit()].getSingleValuedFAB().copy(currentFab);

					// copy the multicell data
					{
						EBCellFAB& ebcf = ebcfSoln[dit()];
						EBISBox ebisBox1 = ebcf.getEBISBox();
            IntVectSet ivsMulti = ebisBox1.getMultiCells(currentFab.box());
            for (VoFIterator vofit(ivsMulti, ebisBox1.getEBGraph()); vofit.ok(); ++vofit)
            {
							if (vofit().cellIndex() == 0)
							{
								IntVect iv = vofit().gridIndex();
								for (int ivar = 0; ivar < ncomp; ++ivar)
								{
									ebcf(vofit(), ivar) = currentFab(iv, ivar);
								}
							}
						}
          }
				}
			}
			computeTotal(iphase, ivol);
			populateVolumeSolution(iphase, ivol);
		} // ivol
	} // iphase

	populateImplicitFunctions();
	
	// read membrane solution and extrapolated values
	pout() << METHOD << "firstFilePhase=" << firstFilePhase << ", firstFileVol=" << firstFileVol << endl;
	if (firstFilePhase != -1)
	{
		Feature* feature = phaseVolumeList[firstFilePhase][firstFileVol]->feature;
		char hdf5FileName[FILE_NAME_BUFSIZE];
		// write membrane variable solution and extrapolated values to the first hdf5 file
		sprintf(hdf5FileName, CHOMBO_OUTPUT_FILE_NAME_FORMAT, SimTool::getInstance()->getBaseFileName(), simulation->getCurrIteration(), feature->getName().c_str(), firstFileVol);
		hid_t h5SimFile =  H5Fopen(hdf5FileName, H5F_ACC_RDONLY, H5P_DEFAULT);
		DataSet::readMembraneSolution(simulation, h5SimFile);
		DataSet::readExtrapolatedValues(simulation, h5SimFile);
	}

	for (int i = 0; i < simulation->getNumVariables(); ++ i)
	{
		Variable* var = simulation->getVariable(i);
		var->computeFinalStatistics();
	}
	
	// read membrane solution
	pout() << "Exit " << METHOD << endl;
}
#endif

void ChomboScheduler::computeTotal()
{
	static const char* METHOD = "(ChomboScheduler::computeTotal)";
	pout() << "Entry " << METHOD << endl;

	for (int iphase = 0; iphase < NUM_PHASES; ++ iphase)
	{
		for (int ivol = 0; ivol < phaseVolumeList[iphase].size(); ++ ivol)
		{
			computeTotal(iphase, ivol);
		}
	}
	pout() << "Exit " << METHOD << endl;
}

void ChomboScheduler::computeTotal(int iphase, int ivol)
{
	static const char* METHOD = "(ChomboScheduler::computeTotal)";
	pout() << "Entry " << METHOD << "(iphase, ivol)=" << "(" << iphase << "," << ivol << ")" << endl;
	
	Feature* iFeature = phaseVolumeList[iphase][ivol]->feature;
	int numDefinedVolVars = iFeature->getNumDefinedVariables();
	int numDefinedMemVars = iphase == phase0 ? iFeature->getMemVarIndexesInAdjacentMembranes().size() : 0;
	if (numDefinedVolVars == 0 && numDefinedMemVars == 0) {
		return;
	}
	// volume total
	for (int ivar = 0; ivar < numDefinedVolVars; ivar ++)
	{
		Variable* var = iFeature->getDefinedVariable(ivar);
		pout() << METHOD << " computeSum: volume variable " << var->getName() << " in " << iFeature->getName() << endl;
		Real sum = computeSum(volSoln[iphase][ivol], vectEbis[iphase][ivol], ivar);
		var->addTotal(sum);
	}

//			if (iphase == phase0 && numDefinedMemVars > 0)
//			{
//				// membrane variable total
//				Vector<ConnectedComponent*>& adjacentVolumes = phaseVolumeList[iphase][ivol]->adjacentVolumes;
//				for (int j = 0; j < adjacentVolumes.size(); ++ j)
//				{
//					Feature* jFeature = adjacentVolumes[j]->feature;
//					Membrane* membrane = SimTool::getInstance()->getModel()->getMembrane(iFeature, jFeature);
//					for (int ivar = 0; ivar < iFeature->getMemVarIndexesInAdjacentMembranes().size(); ivar ++)
//					{
//						int varIndex =	iFeature->getMemVarIndexesInAdjacentMembranes()[ivar];
//						Variable* var = (Variable*)simulation->getMemVariable(varIndex);
//						if (membrane->isVariableDefined(var))
//						{
//							pout() << METHOD << " computeSum: membrane variable " << var->getName() << " in " << iFeature->getName() << endl;
//							Real sumSoln = computeSum(memSoln[ivol], vectEbis[iphase][ivol], ivar);
//							var->addTotal(sumSoln);
//						}
//					}
//				}
//			}
	pout() << "Exit " << METHOD << endl;
}

Real ChomboScheduler::computeSum(const Vector< LevelData<EBCellFAB>* >& a_src,
              const Vector< EBISLayout >&  a_ebisl,
              const int& a_comp)
{
	static const char* METHOD = "(ChomboScheduler::computeSum)";
	pout() << "Entry " << METHOD << endl;

	Real sum = 0;
	Real volume = 0;
	int a_pval = -2;
  EBNormType::NormMode a_mode = EBNormType::OverBoth;
	
	for (int ilev  = 0; ilev < numLevels; ++ ilev)
	{
		RealVect dxLev = vectDxes[ilev];

		//don't count stuff covered by finer levels
		IntVectSet ivsExclude;
		if (ilev < numLevels - 1)
		{
			//put next finer grids into an IVS
			for (LayoutIterator lit = vectGrids[ilev + 1].layoutIterator(); lit.ok(); ++ lit)
			{
				ivsExclude |= vectGrids[ilev+1].get(lit());
			}
			//coarsen back down to this level.
			//ivs will now hold the image of the next finer level
			ivsExclude.coarsen(vectRefRatios[ilev]);
		}

		Real sumLev, volLev;
		EBArith::volWeightedSum(sumLev, volLev, *(a_src[ilev]),  vectGrids[ilev],
									 a_ebisl[ilev], dxLev, ivsExclude,
									 a_comp, a_pval, a_mode);

		sum += sumLev;
		volume += volLev;
		dxLev /= vectRefRatios[ilev];
	}

	pout() << "Exit " << METHOD << endl;
	return sum;
}
