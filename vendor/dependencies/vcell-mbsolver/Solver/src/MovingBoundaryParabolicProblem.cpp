//#pragma warning ( disable: 4996 )
#pragma warning ( disable: 4244 )
#pragma warning ( disable: 4267 )
#include <JobMessaging.h>
#include <MPoint.h>
#include <boost/logic/tribool.hpp>
#include <World.h>
#include <VoronoiMesh.h>
#include <limits>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <exception>
#include <forward_list>
#include <chrono>
#include <vcellutil.h>
#include <VCellChrono.h>
#include <Expression.h>
#include <SimpleSymbolTable.h>
#include <VCellFront.h>
#include <algo.h>
#include <MovingBoundaryParabolicProblem.h>
#include <MeshElementNode.h>
#include <VoronoiMesh.h>
#include <Logger.h>
#include <Modulo.h>
#include <boundaryProviders.h>
#include <Physiology.h>
#include <CoordVect.h>
#include <IndexVect.h>

#include <ExplicitSolver.h>
#include <MBridge/FronTierAdapt.h>
#include <MBridge/Figure.h>
#include <MBridge/MBPatch.h>
#include <MBridge/MBMovie.h>
#include <MBridge/Scatter.h>
#include <MBridge/MatlabDebug.h>

#include <ExpressionException.h>
#include <MapTable.h>
#include <MTExpression.h>
using spatial::cX;
using spatial::cY;

static const IndexVect naturalNeighborOffsets[2 * DIM] = {
							IndexVect(-1, 0),
							IndexVect(1, 0),
							IndexVect(0, -1),
							IndexVect(0, 1),
					};

//*********************************************************
// Validation of input to problem functors & ValidationBase 
//*********************************************************
namespace {
	//matlabBridge::Scatter scatterInside('r',2);
	//matlabBridge::Scatter scatterOutside('b',2);

	/**
	* abstract class which validates input
	*/
	struct ValidationBase: public spatial::FronTierLevel, public spatial::FronTierVelocity {

		/**
		* number of user input functions 
		*/
		const static int NUMBER_FUNCTIONS = 3;

		ValidationBase(const moving_boundary::MovingBoundarySetup &mbs)
			:nFunctionPointers(0) { 
				std::stringstream badset;

#define CHECK(X) checkSet(badset,X, #X);  
#define CHECK2(X,Y) checkSet(badset,X, Y,#X);  

				CHECK(mbs.maxTime);
				CHECK(mbs.frontTimeStep);
#ifdef OLD_FUNCTION_POINTER_IMPLEMENTATION
				CHECK2(reinterpret_cast<void *>(mbs.concentrationFunction),mbs.concentrationFunctionStr);
				CHECK2(reinterpret_cast<void*>(mbs.velocityFunction),mbs.advectVelocityFunctionStrX)
					if (mbs.alternateFrontProvider == 0) {
						CHECK2(reinterpret_cast<void *>(mbs.levelFunction),mbs.levelFunctionStr);
					}
#endif
#undef CHECK
#undef CHECK2
					switch (nFunctionPointers) {
					case 0:
					case NUMBER_FUNCTIONS:
						break;
					default:
						badset << "functions must be specified by pointers or strings, not both ";
						break;
					}
					validateSet(badset);
		}

