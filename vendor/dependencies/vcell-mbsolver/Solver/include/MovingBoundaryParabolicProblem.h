#ifndef MovingBoundaryParabolicProblem_h
#define MovingBoundaryParabolicProblem_h
#include <stdexcept>
#include <iostream>
#include <MovingBoundarySetup.h>
#include <MovingBoundaryTypes.h>

namespace spatial {
	template<class REAL, int N>
	struct MeshDef;
	template<class CT, int N, class TELEMENT>
	struct Mesh;
}
namespace moving_boundary {

	struct MovingBoundaryParabolicProblemImpl;
	namespace biology {
		struct Physiology;
	}

//	typedef double (*ConcentrationFunction)(double x, double y);

	class MovingBoundaryParabolicProblem { 
	public:
		/**
		* initialiation constructor
		*/
		explicit MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs);
		/**
		* default constructor to allow placeholder variables
		*/
		MovingBoundaryParabolicProblem( )
			:sImpl( ) {}
		/**
		* copy constructor -- XXXX transfers ownership impl from rhs to this
		* copy constructor -- shares pointer 
		*/
		MovingBoundaryParabolicProblem(const MovingBoundaryParabolicProblem & rhs) 
			:sImpl(rhs.sImpl) {
		}
		/**
		* stored instance constructor 
		*/
		MovingBoundaryParabolicProblem(const MovingBoundarySetup &mbs, std::istream &is);

		~MovingBoundaryParabolicProblem( );

		/**
		* assignment operator -- XXXX transfers ownership impl from rhs to this
		* shares pointer 
		*/
		MovingBoundaryParabolicProblem & operator=(const MovingBoundaryParabolicProblem & rhs) 
		{ 
			sImpl = rhs.sImpl;
			return *this;
		}

		/**
		* report progress report interval 
		* @param percent how often to report 1 - 100 
		* @param estimateTime ; include time estimate until completion 
		* default is no report 
		* @throws std::domain_error if percent not 1 to 99 
		*/
		void reportProgress(unsigned char percent, bool estimateTime = false);
		/**
		* add a client. Caller responsible for ensuring reference remains valid
		*/
		void add(MovingBoundaryTimeClient &client);
		/**
		* add a client. Caller responsible for ensuring reference remains valid
		*/
		void add(MovingBoundaryElementClient &client);
		/**
		 * return the output files of all report clients.
		 */
		std::string getOutputFiles();
		/**
		* run the simulation
		* @throws #TimeStepTooBig
		*/
		void run( );
		void plotPolygons(std::ostream &os) const ;
		void plotAreas(std::ostream &os) const;
		const spatial::MeshDef<moving_boundary::CoordinateType,2> & meshDef( ) const;
		const spatial::Mesh<moving_boundary::CoordinateType, 2, MeshElementNode>& mesh( ) const;
		const Physiology* physiology( ) const;
		/**
		* @return time step used for moving front
		*/
		double frontTimeStep( ) const;
		double meshInterval() const;

		/**
		* @return time step used by implicit solver
		*/
		double solverTimeStep( ) const;
		unsigned int numberTimeSteps( ) const; 
		double endTime( ) const;
		/**
		* setup used for problem
		*/
		const MovingBoundarySetup &setup( ) const;

		/**
		* register any polymorphic types present
		*/
		void registerInstanceType( ) const;

		static void registerType( );
		/**
		* save this. Note #MovingBoundarySetup must be saved separately by client
		*/
		void persist(std::ostream &os) const;

		std::string frontDescription( ) const;
		spatial::SVector<moving_boundary::VelocityType,2> velocity(spatial::TPoint<CoordinateType,2>& point) const;

		/**
		* return true there are no reactions (zero terms are zero)
		*/
		bool noReaction( ) const;

	private:
		//MovingBoundaryParabolicProblemImpl *impl;
		std::shared_ptr<MovingBoundaryParabolicProblemImpl> sImpl;
	};

	struct TimeStepTooBig : public std::invalid_argument {
		TimeStepTooBig(const char *what)
			:std::invalid_argument(what) {}
		TimeStepTooBig(const std::string & what)
			:std::invalid_argument(what) {}
	};

}
#endif
