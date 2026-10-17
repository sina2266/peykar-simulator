#include <MPoint.h>
#include <map>
#include <functional>
#include <ExplicitSolver.h>
#include <MeshElementNode.h>
#include <Logger.h>
#include <Separator.h>
using moving_boundary::ExplicitSolver;
using moving_boundary::CoordinateType;
using moving_boundary::BioQuanType;
using moving_boundary::TimeType;
using moving_boundary::MeshElementNode;
using moving_boundary::CoordinateProductType;

using spatial::MeshPosition;
namespace {
	//#define DVERSION
#ifdef DVERSION
	struct CoeffData {
		MeshPosition first;
		/*
		* j coefficient
		*/
		BioQuanType second;
		BioQuanType iComponent; 
		CoeffData(const MeshPosition & m, BioQuanType j, BioQuanType i = 0)
			:first(m),
			second(j),
			iComponent(i) {}
	};
#else
	typedef std::pair<MeshPosition,BioQuanType> CoeffData;  
#endif
	//use local for devel, refactor into class later
	TimeType timeStep;
	std::vector<CoeffData> cData;
	std::vector<BioQuanType> resultStore;
	typedef unsigned int MatrixIndex;
	std::map<MatrixIndex, std::map<MatrixIndex, BioQuanType> > matrix;
	std::vector<BioQuanType> rhs;
	size_t fileCount = 0;


	struct DocumentMapping {
		typedef spatial::Mesh<moving_boundary::CoordinateType,2,MeshElementNode> Mesh; 
		DocumentMapping(const Mesh & mesh_, std::ostream & os_)
			:mesh(mesh_),
			os(os_) {}

		void operator( )(MeshElementNode & e) {
			os << e.indexInfo( ) << ':' << mesh.indexOf(e.indexes()).to<int>( ) << std::endl;
		}

		const Mesh & mesh;
		std::ostream & os;
	};

}

ExplicitSolver::ExplicitSolver(Mesh &m)
	:mesh(m),
	sIdx(0)
{
	resultStore.resize(m.numCells( ));
	rhs.resize(m.numCells( ));
	//std::ofstream mf("map.txt");
	//std::for_each(mesh.begin( ),mesh.end( ),DocumentMapping(mesh,mf) );
}