		void checkSet(std::stringstream & badset,const double value, const char * const description) {
			if (value == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		void checkSet(std::stringstream & badset,const std::string & value, const char * const description) {
			if (value.size( ) == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		void checkSet(std::stringstream & badset,void * value, const char * const description) {
			if (value == 0) {
				badset << "Unset value " << description << std::endl;  
			}
		}
		/**
		* check something set only one of two ways
		* @param badset 
		* @param value
		* @param str
		* put error message in badset if not exactly one of value, str, is set
		*/
		void checkSet(std::stringstream & badset, void * value, const std::string & str, const char * const description) {
			int argCount = 0;
			if (value != 0) {
				++argCount;
				++nFunctionPointers;
			}
			if (!str.empty( )) { 
				++argCount;
			}
			switch (argCount) {
			case 0:
				badset << "Unset value " << description << std::endl;  
				break;
			case 1:
				break;
			default:
				badset << description << " set more than once" << std::endl;  
			}
		}
		/**
		* check something set only one of two ways
		* @param badset 
		* @param one 
		* @param other 
		* put error message in badset if not exactly one of one, other is et 
		*/
		template<typename T, typename U>
		void checkSet(std::stringstream & badset, T one, U other, const char * const description) {
			if (one == 0 && other == 0) {
				badset << "Unset value " << description << std::endl;  
			}
			if (one != 0 && other != 0) {
				badset << description << " set twice " << std::endl; 
			}
		}

		void validateSet(std::stringstream & badset ) {
			if (badset.str( ).length() > 0) {
				badset << " in MovingBoundarySetup ";
				std::invalid_argument ia(badset.str( ));
				throw ia; 
			}
		}
		int nFunctionPointers;
	};

}

//**************************************************
// main solver, local definition 
//**************************************************
namespace moving_boundary {
	struct MovingBoundaryParabolicProblemImpl : public ValidationBase {
		typedef MovingBoundaryParabolicProblemImpl Outer;
		typedef VoronoiMesh VrnMesh;
		typedef VrnMesh::MBMeshDef MBMeshDef;
		typedef VrnMesh::MBMesh MBMesh;
		//typedef VrnMesh::FrontType FrontType;
		typedef std::vector<spatial::TPoint<moving_boundary::CoordinateType,2> > FrontType;
		typedef World<moving_boundary::CoordinateType,2> WorldType;
		typedef spatial::TPoint<moving_boundary::CoordinateType,2> WorldPoint;
		typedef spatial::TPoint<double,2> ProblemDomainPoint; 


		MovingBoundaryParabolicProblemImpl(const moving_boundary::MovingBoundarySetup &mbs) 
			:ValidationBase(mbs),
			isRunning(false),
			physiology(mbs.physiology),
			numVolumeVariables(mbs.physiology->numVolumeVariables()),
			world(WorldType::get( )),
			setup_(mbs),
			//diffusionConstant(mbs.diffusionConstant),
			numIteration(0),
			currentTime(0),
			maxTime(mbs.maxTime),
			//TDX			frontTimeStep(mbs.frontTimeStep),
			baselineTime(0),
			baselineGeneration(0),
			minimimMeshInterval(0),

			zeroSourceTerms(boost::logic::indeterminate),

			currentFront( ),
			meshDefinition(createMeshDef(world, mbs, numVolumeVariables)),
			meNodeEnvironment(meshDefinition,physiology),
			interiorVolume(calculateInteriorVolume(world, meshDefinition)),
			primaryMesh(meshDefinition,meNodeEnvironment),
			voronoiMesh(primaryMesh),
			//alloc( ),
			boundaryElements( ),
			lostElements( ),
			gainedElements(  ),
			statusPercent(0),
			estimateProgress(false),
			timeClients( ),
			elementClients( ),
			percentInfo( ),
			stateValues(nullptr),
			pointStateValues(nullptr)
		{  

			MeshElementNode::setProblemToWorldDistanceScale(world.theScale( ));

			assert(nFunctionPointers == 0); 
			double maxConstantDiffusion = std::numeric_limits<double>::min( );
			for (int i = 0; i < numVolumeVariables; i++) {
				const VolumeVariable* sp = physiology->getVolumeVariable(i);
				if (sp->isExpressionConstant(expr_diffusion)) {
					maxConstantDiffusion = std::max(maxConstantDiffusion, sp->getExpressionConstantValue(expr_diffusion ));
				}
			}
			physiology->buildSymbolTable();
			stateValues = new double[physiology->numberSymbols()];
			pointStateValues = new double[physiology->numberPointSymbols()];

			levelExp = new SExpression(mbs.levelFunctionStr, physiology->symbolTable());
			frontVelocityExpX = new SExpression(mbs.frontVelocityFunctionStrX,physiology->symbolTable());
			frontVelocityExpY = new SExpression(mbs.frontVelocityFunctionStrY,physiology->symbolTable());

			vcFront = initFront(world, *this,mbs);
			setInitialValues( );
			setPointInitialValues( );

			//check time step
			/* TDX
			if (mbs.frontTimeStep <= 0) {
			VCELL_EXCEPTION(domain_error,"invalid time step  " << mbs.frontTimeStep);
			}
			*/
			VCell::MapTable mt;
			mt["D"] = maxConstantDiffusion;
			double hx = mt["hx"] = world.distanceToProblemDomain(meshDefinition.interval(spatial::cX));
			double hy = mt["hy"] = world.distanceToProblemDomain(meshDefinition.interval(spatial::cY));
			mt["hmin"] = minimimMeshInterval = std::min(hx,hy);
			mt["hmax"] = std::max(hx,hy);
			VCell::MTExpression fts(mbs.frontTimeStep,mt);
			frontTimeStep = fts.evaluate( );

			double maxStep = minimimMeshInterval * minimimMeshInterval/(4 * maxConstantDiffusion); 
			if (maxStep < frontTimeStep) {
				if (mbs.hardTime) {
					VCELL_EXCEPTION(logic_error,"hard set input time step " << frontTimeStep << " greater than maximum allowed by problem (" << maxStep << ')');
				}
				frontTimeStep = maxStep;
			}

			if (!mbs.outputTimeStep.empty())
			{
				double outputTimeStep;
				std::istringstream(mbs.outputTimeStep) >> outputTimeStep;
				frontTimeStep = outputTimeStep / ( (int)(outputTimeStep/frontTimeStep) + 1);
			}

			using matlabBridge::MatLabDebug;
			if (MatLabDebug::on("tiling")) {
				std::ofstream master("tilingMaster.m");
				matlabBridge::TPolygon<moving_boundary::CoordinateType> pFront("b");
				frontTierAdapt::copyVectorInto(pFront,currentFront);
				int i = 0;
				for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
					MeshElementNode &e = *iter;
					const Volume2DClass & vol = e.getControlVolume();
					if (!vol.empty( )) {
						std::stringstream command;
						command << "tilingX" << e.indexOf(spatial::cX) << 'Y' << e.indexOf(spatial::cY) << std::ends;

						std::stringstream filestr;
						filestr <<  command.str( ).c_str( ) << ".m" << std::ends;
						std::ofstream os(filestr.str( ));

						master << command.str( ) << std::endl;
						master << matlabBridge::PauseSeconds(0.5) << std::endl;
						//os << matlabBridge::FigureName(command.str( ).c_str( ));
						os << matlabBridge::clearFigure << pFront;
						e.writeMatlab(os);
					}
				}
			}
			if (MatLabDebug::on("tilingmovie")) {
				std::ofstream master("tilingMovie.m");
				matlabBridge::Movie movie(master);
				master << "% move 'axis' command to beginning of file" << std::endl;
				master << matlabBridge::FigureName("Tiling, the movie");
				matlabBridge::Polygon pFront("b");
				matlabBridge::FigureLimits<moving_boundary::CoordinateType> limits;
				frontTierAdapt::copyVectorInto(pFront,currentFront);
				int i = 0;
				for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
					MeshElementNode &e = *iter;
					const Volume2DClass & vol = e.getControlVolume();
					if (!vol.empty( )) {
						master << matlabBridge::clearFigure << pFront;
						e.writeMatlab(master, &limits);
						movie.recordFrame( );
					}
				}
				master << "% move this to top before executing "<< std::endl;
				master << limits; 
				movie.play( );
			}
#ifdef TEST_DUMP 
			std::ofstream bp("bp.txt");
			for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				iter->listBoundary(bp,primaryMesh);
			}
#endif
			//validate grid 
			CoordinateType hSpace = primaryMesh.interval(cX);
			CoordinateType vSpace = primaryMesh.interval(cY);
			if (hSpace%2 != 0 || vSpace%2 != 0) {
				VCELL_EXCEPTION(logic_error,"spacings horiz " << hSpace << " and vert " << vSpace << " not even");
			}
			const size_t maxI =  primaryMesh.numCells(cX) - 1;
			const size_t maxJ =  primaryMesh.numCells(cY) - 1;
			for (size_t i = 0; i < maxI; i++) 
				for (size_t j = 0; j < maxJ - 1; j++) 
				{
					std::array<size_t,2> p = {i    ,j};
					std::array<size_t,2> r = {i + 1,j};
					std::array<size_t,2> d = {i    ,j + 1};
					MeshElementNode &  e = primaryMesh.get(p);
					MeshElementNode & down = primaryMesh.get(d);
					MeshElementNode & right = primaryMesh.get(r);
					CoordinateType hGap = right(cX) - e(cX); 
					CoordinateType vGap = down(cY) - e(cY); 
					if (hGap != hSpace) {
						VCELL_EXCEPTION(logic_error, "bad h gap between " << e << " and " << right << ", " << hGap << " != expected " << hSpace);
					}
					if (vGap != vSpace) {
						VCELL_EXCEPTION(logic_error, "bad v gap between " << e << " and " << down << ", " << vGap << " != expected " << vSpace);
					}
				}
		}

		~MovingBoundaryParabolicProblemImpl( ) {
			delete vcFront;
			delete[] stateValues;
		}

		void setInterior(MeshElementNode &e) {
			e.setInteriorVolume(interiorVolume);
		}

		void setConcentrations(MeshElementNode & e) {
			e.allocateSpecies();
			ProblemDomainPoint pdp = world.toProblemDomain(e);
			stateValues[physiology->symbolIndexOfT()] = 0;
			for (int i = 0; i< numVolumeVariables; i++) {
				stateValues[physiology->symbolIndexOfCoordinate()] = pdp(cX);
				stateValues[physiology->symbolIndexOfCoordinate() + 1] = pdp(cY);
				double mu = physiology->getVolumeVariable(i)->evaluateExpression(expr_initial, stateValues);
				e.setConcentration(i,mu);
			}
		}

		/**
		* set initial mesh sizes
		*/
		void setInitialValues( ) {
			using std::vector;
			assert(physiology->numVolumeVariables( ) == numVolumeVariables);

			for (MeshElementNode & e: primaryMesh) {
				e.setInteriorVolume(interiorVolume);
			}
			currentFront = vcFront->retrieveFront( );
			std::cout << "front size " << currentFront.size( ) << std::endl;
			if (currentFront.empty( )) {
				throw std::invalid_argument("empty front");
			}
			VCELL_LOG(warn,"initial front has " << currentFront.size( ) << " points");
			assert(currentFront.front( ) == currentFront.back( ));  //verify closed 
			voronoiMesh.setFront(currentFront);

			//spatial::Positions<MeshElementNode> positions = spatial::classify2(voronoiMesh,currentFront);
			moving_boundary::Positions<MeshElementNode> positions = voronoiMesh.classify2(currentFront);
			for (vector<MeshElementNode *>::iterator iter = positions.inside.begin( ); iter != positions.inside.end( ); ++iter) {
				MeshElementNode  & insideElement = **iter;
				setConcentrations(insideElement);
				insideElement.getControlVolume();
			}
			for (vector<MeshElementNode *>::iterator iter = positions.boundary.begin( ); iter != positions.boundary.end( ); ++iter) {
				MeshElementNode  & boundaryElement = **iter;
				setConcentrations(boundaryElement);
				boundaryElements.push_back(&boundaryElement);
				boundaryElement.findNeighborEdges();
			}
			/*
			for (vector<MeshElementNode *>::iterator iter = positions.outside.begin( ); iter != positions.outside.end( ); ++iter) {
			}
			*/
		}

		void boundaryElementSetup(MeshElementNode & bElem) {
			assert (bElem.mPos( ) == spatial::boundarySurface);
		}

		/**
		* constructor support; front initialization
		*/
		static spatial::VCellFront<moving_boundary::CoordinateType> *initFront(const WorldType & world, const ValidationBase & base,const moving_boundary::MovingBoundarySetup &mbs)  {
            typedef spatial::TGeoLimit<int> LimitType;
            const std::array<LimitType,2> & worldLimits = world.limits( );
            std::vector<spatial::GeoLimit> limits(worldLimits.size( ));
            std::transform(worldLimits.begin( ),worldLimits.end( ),limits.begin( ),spatial::GeoLimitConvert<int, double>( ) );
            if (base.nFunctionPointers == 0) {
                spatial::VCellFront<int> *prv = new spatial::VCellFront<int>(limits, mbs, base, base);
                return prv;
            }  
            throw std::domain_error("function pointers no longer supported");
		}

		/**
		* constructor support; mesh initialization
		*/
		static MBMeshDef createMeshDef(const WorldType & world, const moving_boundary::MovingBoundarySetup &mbs, int numVolumeVariables)  {


			typedef spatial::TGeoLimit<moving_boundary::CoordinateType> LimitType;
			using vcell_util::arrayInit;
			const std::array<LimitType,2> & worldLimits = world.limits( );
			std::array<moving_boundary::CoordinateType,2> origin= arrayInit<moving_boundary::CoordinateType>(worldLimits[0].low( ),worldLimits[1].low( ) );
			std::array<moving_boundary::CoordinateType,2> c = arrayInit<moving_boundary::CoordinateType>(worldLimits[0].span( ),worldLimits[1].span( ) );
			const Universe<2> & universe = world.universe( );
			std::array<size_t,2> p = { universe.numNodes( )[0], universe.numNodes( )[1] };
			return MBMeshDef(origin,c,p, numVolumeVariables);
		}

		/**
		* constructor support; calculate interior volume 
		*/
		static moving_boundary::VolumeType calculateInteriorVolume(const WorldType & world, const MBMeshDef & meshDefinition) {
			//direct multiplication may not work due to type overflow -- assign to VolumeType for type conversion
			moving_boundary::VolumeType rval = meshDefinition.interval(spatial::cX);
			rval *= meshDefinition.interval(spatial::cY);
			rval /= world.theScale( ) * world.theScale( );
			return rval;
		}

		virtual double level(double *in) const {
			double problemDomainValues[2];
			world.toProblemDomain(in,problemDomainValues);
			std::memcpy(stateValues + physiology->symbolIndexOfCoordinate(), problemDomainValues, DIM * sizeof(double));
			double r = levelExp->evaluate(stateValues);
			/*
			if (r > 0) {
			scatterInside.add(problemDomainValues[0],problemDomainValues[1]);
			}
			else {
			scatterOutside.add(problemDomainValues[0],problemDomainValues[1]);
			}
			*/
			return r;
		}

        MeshElementNode* findNearestInsidePointFromNeighbors(const CoordVect& thisPoint, const IndexVect& gridIndex, int layer) const
        {
            MeshElementNode* selectedElement = nullptr;
            double minDistance = std::numeric_limits<double>::max();
            for (int offset_i = -layer; offset_i <= layer; ++ offset_i)
            {
                for (int offset_j = -layer; offset_j <= layer; ++ offset_j)
                {
                    if (std::max(std::abs(offset_i), std::abs(offset_j)) < layer)
                    {
                        continue;
                    }
                    IndexVect neighbor = gridIndex + IndexVect(offset_i, offset_j);
                    MeshElementNode* neighborElement = primaryMesh.query(neighbor);
                    if (neighborElement != nullptr && neighborElement->isInside())
                    {
                        CoordVect neighborCoord(*neighborElement);
                        double distance = thisPoint.distance2(neighborCoord);
                        if (distance < minDistance)
                        {
                            minDistance = distance;
                            selectedElement = neighborElement;
                        }
                    }
                } // end for j
            } // end for i
            return selectedElement;
        }
        
		MeshElementNode* findNearestInsidePoint(const CoordVect& thisPoint) const
		{
			static const string METHOD = "findNearestInsidePoint";
			vcell_util::Logger::debugEntry(METHOD);

			CoordVect scaledCoord = (thisPoint - primaryMesh.geoOrigin())/primaryMesh.Dx();
			IndexVect gridIndex((int) (scaledCoord[0]), (int) scaledCoord[1]);
			// !!!!!! !!!attention: deal with wall case later!!!

			MeshElementNode* thisElement = primaryMesh.query(gridIndex);
			if (thisElement == nullptr)
			{
				std::stringstream ss;
				ss << METHOD << ", Point " << thisPoint << "@" << gridIndex << " is out of domain boundary";
				throw ss.str();
			}

			MeshElementNode* selectedElement = nullptr;
			if (thisElement->isInside())
			{
				selectedElement = thisElement;
			}
			else
			{
				{   // scope the variables
					IndexVect offset(0, 0);
					for (int i = 0; i < DIM; ++i)
					{
						offset[i] = scaledCoord[i] < gridIndex[i] + 0.5 ? -1 : 1;
					}

					// check the following 2 neighbors
					// (i + delta, j)
					IndexVect neighbor1 = gridIndex + IndexVect(offset[0], 0);
					MeshElementNode* neighbor1Element = primaryMesh.query(neighbor1);
					bool neighbor1Inside = neighbor1Element != nullptr && neighbor1Element->isInside();

					// (i, j + delta)
					IndexVect neighbor2 = gridIndex + IndexVect(0, offset[1]);
					MeshElementNode* neighbor2Element = primaryMesh.query(neighbor2);
					bool neighbor2Inside = neighbor2Element != nullptr && neighbor2Element->isInside();

					if (neighbor1Inside && neighbor2Inside)
					{
						CoordVect neighbor1Coord(*neighbor1Element);
						CoordVect neighbor2Coord(*neighbor2Element);
						double distance1 = thisPoint.distance2(neighbor1Coord);
						double distance2 = thisPoint.distance2(neighbor2Coord);
						selectedElement = distance1 < distance2 ? neighbor1Element : neighbor2Element;
					}
					else if (neighbor1Inside)
					{
						selectedElement = neighbor1Element;
					}
					else if (neighbor2Inside)
					{
						selectedElement = neighbor2Element;
					}
					else
					{
						// (i + delta, j + delta)
						IndexVect diagNeighbor = gridIndex + offset;
						MeshElementNode* diagNeighborElement = primaryMesh.query(diagNeighbor);
						if (diagNeighborElement != nullptr && diagNeighborElement->isInside())
						{
							selectedElement = diagNeighborElement;
						}
					}
				}

				if (selectedElement == nullptr)
				{
                    int layer = 1;
                    selectedElement = findNearestInsidePointFromNeighbors(thisPoint, gridIndex, layer);
				}
                if (selectedElement == nullptr)
				{
                    int layer = 2;
                    VCELL_LOG_ALWAYS("At t=" << currentTime << ", going into 2nd layer to find nearest inside point for point " << thisPoint);
                    selectedElement = findNearestInsidePointFromNeighbors(thisPoint, gridIndex, layer);
				}
			}

			vcell_util::Logger::debugExit(METHOD);
			return selectedElement;
		}

		/**
		* front velocity returned in problem domain units
		* @param x world coordinate
		* @param y world coordinate
		*/
		spatial::SVector<double,2> frontVelocityProbDomain(const CoordVect& coord, const CoordVect& normal) const {
			 static const string METHOD = "frontVelocityProbDomain";
			 vcell_util::Logger::debugEntry(METHOD);

			 if (coord.withinWorld())
			 {
					CoordVect thisPoint = world.toProblemDomain(coord);
					stateValues[physiology->symbolIndexOfT()] = currentTime;
					std::memcpy(stateValues + physiology->symbolIndexOfCoordinate(), thisPoint.data(), DIM * sizeof(double));
					std::memcpy(stateValues + physiology->symbolIndexOfNormal(), normal.data(), DIM * sizeof(double));
					if (frontVelocityExpX->isConcentrationDependent() || frontVelocityExpX->isConcentrationDependent())
					{
						if (isRunning)
						{
							MeshElementNode* nearestInsidePoint = findNearestInsidePoint(thisPoint);
							if (nearestInsidePoint == nullptr)
							{
								std::stringstream ss;
								ss << "At t=" << currentTime << ", did not find an inside neighbor to point " << thisPoint << " for extrapolation";
								vcell_util::Logger::Debug(METHOD, ss.str());
								throw ss.str();
							}
							const double* conc = nearestInsidePoint->priorConcentrations();
							std::memcpy(stateValues + physiology->symbolIndexOfSpecies(), conc, numVolumeVariables * sizeof(double));
						}
						else
						{
							// get concentration from from initial condition
							for (int i = 0; i < numVolumeVariables; ++ i)
							{
								stateValues[physiology->symbolIndexOfSpecies() + i] = physiology->getVolumeVariable(i)->evaluateExpression(expr_initial, stateValues);
							}
						}
					}
//					double C = 1;
//					if (isRunning)
//					{
//						std::vector<MeshElementNode*> stencil;
//						findExtrapolationStencil(thisPoint, 1, stencil);
//						C = stencil[0]->priorConcentration(0);
//					}
//					CoordVect n = thisPoint.normalize();
//					vel = n * (1-currentTime);
			 }

			/*
			enum {ex = 0, ey = 1, et = 2};
			syms[et] = currentTime; 
			*/
			double vX = frontVelocityExpX->evaluate(stateValues);
			double vY = frontVelocityExpY->evaluate(stateValues);
			vcell_util::Logger::debugExit(METHOD);
//			return spatial::SVector<double,2>(vel[0], vel[1]);
			return spatial::SVector<double,2>(vX, vY);
		}

		/**
		* front velocity returned  in world units 
		* @param x world coordinate
		* @param y world coordinate
		*/
		spatial::SVector<moving_boundary::VelocityType,2> frontVelocity(const CoordVect& coord, const CoordVect& normal) const {
			//fspy << x << ',' << y << ',' << currentTime << std::endl;
			auto rval =  world.toWorld<moving_boundary::VelocityType>(frontVelocityProbDomain(coord, normal));
			return rval; 
		}

		void computeAdvection(MeshElementNode& e)
		{
			if (e.isOutside())
			{
				return;
			}
			VCELL_LOG(trace,e.indexInfo( ) <<  " computeAdvection");
			double worldValues[2] = {static_cast<double>(e(cX)), static_cast<double>(e(cY))};
			double syms[2];
			world.toProblemDomain(worldValues,syms);
			stateValues[physiology->symbolIndexOfT()] = currentTime;
			std::memcpy(stateValues + physiology->symbolIndexOfCoordinate(), syms, DIM * sizeof(double));
			const double* conc = e.priorConcentrations();
			std::memcpy(stateValues + physiology->symbolIndexOfSpecies(), conc, numVolumeVariables * sizeof(double));
			for (int s = 0; s < numVolumeVariables; ++ s)
			{
				const VolumeVariable* volumeVariable = physiology->getVolumeVariable(s);
				if (volumeVariable->isAdvecting())
				{
					double vX = volumeVariable->evaluateExpression(moving_boundary::expr_advection_x, stateValues);
					double vY = volumeVariable->evaluateExpression(moving_boundary::expr_advection_y, stateValues);
					CoordVect cv(vX, vY);
					CoordVect v = world.toWorld(cv);
					e.setAdvection(s, v);
				}
			}
		}

		virtual int velocity(Front* front, FT_POINT* fpoint, HYPER_SURF_ELEMENT* hse, HYPER_SURF* hs, double* out) const {
			CoordVect coord(fpoint->_coords[cX],fpoint->_coords[cY]);
			double nor[MAXD];
			GetFrontNormal(fpoint, hse, hs, nor,front);
			CoordVect normal(nor);
			const spatial::SVector<double, 2> & v = frontVelocity(coord, normal);
			out[cX] = v(cX); 
			out[cY] = v(cY); 

			return 0; //return value not used by frontier
		}
		/**
		* @return <timeNow, timeIncrement>
		*/
		std::pair<double,double> times(int numIteration) {
			double timeNow = baselineTime + (numIteration - baselineGeneration) * frontTimeStep;
			if (timeNow > maxTime) {
				timeNow = maxTime;
			}
			double incr = timeNow - currentTime;
			return std::pair<double,double>(timeNow,incr);
		}

		/**
		* update front time step to ensure last step(s) do not proceed past maxTime
		* for the simulation, and try make steps as even as possible
		* @param desiredStep 
		* @param numIteration for comparing to baseline
		*/

		void updateTimeStep(double desiredStep, int numIteration) {
			double intervalTilEnd = maxTime - baselineTime;
			int n = ceil(intervalTilEnd / desiredStep);
			frontTimeStep = intervalTilEnd / n;
			baselineGeneration = numIteration;
			baselineTime = currentTime;
		}

		//This implementation is optimized for the case 10/2014 where this is but a single "element" client
		//If there are multiple clients it could be better to loop over them inside this function -- probably
		//more efficient to iterate over std::vector than the mesh
		/**
		* @param changed nodes have changed since last report
		*/
		void giveElementsToClient(MovingBoundaryElementClient & client, size_t numIteration, bool changed) {
			for (MBMesh::const_iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				//std::cout << iter->ident( ) << std::endl;
				if (iter->isInside( )) {
					client.element(*iter);
				}
			}
			client.iterationComplete();
		}

		/**
		* notify all clients that time has changed, and give the elements to those  that want them
		*/
		void notifyClients(size_t numIteration, bool changed) {
			for (MovingBoundaryTimeClient *tclient: timeClients) {
				GeometryInfo<moving_boundary::CoordinateType> gi(currentFront,changed);
				tclient->time(currentTime, numIteration, currentTime == maxTime, gi);
			}
			for (MovingBoundaryElementClient *eclient: elementClients) {
				giveElementsToClient(*eclient,numIteration,changed);
			}
		}

		void debugDump(size_t gc, char key) {
			static bool fullDump = matlabBridge::MatLabDebug::on("frontmovefull");
			VCELL_LOG(trace,"dd " << gc << key << " time " << currentTime); 
			std::ostringstream oss;
			oss << "frontmove"  << std::setfill('0') << std::setw(3) << gc << key << ".m" << std::ends;
			std::ofstream dump(oss.str( ));
			{
				std::ostringstream mbuf;
				mbuf << "Time " << currentTime;
				dump << matlabBridge::ConsoleMessage(mbuf.str());
				dump << matlabBridge::FigureName(oss.str( ).c_str( ));
			}
			matlabBridge::TScatter<CoordinateType> deepInside('b',3,false,0);
			matlabBridge::TScatter<CoordinateType> inside('b',2,true,1);
			matlabBridge::TScatter<CoordinateType> boundary('g',2,true,2);
			matlabBridge::TScatter<CoordinateType> outside('r',2,true,3);
			matlabBridge::TScatter<CoordinateType> deepOutside('r',3,false,4);
			for (MBMesh::const_iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter) {
				switch(iter->mPos( )) {
				case spatial::interiorSurface:
					if (iter->boundaryOffset( ) > 1) {
						frontTierAdapt::copyPointInto(deepInside,*iter);
					}
					else {
						frontTierAdapt::copyPointInto(inside,*iter);
					}
					break;
				case spatial::boundarySurface:
					frontTierAdapt::copyPointInto(boundary,*iter);
					break;
				case spatial::outsideSurface:
					if (iter->boundaryOffset( ) > 1) {
						frontTierAdapt::copyPointInto(deepOutside,*iter);
					}
					else {
						frontTierAdapt::copyPointInto(outside,*iter);
					}
					break;
				default:
					VCELL_EXCEPTION(domain_error,"bad state " << static_cast<int>(iter->mPos( )) << iter->ident( )) ;
				}
				VCELL_LOG(trace,"dd " << iter->ident( ));
				double  x = iter->get(cX);
				double  y = iter->get(cY);

				std::stringstream ss;
				ss << iter->indexOf(cX) << ',' << iter->indexOf(cY) << '-' << static_cast<unsigned int>(iter->boundaryOffset( )) << std::ends;
				dump << matlabBridge::Text(x,y, ss.str( ).c_str( ));
				if (fullDump) {
					matlabBridge::Polygons pgons("k");
					const Volume2DClass & vol = iter->getControlVolume( );
					frontTierAdapt::copyVectorsInto(pgons,vol.points( ));
					dump << pgons;
				}
			}
			dump << deepInside << inside << boundary << outside << deepOutside; 
			matlabBridge::TPolygon<CoordinateType> pPoly("k",1);
			frontTierAdapt::copyVectorInto(pPoly,currentFront);
			dump << pPoly; 
		}

		static void commenceSimulation(MeshElementNode &e) {
			e.beginSimulation( );
		}

		/**
		* Functor
		* base class for functors which need access to #Outer
		*/
		struct FunctorBase {
			FunctorBase(const Outer &o)
				:outer(o) {}
			const Outer & outer;
		};
		/**
		* Functor
		* move front
		*/
		struct MoveFront : public FunctorBase {
			MoveFront(Outer &o)
				:FunctorBase(o){}
			void operator( )(MeshElementNode * p) {
				assert(p != nullptr);
				VCELL_LOG(trace,p->indexInfo( ) << " front move")
					if (p->mPos() == spatial::boundarySurface) {
						p->moveFront(this->outer.currentFront);
					}
					else {
						throw std::domain_error("MoveFront");
					}
			}
		};

		/**
		* Functor
		* FindNeighborEdges 
		*/
		struct FindNeighborEdges  : public FunctorBase {
			FindNeighborEdges (Outer &o)
				:FunctorBase(o) {}

			void operator( )(MeshElementNode * p) {
				if (p->isInside( ) ) {
					p->findNeighborEdges( );
				}
			}
		};
#ifdef HOLD
#error should use front velocity, not elements
		/**
		* Functor
		* apply velocity, record max
		*/
		struct MaxVel : public FunctorBase {
			MaxVel (Outer &o)
				:FunctorBase(o),
				maxVel(0) {}
			void operator( )(MeshElementNode & e) {
				VCELL_LOG(trace,e.indexInfo( ) << " max velocity")
					SVector<double,2> v = this->outer.velocity(e(cX),e(cY));
				maxVel  = std::max(maxVel, v(cX));
				maxVel  = std::max(maxVel, v(cY));
				e.setVelocity(v);
			}

			REAL maxVel;
		};
#endif
		/**
		* Functor
		* find maximum velocity of front points (squared) 
		*/
		struct FrontVelocity : public FunctorBase {
			FrontVelocity (Outer &o)
				:FunctorBase(o),
				maxSquaredVel(0){}
			void operator( )(const spatial::TPoint<CoordinateType,2> & point) {
				CoordVect coord(point.getCoords());
				CoordVect normal(point.getNormal());
				spatial::SVector<double,2> velVector = this->outer.frontVelocityProbDomain(coord, normal);
				maxSquaredVel = std::max(maxSquaredVel,velVector.magnitudeSquared( ));
			}
			moving_boundary::VelocityType maxSquaredVel;
		};

		/**
		* Functor diffuseAdvect
		*/
		template <class SOLVER>
		struct DiffuseAdvect {
			DiffuseAdvect(SOLVER &s)
				:speciesIdx(0),
				solver(s) {}

			void operator( )(MeshElementNode & e) {
				VCELL_LOG(trace,e.ident( ) << " diffuseAdvect");
				e.diffuseAdvect(solver,speciesIdx);
			}
			unsigned int speciesIdx;
			SOLVER &solver;

		};

		/**
		* Functor advectComplete
		*/
		struct AdvectComplete {
			AdvectComplete( ) {} //required by Mac OS X compiler
			void operator( )(MeshElementNode & e) {
				if (e.isInside( )) {
					VCELL_LOG(trace,e.ident( ) << " advectComplete");
					//e.advectComplete( );
				}
			}
		};

		/**
		* Functor react 
		*/
		struct React {
			React(double time_, double timeStep_)
				:time(time_),
				timeStep(timeStep_){}

			void operator( )(MeshElementNode & e) {
				if (e.isInside( )) {
					VCELL_LOG(trace,e.ident( ) << " react")
						e.react(time, timeStep);
				}
			}

			const double time;
			const double timeStep;
		};

#if 0
		/**
		* Functor ClearDiffuseAdvect
		*/
		struct ClearDiffuseAdvect {
			void operator( )(MeshElementNode & e) {
				if (e.mPos( ) != outsideSurface) {
					VCELL_LOG(trace,e.ident( ) << " clearDiffuseAdvect")
						e.clearDiffuseAdvectAmounts( );
				}
			}

		};
#endif

		/**
		* Functor
		* collect mass 
		*/
		struct CollectMass : public FunctorBase {
			CollectMass (Outer &o)
				:FunctorBase(o) {}
			void operator( )(MeshElementNode *p) {
				VCELL_LOG(trace,p->ident( ) << " collectMass")
					p->collectMass(this->outer.currentFront);
			}
		};

		/**
		* Functor
		* apply front
		*/
		struct ApplyFront : public FunctorBase {
			ApplyFront(Outer &o)
				:FunctorBase(o) {}
			void operator( )(MeshElementNode *p) {
				VCELL_LOG(trace,p->ident( ) << " applyFront")
					p->applyFront(this->outer.currentFront, this->outer.interiorVolume);
			}
		};

		/**
		* Functor
		* distribute lost mass 
		*/
		struct DistributeLost { //OPTIMIZE -- keep collection of objects in state 
			void operator( )(MeshElementNode &e) {
				VCELL_LOG(trace,e.indexInfo( ) << " distributeLost")
					e.distributeMassToNeighbors();
			}
		};

		/**
		* Functor
		* distribute lost mass 
		*/
		struct EndCycle : public FunctorBase {
			EndCycle(Outer &o)
				:FunctorBase(o) {}
			void operator( )(MeshElementNode &e) {
				VCELL_LOG(trace,e.indexInfo( ) << " end of cycle");
				e.endOfCycle( );
			}
		};

		void add(MovingBoundaryElementClient & client) {
			VCELL_LOG(info,"added element client " << typeid(client).name( ) );
			if (std::find(elementClients.begin( ), elementClients.end( ),&client) == elementClients.end( ) ) {
				elementClients.push_back(&client);
			}
			add(static_cast<MovingBoundaryTimeClient &>(client) );
		}

		void add(MovingBoundaryTimeClient & client) {
			VCELL_LOG(info,"added time client " << typeid(client).name( ) );
			if (std::find(timeClients.begin( ), timeClients.end( ),&client) == timeClients.end( ) ) {
				timeClients.push_back(&client);
			}
		}

		std::string getOutputFiles()
		{
			std::string files;
			for (MovingBoundaryTimeClient *tclient: timeClients) {
				files += tclient->outputName() + ", ";
			}
			return files;
		}

		/**
		* package of percent progress variables for convenience / clarity
		*/
		struct ProgressPercentInfo {
			double simStartTime;
			double lastPercentTime; //PDEL
			double progressDelta; //statusPercent in time 
			size_t nextProgressGeneration; 
			std::chrono::steady_clock::time_point runStartTime;

			/**
			* initialize to unused state
			*/
			ProgressPercentInfo( )
				:simStartTime(0),
				lastPercentTime(0),
				progressDelta(0),
				nextProgressGeneration(inactiveValue( )),
				runStartTime( ) {}

			/**
			* estimate next iteration a percent should be output
			* assumes progressData et. al. have been set externally
			*/
			void calculateNextProgress(size_t currentGeneration, double frontTimeStep) {
				//double nextTime = lastPercentTime + progressDelta;
				nextProgressGeneration = static_cast<size_t>(progressDelta/frontTimeStep) + currentGeneration;
				VCELL_KEY_LOG(debug,Key::progressEstimate, "PE lpt " << lastPercentTime << " pd " << progressDelta 
					//	<< " nextTime " << nextTime 
					<< " frontTimeStep " << frontTimeStep << " current gen " << currentGeneration 
					<< " next Gen " << nextProgressGeneration);
			}

			bool isActive( ) const {
				return nextProgressGeneration != inactiveValue( );
			}

			static size_t inactiveValue( ) {
				return std::numeric_limits<size_t>::max( );
			}
		};

		double getFTMaxSpeed()
		{
			CoordVect maxSpeed(Spfr(vcFront->c_ptr()));
			maxSpeed = world.toProblemDomain(maxSpeed);
			return maxSpeed.max();
		}

		void populatePointStateValues()
				{
			pointStateValues[physiology->pointSymbolIndexOfT()] = currentTime;
			for (int ipv = 0; ipv < physiology->numPointVariables(); ++ ipv)
			{
				pointStateValues[physiology->pointSymbolIndexOfSpecies() + ipv] = physiology->getPointVariable(ipv)->getCurrSol(0);
			}
		}

		void updatePointPositions()
		{
			populatePointStateValues();
			for (int ips = 0; ips < physiology->numPointSubdomains(); ++ ips)
			{
				PointSubdomain* ps = physiology->getPointSubdomain(ips);
				ps->updatePosition(pointStateValues);
			}
		}

		void setPointInitialValues()
		{
			// Set intial conditions of Point Variables, which we assume is a constant for now
			for (int ips = 0; ips < physiology->numPointSubdomains(); ++ ips )
			{
				PointSubdomain* ps = physiology->getPointSubdomain(ips);
				for (int ipv = 0; ipv < ps->numVariables(); ++ ipv)
				{
					PointVariable* pv = (PointVariable*)ps->getVariable(ipv);
					double* currSol = pv->getCurrSol();
					currSol[0] = pv->getExpressionConstantValue(expr_initial);
				}
			}
			// with initial values of point variables, compute initial position
			updatePointPositions();
		}

		void solvePointVariables(double dT)
		{
			populatePointStateValues();
			for (int ips = 0; ips < physiology->numPointSubdomains(); ++ ips)
			{
				PointSubdomain* ps = physiology->getPointSubdomain(ips);
				// one step solve for all point variables defined in this point subdomain
				std::memcpy(pointStateValues + physiology->pointSymbolIndexOfCoordinate(), ps->getPositionValues(), DIM * sizeof(double));
				for (int ipv = 0; ipv < ps->numVariables(); ++ ipv)
				{
					 // advance solution for each point variable in this pt subdomain (explicit euler)
					PointVariable* pv = (PointVariable*)ps->getVariable(ipv);
					double* currSol = pv->getCurrSol();
					currSol[0] += dT * pv->evaluateExpression(expr_source, pointStateValues);
				}
			}

			// with new values of point variables, compute position again for output and next time step
			updatePointPositions();
		}

#ifndef OLD_BOUNDARY_OFFSETS
	void setBoundaryOffsetValues()
	{
		static bool compareOldNewBoundaryOffsets = false;

		//initialize offset for all volume elements: 2 outside, 3 inside, 0 at points near membrane
		// use outsideMaxOffset and insideMaxOffset (assume >outsideMaxOffset)
		int outsideMaxOffset = 2;
		int insideMaxOffset = 3;
		assert(insideMaxOffset >= outsideMaxOffset);
		for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter)
		{
			MeshElementNode &e = *iter;
			int offset = 0;
			if (e.isBoundary())
			{
				offset = 0;
			}
			else if (e.isInside())
			{
				offset = insideMaxOffset;
			}
			else
			{
				offset = outsideMaxOffset;
			}
			e.setBoundaryOffset(offset);
		}

		const int numNaturalNeighbors = 4;
		for (unsigned char thisOffset = 1; thisOffset <= insideMaxOffset - 1; ++ thisOffset)
		{
			for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter)
			{
				MeshElementNode &e = *iter;
				if (thisOffset == outsideMaxOffset  && !e.isInside())
				{
					continue; // for outside points, stop at offset =2
				}
				if (e.boundaryOffset() == thisOffset - 1)
				{
					IndexVect gridIndex(e.indexOf(cX), e.indexOf(cY));

					for (int n = 0; n < numNaturalNeighbors; ++ n)
					{
						MeshElementNode* neighborElement = primaryMesh.query(gridIndex + naturalNeighborOffsets[n]);
						if (neighborElement != nullptr)
						{
							neighborElement->setBoundaryOffset(std::min(thisOffset, neighborElement->boundaryOffset()));
						}
					}
				}
			}
		}

