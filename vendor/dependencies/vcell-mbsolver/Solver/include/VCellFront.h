#ifndef VCellFront_h
#define VCellFront_h
#include <vector>
#include <TPoint.h>
#include <World.h> //GeoLimit
#pragma warning ( disable: 4800 )
#include <boost/multi_array.hpp>
#include <VFrontier.h>

namespace moving_boundary
{
    class MovingBoundarySetup;
}
#include <sstream>
namespace spatial {

	typedef double (*FronTierLevelFunction)(POINTER, double *); 

	typedef int (*FronTierVelocityFunction)(POINTER,
		Front*,FT_POINT*,HYPER_SURF_ELEMENT*, HYPER_SURF*,double*);

	struct FronTierLevel {
		virtual double level(double *) const=0;
	};

	struct FronTierVelocity {
		virtual int velocity(Front*,FT_POINT*,HYPER_SURF_ELEMENT*, HYPER_SURF*,double*) const = 0;
	};

	template <typename T>
	struct FrontProvider {
		virtual ~FrontProvider( ) {}
		/**
		* propagate to specified time
		* @return false if problem
		*/
		virtual bool propagateTo(double time) = 0;
		/**
		* get current front 
		*/
		virtual std::vector<spatial::TPoint<T,2> > retrieveFront( ) = 0;
		/**
		* provide description of front
		*/
		virtual std::string describe( ) const = 0;
		/**
		* register persistent type
		*/
		virtual void registerType( ) const = 0; 
		/**
		* store
		*/
		virtual void persist(std::ostream &os) const = 0; 

	};

//	template <typename T>
//	struct FrontProviderCreator {
//		/**
//		* identification token
//		*/
//		virtual const std::string & token( ) const = 0;
//		virtual FrontProvider<T> * create(std::istream &is) const = 0;
//	};
//
//	template <typename T>
//	struct FrontProviderFactory {
//		static void identify(const FrontProviderCreator<T> &);
//		const FrontProviderCreator<T> &lookup(const std::string &);  
//	};

	/**
	* @tparam FCT front coordinate type
	*/
	template <typename FCT>
	class VCellFront : public FrontProvider<FCT> {
	public:
		VCellFront(std::vector<GeoLimit> & worldExtents, const moving_boundary::MovingBoundarySetup &mbs,
			FronTierLevelFunction levelFunction,
			FronTierVelocityFunction velocityFunction);

		VCellFront(std::vector<GeoLimit> & worldExtents, const moving_boundary::MovingBoundarySetup &mbs,
			const FronTierLevel & level,
			const FronTierVelocity &vel);

		/**
		* advance front to specified time
		* @return false if time <= current max_time
		*/
		bool propagateTo(double time);
		/**
		* advance front to specified time
		* @param csv stream to log intermediate outputs to
		* @return false if time <= current max_time
		*/
		bool propagateTo(double time, std::ostream  &csv);

		virtual std::string describe( ) const {
			return std::string("Frontier provided front");
		}

		std::vector<spatial::TPoint<FCT,2> > retrieveSurf( );

		virtual std::vector<spatial::TPoint<FCT,2> > retrieveFront( );
		/**
		* not implemented
		*/
		virtual void persist(std::ostream &os) const {} 
		/**
		* not implemented
		*/
		virtual void registerType( ) const {} 

		/**
		* retrieve front while logging points to csv file
		*/
		virtual std::vector<spatial::TPoint<FCT,2> > retrieveFront(std::ostream &csv );

		std::vector<std::vector<spatial::TPoint<FCT,2> > > retrieveCurves( );

		Front* const c_ptr( ) { return front; }

		void getPointNormal(FT_POINT* p, double* nor);

	private:
		typedef spatial::TPoint<FCT,2> VCFPointType; 
		void init(std::vector<GeoLimit> & worldExtents, const moving_boundary::MovingBoundarySetup &mbs,
			FronTierLevelFunction levelFunction,
			FronTierVelocityFunction velocityFunction,
			bool isAdapter);
		Front* front;
                
		const FronTierLevel * const levelObj;
		const FronTierVelocity * const velObj;
		std::array<GeoLimit, 2> domainLimits;
		static double levelAdapter(POINTER, double *); 

		static int velocityAdapter(POINTER,Front*,FT_POINT*,HYPER_SURF_ELEMENT*, HYPER_SURF*,double*);
	};
}
#endif
