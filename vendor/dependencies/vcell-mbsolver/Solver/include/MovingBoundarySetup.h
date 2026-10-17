#ifndef MovingBoundarySetup_h
#define MovingBoundarySetup_h
//#include <stdexcept>
//#include <iostream>
#include <MovingBoundaryTypes.h>
#include <VCellFront.h>
#include <VolumeVariable.h>
namespace tinyxml2 {
	class XMLElement;
}

#include <CoordVect.h>
#include <IndexVect.h>

namespace moving_boundary {
	struct MeshElementNode; 

	struct MovingBoundarySetup {
		IndexVect Nx;
		CoordVect extentX;
		CoordVect extentY;

		double frontToNodeRatio;
		REDISTRIBUTION_MODE redistributionMode;
		REDISTRIBUTION_VERSION redistributionVersion;
		unsigned int redistributionFrequency;
		EXTRAPOLATION_METHOD extrapolationMethod;
                
		double maxTime;
		/**
		* front time step increment
		*/
		std::string frontTimeStep;
		/**
		* explicit solver time step 
		*/
		std::string solverTimeStep;

		/**
		 * output time step
		 */
		std::string outputTimeStep;
		/**
		* if true, fail simluation if time set by #numberTimeSteps or #timeStep not numerically stable. If false, time
		* step may be adjusted
		*/
		bool hardTime;

		//double diffusionCoefficient;
		std::string levelFunctionStr;
		/**
		* velocity used for front; optional, default is #advectVelocityFunctionStrX 
		*/
		std::string frontVelocityFunctionStrX;
		/**
		* velocity used for front; optional, default is #advectVelocityFunctionStrY
		*/
		std::string frontVelocityFunctionStrY;

		Physiology* physiology;
		double diffusionConstant;

		MovingBoundarySetup( ) 
			:
			frontToNodeRatio(5),
			maxTime( ),
			frontTimeStep(),
			solverTimeStep(),
			hardTime(false),
			//diffusionCoefficient( ),
			levelFunctionStr( ),
			frontVelocityFunctionStrX( ),
			frontVelocityFunctionStrY( ),
			diffusionConstant(0)
		{}

		MovingBoundarySetup(const MovingBoundarySetup &rhs)
			:
			Nx(rhs.Nx),
			extentX(rhs.extentX),
			extentY(rhs.extentY),
			frontToNodeRatio(rhs.frontToNodeRatio),
			redistributionMode(rhs.redistributionMode),
			redistributionVersion(rhs.redistributionVersion),
			redistributionFrequency(rhs.redistributionFrequency),
			extrapolationMethod(rhs.extrapolationMethod),
			maxTime(rhs.maxTime),
			frontTimeStep(rhs.frontTimeStep),
			solverTimeStep(rhs.solverTimeStep),
			outputTimeStep(rhs.outputTimeStep),
			hardTime(rhs.hardTime),
			levelFunctionStr(rhs.levelFunctionStr),
			frontVelocityFunctionStrX(rhs.frontVelocityFunctionStrX),
			frontVelocityFunctionStrY(rhs.frontVelocityFunctionStrY),
			physiology(rhs.physiology),
			diffusionConstant(rhs.diffusionConstant)
                {
		}

	MovingBoundarySetup &operator= (const MovingBoundarySetup &rhs) {
		  Nx = rhs.Nx;
		  extentX = rhs.extentX;
		  extentY = rhs.extentY;
			frontToNodeRatio = rhs.frontToNodeRatio;
			redistributionMode = rhs.redistributionMode;
			redistributionVersion = rhs.redistributionVersion;
			redistributionFrequency = rhs.redistributionFrequency;
			extrapolationMethod = rhs.extrapolationMethod;
			maxTime = rhs.maxTime;
			frontTimeStep = rhs.frontTimeStep;
			solverTimeStep = rhs.solverTimeStep;
			outputTimeStep = rhs.outputTimeStep;
			hardTime = rhs.hardTime;
			levelFunctionStr = rhs.levelFunctionStr;
			frontVelocityFunctionStrX = rhs.frontVelocityFunctionStrX;
			frontVelocityFunctionStrY = rhs.frontVelocityFunctionStrY;
			physiology = rhs.physiology;
			diffusionConstant = rhs.diffusionConstant;
			return *this;
		}
		explicit MovingBoundarySetup(std::istream &is) ;
		~MovingBoundarySetup( );
		void persist(std::ostream &os) const;
		static void registerType( );

		/**
		* @param XMLElement containing "problem" XML child
		* @param lockUniverse lock universe from future changes -- settable to false for unit testing
		*/
		static MovingBoundarySetup setupProblem(const tinyxml2::XMLElement &root, int taskId, int paramNx);

	};

	/**
	* information about geometry of simulation at a given time
	*/
	template <class T>
	struct GeometryInfo {
		GeometryInfo(const std::vector<spatial::TPoint<T,2> > & boundary, bool adjusted_)
			:boundary(boundary),
			nodesAdjusted(adjusted_) {};

		/**
		* current control front
		*/
		const std::vector<spatial::TPoint<T,2> > & boundary;
		/**
		* have nodes changed since last time?
		*/
		const bool nodesAdjusted;
	};

	struct MovingBoundaryTimeClient {
		virtual ~MovingBoundaryTimeClient( ) {}
		/**
		 * return the name of the output if any.
		 */
		virtual std::string outputName( ) const = 0;
		/**
		* time of simulation
		* @param t current time
		* @param generationCount count of generations
		* @param last is this last time increment of sim?
		* @param geometryInfo current geometry information. Reference is not valid after return of function call.
		*/
		virtual void time(double t, unsigned int generationCount, bool last, const GeometryInfo<moving_boundary::CoordinateType> & geometryInfo) = 0; 
		/**
		* simulation has finished executing
		*/
		virtual void simulationComplete( ) = 0;
	};

	struct MovingBoundaryElementClient : public MovingBoundaryTimeClient {

		/**
		* state of inside / boundary nodes
		*/
		virtual void element(const MeshElementNode &e) = 0;
		/**
		* notify client they've received all elements
		*/
		virtual void iterationComplete( ) = 0;
	};
}

#endif
