#ifndef VoronoiMesh_h
#define VoronoiMesh_h
#include <MovingBoundaryCollections.h>
#include <Mesh.h>
#include <VoronoiResult.h>
namespace spatial {
	template <class CFrontType, typename CoordinateType> struct InsideCache;
}

namespace moving_boundary {

	struct MeshElementNode; 
	template<class MPOINT >
	struct Positions {
		std::vector<MPOINT *> inside;
		std::vector<MPOINT *> outside;
		std::vector<MPOINT *> boundary;
	}; 

	struct VoronoiMesh {
		//typedef REAL FloatingPointType;
		typedef spatial::MeshDef<moving_boundary::CoordinateType,2> MBMeshDef; 
		typedef MeshElementNode Element;
		typedef spatial::Mesh<moving_boundary::CoordinateType,2,Element> MBMesh; 
		typedef spatial::InsideCache<FrontType,moving_boundary::CoordinateType> VMInsideCache;

		explicit VoronoiMesh(MBMesh &m);
		VoronoiMesh();

		~VoronoiMesh( );

		/**
		* set the front to use
		*/
		void setFront(const FrontType & front);

		/**
		* get voronoi for specific element
		*/
		void getResult(spatial::VoronoiResult &, const Element & element) const;

		const MBMesh &mesh( ) const {
			return *mesh_;
		}

		/**
		* dump out current voronoi to stream 
		* @param os output stream
		* @param pFront pointer to front (optional, may be null)
		*/
		void matlabPlot(std::ostream &, const FrontType *pFront = nullptr);

		Positions<Element> classify2(const FrontType & polygon); 

		/**
		* adjust positions of nodes relative to front using last call to #setFront( ) 
		* @param boundaryContainer struct which supports push_back (e.g. STL)
		*/
		template<class STL_CONTAINER>
		bool adjustNodes(STL_CONTAINER & boundaryContainer);

		/**
		* set mesh for default constructed VoronoiMesh
		* @param m mesh to set
		* @throws if mesh already set 
		*/
		void setMesh(MBMesh &m);

	private:
		/**
		* not implemented
		*/
		VoronoiMesh(const VoronoiMesh &);
		/**
		* not implemented
		*/
		VoronoiMesh & operator=(const VoronoiMesh &);
		struct VoronoiMeshImpl;

		MBMesh *mesh_;
		VMInsideCache *insideCache;
		VoronoiMeshImpl * impl;
		
	};

}
#endif