		if (compareOldNewBoundaryOffsets)
		{
			boundaryElements.front( )->setBoundaryOffsetValues( );

			// compare old and new
			VCELL_LOG_ALWAYS("-----------Comparing old and new boundary offsets started ");
			for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter)
			{
				MeshElementNode &e = *iter;
				e.compareBoundaryOffsets();
			}
			VCELL_LOG_ALWAYS("------------Comparing old and new boundary offsets finished ");
		}
	}
#endif

		void run( ) {
			VCELL_LOG(info,"commence simulation");

			if (SimulationMessaging *messaging = moving_boundary::jobMessaging()) {
				messaging->setWorkerEvent(JobEvent::JOB_STARTING, "Simulation started");
			}

			const AdvectComplete advectComplete;
			ExplicitSolver solver(primaryMesh);
			DiffuseAdvect<ExplicitSolver> diffuseAdvect(solver);
			isRunning = true;

			if (statusPercent > 0)  {
				percentInfo.simStartTime = currentTime;
				percentInfo.progressDelta = statusPercent / 100.0 * maxTime;
				percentInfo.calculateNextProgress(numIteration,frontTimeStep);
				if (estimateProgress) {
					percentInfo.runStartTime = std::chrono::steady_clock::now( );
				}
			}

			const bool frontMoveTrace = matlabBridge::MatLabDebug::on("frontmove") || matlabBridge::MatLabDebug::on("frontmovefull");
			FrontType oldFront; //for debugging

			try { 
				if (numIteration == 0) {
					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),commenceSimulation);
				}
				notifyClients(numIteration,true);

				if (SimulationMessaging *messaging = moving_boundary::jobMessaging()) {
					messaging->setWorkerEvent(JobEvent::JOB_DATA, 0, 0);
				}

				if (frontMoveTrace) {
					debugDump(numIteration, 's');
				}

				++ numIteration;
				while (currentTime < maxTime) {
					solver.begin( );
					std::pair<double,double> nowAndStep = times(numIteration);
					//TODO -- we're approximating front velocity for time step with velocity at beginning of time step
					FrontVelocity fv = std::for_each(currentFront.begin( ),currentFront.end( ),FrontVelocity(*this));
					double maxVel_Gerard = sqrt(fv.maxSquaredVel);
//					double maxVel_FT = getFTMaxSpeed();
					double maxVel = maxVel_Gerard;
					double maxTimeStep_ = minimimMeshInterval / (2 * maxVel);
					if (nowAndStep.second > maxTimeStep_) {
						updateTimeStep(maxTimeStep_,numIteration - 1);
						nowAndStep = times(numIteration);
					}
					//currentTime = nowAndStep.first;
					const double endOfStepTime = nowAndStep.first;
					double dT = nowAndStep.second;

					for (MBMesh::iterator iter = primaryMesh.begin( ); iter != primaryMesh.end( ); ++iter)
					{
						MeshElementNode &e = *iter;
						computeAdvection(e);
					}

					vcFront->propagateTo(endOfStepTime); 
					VCELL_LOG_ALWAYS("t=" << currentTime << ", t_next=" << endOfStepTime << ", dt=" << dT
							<< ", iteration=" << numIteration << ", front has " << currentFront.size( ) << " points"
//							<< std::setprecision(10) << ", maxVel_gerard=" << maxVel_gerard << ",maxVel_FT="<<maxVel_FT
					);
					/*
					VCellFront *vcp = dynamic_cast<VCellFront *>(vcFront);
					if (vcp != nullptr) {
					Front * checkpoint = copy_front(vcp->c_ptr( ));
					FT_Save(checkpoint, "testsave.txt");
					}
					*/


					if (frontMoveTrace) {
						oldFront = currentFront; //for debugging
					}
					currentFront = vcFront->retrieveFront( );
					voronoiMesh.setFront(currentFront);

					{
						//					std::ofstream f("fullvdump.m");
						//					voronoiMesh.matlabPlot(f, &currentFront);
					}
					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),MoveFront(*this));

					try {
						std::for_each(primaryMesh.begin( ),primaryMesh.end( ), React(currentTime,dT) );
					} catch (ReverseLengthException &rle) {
						std::ofstream s("rle.m");
						rle.aElement.writeMatlab(s, nullptr,false, 20);
						rle.bElement.writeMatlab(s, nullptr,false, 20);
						throw;
					}
					primaryMesh.diffuseAdvectCache( ).start( );
					for (unsigned s = 0; s < physiology->numVolumeVariables( ) ; s++) {
						solver.setStepAndSpecies(dT,s);
						diffuseAdvect.speciesIdx = s;
						std::for_each(primaryMesh.begin( ),primaryMesh.end( ), diffuseAdvect);
						solver.solve( );
					}
					primaryMesh.diffuseAdvectCache( ).finish( );
					//std::for_each(primaryMesh.begin( ),primaryMesh.end( ), advectComplete);

					solvePointVariables(dT);

					currentTime = endOfStepTime;
					if (frontMoveTrace) {
						debugDump(numIteration,'b');
					}


					bool changed;
					//adjustNodes calls MeshElementNode.updateBoundaryNeighbors
					try {
						changed = voronoiMesh.adjustNodes(boundaryElements);
					} catch (SkipsBoundary & skips) {
						std::ofstream sb("sb.m");
						matlabBridge::TPolygon<long long> oldPoly("k",1);
						frontTierAdapt::copyVectorInto(oldPoly,oldFront);
						matlabBridge::TPolygon<long long> newPoly("r",1);
						frontTierAdapt::copyVectorInto(newPoly,currentFront);
						matlabBridge::TScatter<long> scatter('b',2,true);
						scatter.add(skips.mes(cX),skips.mes(cY));
						sb << '%' << skips.mes.ident( ) << std::endl;
						sb << oldPoly << newPoly << scatter;
						std::cerr << "rethrowing skips" << std::endl;
						debugDump(numIteration,'e');
						throw;
					}
					//TODO: nochange

					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),CollectMass(*this));

					std::for_each(boundaryElements.begin( ),boundaryElements.end( ),ApplyFront(*this));

					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),DistributeLost());

					std::for_each(primaryMesh.begin( ),primaryMesh.end( ),EndCycle(*this));

