#ifndef ExplicitSolver_h
#define ExplicitSolver_h
#include <MovingBoundaryTypes.h>
namespace spatial {
	template<class CT, int N, class TELEMENT> struct Mesh; 
}
namespace moving_boundary {
	struct MeshElementNode;
	struct ExplicitSolver {
		typedef spatial::Mesh<moving_boundary::CoordinateType,2,MeshElementNode> Mesh; 
		ExplicitSolver(Mesh &m);
		void begin( ) {}
		void setStepAndSpecies(TimeType t, unsigned int speciesIndex); 
		void setCoefficent(const MeshElementNode &i, const MeshElementNode &j, BioQuanType coeff, BioQuanType iDebug = 0);
		void setSolvingFor(MeshElementNode &i, BioQuanType coeff, BioQuanType rhs);
		void solve( );
	private:
		void setMass(MeshElementNode &) {}
		void setConcentration(MeshElementNode &);
		Mesh &mesh;
		unsigned int sIdx;
		struct SetConc {
			SetConc(ExplicitSolver &s)
				:solver(s) {}
			void operator( )(MeshElementNode & e) {
				solver.setConcentration(e);
			}
			ExplicitSolver &solver;
		};
	};
}
#endif