void ExplicitSolver::setStepAndSpecies(TimeType t, unsigned int s) {
	timeStep = t;
	sIdx= s;
	cData.clear( );
	std::fill(rhs.begin( ), rhs.end( ), 0);
	std::fill(resultStore.begin( ), resultStore.end( ), 0);
	matrix.clear( );
	//set diagonals to 1
	int numCells = mesh.numCells( );
	for (MatrixIndex i = 0; i < numCells; ++i) {
		matrix[i][i] = 1;
	}
}
void ExplicitSolver::setCoefficent( const MeshElementNode &i, const MeshElementNode &j, BioQuanType coeff, BioQuanType iCoeff) {
	const BioQuanType concenCoeff = coeff  * timeStep;
	MeshPosition jp = mesh.indexOf(j.indexes( )); 
#ifdef DVERSION
	cData.push_back(CoeffData(jp,concenCoeff,iCoeff));
#else	
	cData.push_back(CoeffData(jp,concenCoeff));
#endif
	MeshPosition ip = mesh.indexOf(i.indexes( )); 
	matrix[ip.to<MatrixIndex>( )][jp.to<MatrixIndex>( )] = -1 * concenCoeff; 

}
#ifdef DVERSION
void ExplicitSolver::setSolvingFor(MeshElementNode &i, BioQuanType coeff) {
	VCELL_COND_LOG(info,sIdx == 0,i.ident( ) << " timeStep " << timeStep); 
	BioQuanType result = 0;
	//explicity find and calculate the j terms
	for (std::vector<CoeffData>::const_iterator  iter = cData.begin( ); iter != cData.end( ); ++iter) {
		MeshElementNode & node = mesh.get(iter->first);
		const BioQuanType jdelta =  node.priorConcentration(sIdx) * iter->second; 
		const BioQuanType idelta =  i.priorConcentration(sIdx) * iter->iComponent * timeStep; 


		const BioQuanType delta = jdelta - idelta; 

		result += delta; 
		VCELL_COND_LOG(info,sIdx == 0,i.ident( ) << " j " << node.ident( ) << " u " << node.priorConcentration(sIdx) 
			<< " cf " << iter->second << " * t product = " << jdelta  << " i u " << i.priorConcentration(sIdx) << " cf " 
			<< iter->iComponent << " * t product = " << idelta << " delta " << delta
			<< " result " <<result); 
	}
	//const BioQuanType delta =  i.mass(sIdx) * coeff * timeStep;
	//result += delta; 
	const BioQuanType u0 = i.concentration(sIdx); 
	const BioQuanType sum =  result + u0;

	MeshPosition p = mesh.indexOf(i.indexes( )); 
	resultStore[p.to<size_t>()]= sum; 
	VCELL_COND_LOG(info,sIdx == 0,i.ident( ) << " result " << result << " u0 " << u0 << " = " << sum << " in slot " << p.to<size_t>( )); 
	cData.clear( );
}
#else
void ExplicitSolver::setSolvingFor(MeshElementNode &i, BioQuanType coeff, BioQuanType rhsValue) {
	VCELL_COND_LOG(info,sIdx == 0,i.ident( ) << " timeStep " << timeStep); 
	BioQuanType result = 0;
	//explicity find and calculate the j terms
	for (std::vector<CoeffData>::const_iterator  iter = cData.begin( ); iter != cData.end( ); ++iter) {
		MeshElementNode & node = mesh.get(iter->first);
		const BioQuanType jdelta =  node.priorConcentration(sIdx) * iter->second; 
		result += jdelta; 
		VCELL_COND_LOG(info,sIdx == 0,i.ident( ) << " j " << node.ident( ) << " u " << node.priorConcentration(sIdx) 
			<< " * cf " << iter->second << " = " << jdelta  
			<< " result " <<result); 
	}
	const BioQuanType  concenCoeff  =  coeff * timeStep; 
	const BioQuanType idelta =  i.priorConcentration(sIdx) * concenCoeff; 
	result -= idelta; 
	const BioQuanType sum =  result + rhsValue;
	MeshPosition p = mesh.indexOf(i.indexes( )); 
	const MatrixIndex mi = p.to<MatrixIndex>( );
	CoordinateProductType adjust = i.volumePD( )  / i.getLastVolume( );
	//matrix[mi][mi] = ( 1 - concenCoeff ) * i.volumePD( )  / i.getLastVolume( );
	matrix[mi][mi] = ( 1 - concenCoeff ); 
	rhs[mi] = rhsValue;
	VCELL_COND_LOG(info,sIdx == 0, i.ident( ) << " adjust " << adjust); 

	resultStore[p.to<size_t>()]= sum;
	VCELL_COND_LOG(info,sIdx == 0,i.ident( ) << " i mass " << i.mass(sIdx)  << " cf " << concenCoeff << " ="
		<< idelta << " result " <<result << " rhs " << rhsValue << " = " << sum << " in slot " << p.to<size_t>( )); 
	cData.clear( );
}
#endif

void ExplicitSolver::solve( ) {
	std::for_each(mesh.begin( ),mesh.end( ),SetConc(*this));
	#ifdef IMPLICIT_HOOK_TESTING
	{
		std::string mfilename("matrix");
		mfilename += std::to_string(fileCount);
		std::ofstream mfile(mfilename);
		vcell_util::Separator<char> sep;
		for (MatrixIndex row = 0; row < mesh.numCells( ); ++row) {
			for (MatrixIndex col = 0; col < mesh.numCells( ); ++col) {
				mfile << sep << matrix[row][col];
			}
			mfile << std::endl;
			sep.reset( );
		}
	}

	{
		vcell_util::Separator<char> sep;
		std::string vfilename("rhs");
		vfilename += std::to_string(fileCount);
		std::ofstream vfile(vfilename);
		for (size_t i = 0; i < mesh.numCells( ); ++i) {
			vfile << sep << rhs[i]; 
		}
		vfile << std::endl;
	}

	{
		vcell_util::Separator<char> sep;
		std::string sfilename("ans");
		sfilename += std::to_string(fileCount++);
		std::ofstream sfile(sfilename);
		for (size_t i = 0; i < mesh.numCells( ); ++i) {
			sfile << sep << resultStore[i]; 
		}
		sfile << std::endl;
	}
	#endif
}

void ExplicitSolver::setConcentration(MeshElementNode &e) {
	MeshPosition p = mesh.indexOf(e.indexes());
	if (e.isInside( )) {
		const size_t slot = p.to<size_t>();
		const BioQuanType u = resultStore[slot];

		e.setTransConcentration(sIdx,u);
		VCELL_COND_LOG(info,sIdx == 0,e.ident( ) << " conc " << u << " from slot " << p.to<size_t>( )); 
	}
}