#ifdef OLD_BOUNDARY_OFFSETS
					boundaryElements.front( )->setBoundaryOffsetValues( );
#else
					setBoundaryOffsetValues();
#endif


					if (frontMoveTrace) {
						debugDump(numIteration,'a');
					}

					//tell the clients about it
					notifyClients(numIteration ++, changed);
					if (numIteration >= percentInfo.nextProgressGeneration) {
						unsigned int percent = static_cast<unsigned int>(100 * currentTime / maxTime + 0.5);
						if (percent < 100) { //looks silly to report 100% when still running
							//std::cout << std::setw(2) << percent << "% complete";
							if (estimateProgress) {
								namespace chrono = std::chrono;
								using chrono::steady_clock;
								steady_clock::time_point timeNow = steady_clock::now( );
								steady_clock::duration elapsed = timeNow - percentInfo.runStartTime;
								chrono::seconds eseconds = chrono::duration_cast<chrono::seconds>(elapsed); 
								VCELL_KEY_LOG(debug,Key::progressEstimate, "PE percent " << percent << " elapsed " << eseconds.count( ));

								//simulation may not have started at time 0 if this run was restored from a persisted problem
								const double simTimeThisRun = maxTime - percentInfo.simStartTime;
								const double simTimeThusFar = currentTime - percentInfo.simStartTime;
								if (eseconds.count( ) > 0) { //can't estimate at beginning
									const double t = eseconds.count( )  * simTimeThisRun  / simTimeThusFar;
									chrono::seconds total(static_cast<int>(t));
									chrono::seconds remaining = total - eseconds; 
									namespace HMSf = vcell_util::HoursMinutesSecondsFormat;
									typedef vcell_util::HoursMinutesSeconds<chrono::seconds,HMSf::FIXED|HMSf::ALL> HMS;
									typedef vcell_util::HoursMinutesSeconds<chrono::seconds,HMSf::FIXED|HMSf::ALL|HMSf::LONG_UNITS> HMSu;
									std::cout << ", elasped time " << HMS(eseconds) << ", estimated time remaining " << HMSu(remaining);
									VCELL_KEY_LOG(debug,Key::progressEstimate, "PE thisRun " <<  simTimeThisRun 
										<< " thusFar " << simTimeThusFar << " t calc " << t
										<< " est total seconds " << total.count( ) << ' ' << remaining.count( ) << " sec");

								}
							}
							std::cout << std::endl;
							percentInfo.lastPercentTime = currentTime;
							percentInfo.calculateNextProgress(numIteration,frontTimeStep);

							if (SimulationMessaging *messaging = moving_boundary::jobMessaging()) {
								messaging->setWorkerEvent(JobEvent::JOB_PROGRESS, percent * 1.0/100, percentInfo.lastPercentTime);
							}
						}
					}
				}
				if (percentInfo.isActive( )) {
					std::cout << "100 % complete";
					if (estimateProgress) {
						namespace chrono = std::chrono;
						using chrono::steady_clock;
						steady_clock::time_point timeNow = steady_clock::now( );
						steady_clock::duration elapsed = timeNow - percentInfo.runStartTime;
						chrono::seconds eseconds = chrono::duration_cast<chrono::seconds>(elapsed); 
						namespace HMSf = vcell_util::HoursMinutesSecondsFormat;
						typedef vcell_util::HoursMinutesSeconds<chrono::seconds,HMSf::FIXED|HMSf::ALL> HMS;
						std::cout << ", elasped time " << HMS(eseconds); 
					}
					std::cout << std::endl;
				}
				for (MovingBoundaryTimeClient *client : timeClients) {
					client->simulationComplete( );
				}
				if (SimulationMessaging *messaging = moving_boundary::jobMessaging()) {
					messaging->setWorkerEvent(JobEvent::JOB_PROGRESS, 1.0, currentTime);
					messaging->setWorkerEvent(JobEvent::JOB_COMPLETED, 1.0, currentTime);
				}
			} catch (std::exception &e) {
				VCELL_LOG(fatal,"run( ) caught " << e.what( )  << " at iteration " << numIteration << ", time " << currentTime);
				throw;
			}
		}

		const spatial::MeshDef<moving_boundary::CoordinateType,2> & meshDef( ) const {
			return meshDefinition;
		}

		unsigned int numberTimeSteps( ) const {
			return static_cast<unsigned int>(maxTime / frontTimeStep);
		}
		void reportProgress(unsigned char percent, bool estimateTime) {
			if (!isRunning) {
				statusPercent = percent;
				estimateProgress = estimateTime;
				return;
			}
			throw std::logic_error("Call to reportProgress( ) while simulation is running");
		}

		bool noReaction( ) const {
			if (zeroSourceTerms.value == boost::logic::tribool::indeterminate_value) {
				zeroSourceTerms = true; //assume true, test for falseness
				for (auto iter = physiology->beginVolumeVariable( ); iter != physiology->endVolumeVariable( ); ++ iter) {
					if ((*iter)->isExpressionNonZero(moving_boundary::expr_source)) {
						zeroSourceTerms = false; 
						break;
					}
				}
			}
			return (bool)zeroSourceTerms;
		}

		const MovingBoundarySetup & setup( ) const {
			return setup_;
		}

		static void registerType( ) {
			MBMesh::registerType( );
			vcell_persist::Registrar::reg<MovingBoundaryParabolicProblemImpl>("MovingBoundaryParabolicProblemImpl");
		}

		void registerInstanceType( ) const {
			vcFront->registerType(  );
		}

		/**
		* Functor -- convert Element * to index 
		*/
		typedef unsigned short ElementStorageType;

		struct ElementToIndex : public FunctorBase {
			ElementToIndex(const Outer &o)
				:FunctorBase(o) {}

			ElementStorageType operator( )(const MeshElementNode *in) {
				spatial::MeshPosition mp = outer.primaryMesh.indexOf(in->indexes( ));
				return mp.to<ElementStorageType>( ); 
			}
		};

		/**
		* Functor -- convert index to Element *  
		*/
		struct IndexToElement : public FunctorBase {
			IndexToElement(const Outer &o)
				:FunctorBase(o) {}

			MeshElementNode * operator( )(ElementStorageType i) {
				spatial::MeshPosition mp(i);
				return & outer.primaryMesh.get(mp);
			}
		};

		/**
		* persist given vector of elements to os
		* converts Element * to ElementStorageType
		* @tparam E const Element or Element
		*/
		template <class E>
		void persistElementsVector(std::ostream &os, const vector<E *>  &vec) const {
			std::vector<ElementStorageType> writeBuffer(vec.size( ));
			ElementToIndex eToIndex(*this);
			std::transform(vec.begin( ),vec.end( ),writeBuffer.begin( ),eToIndex);
			vcell_persist::save(os,writeBuffer);
		}

		/**
		* restore given vector of elements from is 
		* converts ElementStorageType to (const) Element * 
		* @tparam E const Element or Element
		*/
		template <class E>
		void restoreElementsVector(std::istream &is, vector<E *>  &vec) const {
			std::vector<ElementStorageType> readBuffer; 
			vcell_persist::restore(is,readBuffer);
			IndexToElement iToElement(*this);
			vec.resize( readBuffer.size( ) );
			std::transform(readBuffer.begin( ),readBuffer.end( ),vec.begin( ),iToElement);
		}

		/**
		* save this. Note #MovingBoundarySetup must be saved separately by client
		*/
		void persist(std::ostream &os) const {
//			//setup_.persist(os);
//			vcell_persist::Token::insert<MovingBoundaryParabolicProblemImpl>(os);
//			//vcell_persist::binaryWrite(os,diffusionConstant);
//			vcell_persist::binaryWrite(os,numIteration);
//			vcell_persist::binaryWrite(os,currentTime);
//			//vcell_persist::binaryWrite(os,maxTime);
//			vcell_persist::binaryWrite(os,frontTimeStep);
//			vcell_persist::binaryWrite(os,baselineTime);
//			vcell_persist::binaryWrite(os,baselineGeneration);
//			vcell_persist::binaryWrite(os,minimimMeshInterval);
//			vcFront->persist(os);
//			vcell_persist::save(os,currentFront);
//			meshDefinition.persist(os);
//			primaryMesh.persist(os);
//			vcell_persist::binaryWrite(os,interiorVolume);
//
//			persistElementsVector(os, boundaryElements);
//			persistElementsVector(os, lostElements);
//			persistElementsVector(os, gainedElements);
//			vcell_persist::binaryWrite(os,statusPercent);
//			if (statusPercent > 0) {
//				vcell_persist::binaryWrite(os,estimateProgress);
//			}
		}

		MovingBoundaryParabolicProblemImpl(const moving_boundary::MovingBoundarySetup &mbs, std::istream &is)
			:ValidationBase(mbs),
			isRunning(false),
			physiology(mbs.physiology),
			numVolumeVariables(physiology->numVolumeVariables()),
			world(WorldType::get( )),
			setup_(mbs),
			//diffusionConstant(mbs.diffusionConstant),
			numIteration(0),
			currentTime(0),
			maxTime(mbs.maxTime),
			//TDX			frontTimeStep(mbs.frontTimeStep),
			//TDX			solverTimeStep(mbs.solverTimeStep),
			baselineTime(0),
			baselineGeneration(0),
			minimimMeshInterval(0),

