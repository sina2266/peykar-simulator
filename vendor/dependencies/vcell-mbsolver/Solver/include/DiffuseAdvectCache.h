#ifndef DiffuseAdvectCache_h
#define DiffuseAdvectCache_h

namespace spatial {
	/**
	* #MeshElementNode needs place to check forward and back diffusion
	* This is interface class to facilitate this without introducing excessive 
	* compile dependencies
	*/
	struct DiffuseAdvectCache {
		virtual ~DiffuseAdvectCache( ) {} 
		/**
		* begin diffusion advection on all (inside) nodes in the mesh
		*/
		virtual void start( ) = 0;
		/**
		* diffusion advection on all (inside) nodes in the mesh complete
		*/
		virtual void finish( ) = 0;
	};
}
#endif