//			symTable(buildSymTable( )),
//			levelExp(mbs.levelFunctionStr,symTable),
//			advectVelocityExpX(mbs.advectVelocityFunctionStrX,symTable),
//			advectVelocityExpY(mbs.advectVelocityFunctionStrY,symTable),
//			frontVelocityExpX(mbs.frontVelocityFunctionStrX,symTable),
//			frontVelocityExpY(mbs.frontVelocityFunctionStrY,symTable),
//			initialConcentrationExpressions( ), //PERSIST TODO
			zeroSourceTerms( ), //PERSIST TODO

			vcFront(nullptr),
			currentFront( ),
			meshDefinition( ),
			meNodeEnvironment(meshDefinition,physiology),
			interiorVolume( ),
			primaryMesh(),
			voronoiMesh(),
			//alloc( ),
			boundaryElements( ),
			lostElements( ),
			gainedElements(  ),
			statusPercent(0),
			estimateProgress(false)
		{
			MeshElementNode::setProblemToWorldDistanceScale(world.theScale( ));
			vcell_persist::Token::check<MovingBoundaryParabolicProblemImpl>(is);
			//vcell_persist::binaryRead(is,diffusionConstant);
			vcell_persist::binaryRead(is,numIteration);
			vcell_persist::binaryRead(is,currentTime);
			//vcell_persist::binaryRead(is,maxTime);
			vcell_persist::binaryRead(is,frontTimeStep);
			vcell_persist::binaryRead(is,baselineTime);
			vcell_persist::binaryRead(is,baselineGeneration);
			vcell_persist::binaryRead(is,minimimMeshInterval);
//			vcFront = moving_boundary::restoreFrontProvider(is);
			vcell_persist::restore(is,currentFront);
			meshDefinition = MBMeshDef(is);
			//MBMesh temp(is);
			primaryMesh.restore(is, meNodeEnvironment);
			vcell_persist::binaryRead(is,interiorVolume);

			restoreElementsVector(is, boundaryElements);
			restoreElementsVector(is, lostElements);
			restoreElementsVector(is, gainedElements);
			vcell_persist::binaryRead(is,statusPercent);
			if (statusPercent > 0) {
				vcell_persist::binaryRead(is,estimateProgress);
			}
			voronoiMesh.setMesh(primaryMesh);
		}

		/**
		* runtime flag (not-persistent), must be first member
		*/
		bool isRunning;

		WorldType & world;
		const MovingBoundarySetup setup_;
		//const double diffusionConstant;

		size_t numIteration;
		/**
		* currentTime must be set before #vcFront initialized
		*/
		double currentTime;
		const double maxTime;
		double frontTimeStep; 
		//double solverTimeStep; 
		/**
		* time last frontTimeStep was calculated
		*/
		double baselineTime;
		/**
		* iteration last time step was calculated
		*/
		unsigned int baselineGeneration;
		/**
		* in problem domain units
		*/
		double minimimMeshInterval;

		/**
		* level expression, must be created before front
		*/
		mutable SExpression* levelExp; //Expression doesn't currently support "const"
		mutable SExpression* frontVelocityExpX;
		mutable SExpression* frontVelocityExpY;
		mutable double* stateValues;
		double* pointStateValues;
		mutable boost::logic::tribool zeroSourceTerms; //logically const, but lazily evaluated
		/**
		* FronTier integration
		*/
		spatial::VCellFront<moving_boundary::CoordinateType> * vcFront;
		FrontType currentFront;
		MBMeshDef meshDefinition;
		Physiology* physiology;
		MeshElementNode::Environment meNodeEnvironment;
		const int numVolumeVariables;

		/**
		* standard volume of inside point
		*/
		double interiorVolume;
		MBMesh primaryMesh;
		VrnMesh voronoiMesh;
		//		vcell_util::ChunkAllocator<VoronoiResult,60> alloc;
		/**
		* current boundary elements
		*/
		//typedef std::forward_list<MeshElementNode *> ElementList; 
		std::vector<MeshElementNode *> boundaryElements;
		/**
		* elements "lost" (moved outside of boundary) during transition between generations
		* point to previous iteration
		*/
		std::vector<const MeshElementNode *> lostElements;
		/**
		* elements "gained" (moved inside of boundary) during transition between generations
		* point to next iteration
		*/
		std::vector<MeshElementNode *> gainedElements;
		unsigned char statusPercent;
		bool estimateProgress;

		/**
		* client storage; not persistent
		*/
		std::vector<MovingBoundaryTimeClient *> timeClients;
		std::vector<MovingBoundaryElementClient *> elementClients;
		ProgressPercentInfo percentInfo;

	};

	//**************************************************
	// local definitions, functors / functions 
	//**************************************************
	namespace {
		struct Evaluator {
			virtual ~Evaluator( ) {}
			virtual double evaluate(MeshElementNode & mes) const = 0;
			virtual const char * const label( ) const = 0;
			virtual const char * const checkFunction( ) const = 0;
			bool supportsCheck( ) const {
				return checkFunction( ) != 0; 
			}
		};

		struct Concentration : public Evaluator {
			virtual double evaluate(MeshElementNode & mes) const { 
				return mes.concentration(0);
			}
			virtual const char * const label( ) const {
				return "concentration";
			}
			virtual const char * const checkFunction( ) const {
				return 0;
			}
		};

		struct VolumeEval : public Evaluator {
			virtual double evaluate(MeshElementNode & mes) const { 
				double v = mes.volumePD( );
				return v;
			}
			virtual const char * const label( ) const {
				return "volume";
			}
			virtual const char * const checkFunction( ) const {
				return  "polyarea";
			}
		};

		void meshElementPlot(const Evaluator & ev, MovingBoundaryParabolicProblemImpl & impl, std::ostream &os) {
			char colors[] = {'y','m','c','g','b','k'};	
			vcell_util::Modulo<int,int> cIndex(0,sizeof(colors)/sizeof(colors[0]));

			MovingBoundaryParabolicProblemImpl::MBMesh::iterator iter = impl.primaryMesh.begin( );
			matlabBridge::Patch patch(ev.label( ), ev.checkFunction( ), "1e-3");
			matlabBridge::Scatter scatter('g',2,true);
			matlabBridge::Scatter empty('r',2,true);
			int seqNo = 1; //used to cause matlab variables to change
			for (;iter != impl.primaryMesh.end( ); ++iter, ++seqNo) {
				MeshElementNode & mes = *iter;
				/*
				if (mes.indexOf(cX) != 5) 
				continue;
				if (mes.indexOf(cY) != 11 && mes.indexOf(cY) != 11 ) {
				continue;
				}
				*/

				std::stringstream ss; ss << '[' << mes.indexOf(cX) << ',' << mes.indexOf(cY) << ']';
				std::string spec("-");
				spec += colors[cIndex];
				++cIndex;
				matlabBridge::Polygon boundingPoly("k",2, seqNo);
				const Volume2DClass & vol = mes.getControlVolume();
				if (vol.empty( )) {
					empty.add(mes(cX),mes(cY));
					ss << "empty";
					os << matlabBridge::ConsoleMessage(ss.str( ).c_str( )); 
					continue;
				}
				double c =  ev.evaluate(mes);
				Volume2DClass::VectorOfVectors vOfV = vol.points( );
				const bool checkVal = vOfV.size( ) == 1;
				patch.checkControl( ) = ev.supportsCheck( ) && checkVal;
				for (Volume2DClass::VectorOfVectors::const_iterator vvIter = vOfV.begin( ); vvIter != vOfV.end( );++vvIter) {
					matlabBridge::Polygon pPoly("k",3);
					patch.specifyValueAndClear(c, seqNo);
					frontTierAdapt::copyVectorInto(pPoly,*vvIter);
					frontTierAdapt::copyVectorInto(patch,*vvIter);
					os << patch << pPoly;
				}
				scatter.add(mes(cX),mes(cY));
				os << matlabBridge::ConsoleMessage(ss.str( ).c_str( )); 
			}
			patch.setScale(os);
			os << scatter << empty;
		}
	}

	//**************************************************
	// MovingBoundary proper
	//**************************************************

	MovingBoundaryParabolicProblem::MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs)
		:sImpl(std::make_shared<MovingBoundaryParabolicProblemImpl>(mbs) ) {}
	//:impl(new MovingBoundaryParabolicProblemImpl(mbs)) { }

	MovingBoundaryParabolicProblem::MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs, std::istream &is) 
		:sImpl(std::make_shared<MovingBoundaryParabolicProblemImpl>(mbs,is) ) {}
	//:impl(new MovingBoundaryParabolicProblemImpl(mbs,is)) { }

	const spatial::MeshDef<moving_boundary::CoordinateType,2> & MovingBoundaryParabolicProblem::meshDef( ) const {
		return sImpl->meshDef( );
	}
	const spatial::Mesh<moving_boundary::CoordinateType,2, MeshElementNode> & MovingBoundaryParabolicProblem::mesh( ) const {
		return sImpl->primaryMesh;
	}
	const Physiology* MovingBoundaryParabolicProblem::physiology( ) const {
		return sImpl->physiology;
	}
	double MovingBoundaryParabolicProblem::frontTimeStep( ) const {
		return sImpl->frontTimeStep;
	}
	double MovingBoundaryParabolicProblem::meshInterval() const {
		return sImpl->minimimMeshInterval;
	}
	double MovingBoundaryParabolicProblem::solverTimeStep( ) const {
		return sImpl->frontTimeStep;
	}
	unsigned int MovingBoundaryParabolicProblem::numberTimeSteps( ) const {
		return sImpl->numberTimeSteps( );
	}

	MovingBoundaryParabolicProblem::~MovingBoundaryParabolicProblem( ) {
	}

	void MovingBoundaryParabolicProblem::reportProgress(unsigned char percent, bool estimateTime) {
		if (percent == 0 || percent > 99) {
			unsigned int p = percent;
			VCELL_EXCEPTION(domain_error, "percent value " << p << " must be between 1 and 99");
		}
		sImpl->reportProgress(percent,estimateTime);
	}

	void MovingBoundaryParabolicProblem::add(moving_boundary::MovingBoundaryTimeClient & client) {
		sImpl->add(client);
	}
	void MovingBoundaryParabolicProblem::add(moving_boundary::MovingBoundaryElementClient & client) {
		sImpl->add(client);
	}
	std::string MovingBoundaryParabolicProblem::getOutputFiles() {
		return sImpl->getOutputFiles();
	}
	void MovingBoundaryParabolicProblem::run( ) {
		sImpl->run( );
	}
	bool MovingBoundaryParabolicProblem::noReaction( ) const {
		return sImpl->noReaction( );
	}

	void MovingBoundaryParabolicProblem::plotPolygons(std::ostream &os)  const {
		meshElementPlot(Concentration( ),*sImpl,os);
	}
	void MovingBoundaryParabolicProblem::plotAreas(std::ostream &os) const {
		meshElementPlot(VolumeEval( ),*sImpl,os);
	}
	double MovingBoundaryParabolicProblem::endTime( ) const {
		return sImpl->maxTime;
	}
	const MovingBoundarySetup &MovingBoundaryParabolicProblem::setup( ) const {
		return sImpl->setup( );
	}
	std::string MovingBoundaryParabolicProblem::frontDescription( ) const {
		return sImpl->vcFront->describe( );
	}

	void MovingBoundaryParabolicProblem::persist(std::ostream &os) const {
		sImpl->persist(os);
	}

	void MovingBoundaryParabolicProblem::registerType( ) {
		MovingBoundaryParabolicProblemImpl::registerType( );
	}

	void MovingBoundaryParabolicProblem::registerInstanceType( ) const {
		sImpl->registerInstanceType( );
	}
}
