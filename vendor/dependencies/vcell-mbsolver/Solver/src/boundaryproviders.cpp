#include <iomanip>
#include <boundaryProviders.h>
#include <MovingBoundaryCollections.h>
#include <vcellxml.h>
#include <World.h>
#include <persist.h>
#include <SExpression.h>
#include <SimpleSymbolTable.h>

namespace { 
	const double Pi = 3.14159265358979323846264338327950288419716939937;

	class FixedBoundary : public spatial::FrontProvider<moving_boundary::CoordinateType> {
		typedef spatial::TPoint<moving_boundary::CoordinateType,2> FixedBoundaryPoint;
		typedef moving_boundary::World<moving_boundary::CoordinateType,2> WorldType; 
	protected:
		const WorldType & world;
		/**
		* used for saving / restoring
		*/
		const double problemDomainVelocityx;
		/**
		* used in problem, scaled value
		*/
		const double xVel;
		double time;
		moving_boundary::FrontType baseFront;

		FixedBoundary(double xvelocity) 
			:world(WorldType::get( )),
			problemDomainVelocityx(xvelocity),
			xVel(xvelocity * world.theScale( ) ),
			time(0) { }


	public:

		virtual bool propagateTo(double time_) {
			time = time_;
			return true;
		}
		/**
		* get current front 
		*/
		virtual std::vector<FixedBoundaryPoint> retrieveFront( ) {
			std::vector<FixedBoundaryPoint> rval(baseFront.size( ));
			std::transform(baseFront.begin( ),baseFront.end( ),rval.begin( ),*this);
			return rval;
		}

		/**
		* make self a transform operator
		*/
		moving_boundary::FrontPointType operator( )(const moving_boundary::FrontPointType & in) {
			moving_boundary::FrontPointType rval(in);
			rval(spatial::cX) += static_cast<moving_boundary::CoordinateType>(xVel *time + 0.5);
			return rval;
		}

		std::string describe( ) const {
			std::ostringstream os;
			os << "Fixed boundary front with " << baseFront.size( ) << " points and x velocity " << xVel << std::ends;
			return os.str( ); 
		}
		void persist(std::ostream &os) const {
		}

		/**
		* "class" register type; disambiguate from virtual #registerType
		*/
		static void cRegisterType( ) {
			vcell_persist::Registrar::reg<FixedBoundary>("fixedBoundary");
			FixedBoundaryPoint::registerType( );
		}

		/**
		* implement virtual 
		*/
		void registerType( ) const {
			cRegisterType( );
		}

	protected:
		/**
		* close polygon
		*/
		void close( ) {
			baseFront.push_back(baseFront.front( ));
		}

	};

	struct Circle : public FixedBoundary {
		/**
		* @param originx
		* @param originy
		* @param radius
		* @param step (degrees to step to create circle)
		* @param velocityx 
		*/
		Circle(double originx, double originy, double radius, double step, double velocityx)
			:FixedBoundary(velocityx),
			x(originx),
			y(originy),
			r(radius),
			s(step)
		{
			for (double theta = 0; theta < 2 *Pi ; theta += step) {
				spatial::Point2D t(originx + radius * cos(theta), originy + radius * sin(theta) );
				moving_boundary::FrontPointType fP = world.toWorld(t);
				baseFront.push_back(fP);
			}
			close( );
		}

		static const std::string token( ) {
			return "circle";
		}

		std::string describe( ) const {
			std::ostringstream os;
			os << "Circle at " << x << ',' << y << " of radius " << r << " theta step " << std::setprecision(12) << s << " with " 
				<< baseFront.size( ) << " points and x velocity " << xVel << std::endl;
			return os.str( ); 
			return std::string( );
		}

		~Circle( ) {
		}

		static Circle *restore(std::istream &is) {
			//factory "Builder" has already checked token and type
			double originx;
			double originy;
			double radius; 
			double step; 
			double pdVelocityx; 
			double time;
			vcell_persist::binaryRead(is,time);
			vcell_persist::binaryRead(is,pdVelocityx);
			vcell_persist::binaryRead(is,originx);
			vcell_persist::binaryRead(is,originy);
			vcell_persist::binaryRead(is,radius);
			vcell_persist::binaryRead(is,step);
			//Circle constructor scales velocity, so set to zero and manually set after
			Circle * rval = new Circle(originx,originy,radius,step,pdVelocityx);
			rval->time = time;
			vcell_persist::restore(is,rval->baseFront);
			return rval;
		}

		void persist(std::ostream &os) const {
			registerType( );
			vcell_persist::Token::insert<FixedBoundary>(os);
			vcell_persist::StdString<>::save(os,token( ));
			vcell_persist::binaryWrite(os,time);
			vcell_persist::binaryWrite(os,problemDomainVelocityx);
			vcell_persist::binaryWrite(os,x);
			vcell_persist::binaryWrite(os,y);
			vcell_persist::binaryWrite(os,r);
			vcell_persist::binaryWrite(os,s);
			vcell_persist::save(os,baseFront);
		}
	private:
		const double x; 
		const double y; 
		const double r;
		const double s; 
	};


	struct Rectangle : public FixedBoundary {
		Rectangle(double originx, double originy, double width, double height, double velocityx)
			:FixedBoundary(velocityx),
			x(originx),
			y(originy),
			w(width),
			h(height) 
		{
			using spatial::cX;
			using spatial::cY;

			spatial::Point2D cursor(originx,originy); 
			baseFront.push_back( world.toWorld(cursor) );
			cursor(cX) += w;
			baseFront.push_back( world.toWorld(cursor) );
			cursor(cY) += h;
			baseFront.push_back( world.toWorld(cursor) );
			cursor(cX) -= w;
			baseFront.push_back( world.toWorld(cursor) );
			close( );
		}

		std::string describe( ) const {
			std::ostringstream os;
			os << "Rectangle corner at " << x << ',' << y << " width " << w << " height " << h 
				<< "and x velocity " << xVel << std::endl;
			return os.str( ); 
			return std::string( );
		}

		static const std::string token( ) {
			return "rectangle";
		}

		void persist(std::ostream &os) const {
			registerType( );
			vcell_persist::Token::insert<FixedBoundary>(os);
			vcell_persist::StdString<>::save(os,token( ));
			vcell_persist::binaryWrite(os,time);
			vcell_persist::binaryWrite(os,x);
			vcell_persist::binaryWrite(os,y);
			vcell_persist::binaryWrite(os,w);
			vcell_persist::binaryWrite(os,h);
		}
	private:
		const double x; 
		const double y; 
		const double w;
		const double h; 
	};

	class ExpandingCircle : public spatial::FrontProvider<moving_boundary::CoordinateType> {
		typedef spatial::TPoint<moving_boundary::CoordinateType,2> ExpandingCirclePoint;
		typedef moving_boundary::World<moving_boundary::CoordinateType,2> WorldType; 

		const WorldType & world;
		/**
		* used for saving / restoring
		*/
		//const double problemDomainVelocityx;
		/**
		* used in problem, scaled value
		*/
		//const double xVel;
		//double time;
		moving_boundary::FrontType baseFront;
		SimpleSymbolTable* symTable;
		moving_boundary::SExpression radiusExpression;
		double multiplier;
		std::array<double,3> values;
		const static size_t TIME_INDEX = 2;

	public:
		ExpandingCircle(double theta, const char * const expression) 
			:world(WorldType::get( )),
			symTable(buildSymTable( )),
			radiusExpression(expression,symTable),
			multiplier(1),
			values( ) {
				//estimate final vector size to make a little quicker
				size_t estimate = (2 * Pi)/theta + 2; //two to allow for closing point
				baseFront.reserve(estimate);

				double radius = radiusExpression.evaluate(values);
				for (double t = 0; t < 2 *Pi; t+=theta) {
					spatial::Point2D in(radius * sin(t), radius * cos(t));
					ExpandingCirclePoint pt = world.toWorld(in);
					baseFront.push_back(pt);
				}
				close( );
				assert(symTable->getEntry("t")->getIndex( ) == TIME_INDEX);
		}



		virtual bool propagateTo(double time_) {
			values[TIME_INDEX] = time_;
			multiplier = radiusExpression.evaluate(values);
			return true;
		}
		/**
		* get current front 
		*/
		virtual std::vector<ExpandingCirclePoint> retrieveFront( ) {
			std::vector<ExpandingCirclePoint> rval(baseFront.size( ));
			std::transform(baseFront.begin( ),baseFront.end( ),rval.begin( ),*this);
			return rval;
		}

		/**
		* make self a transform operator
		*/
		moving_boundary::FrontPointType operator( )(const moving_boundary::FrontPointType & in) {
			moving_boundary::FrontPointType rval(in(spatial::cX) * multiplier, in(spatial::cY) *  multiplier);
			return rval;
		}

		std::string describe( ) const {
			std::ostringstream os;
			os << "Expanding circle with " << baseFront.size( ) << " points and radius " << radiusExpression.infix( ); 
			return os.str( ); 
		}
		void persist(std::ostream &os) const {
			throw std::domain_error("expanding circle persistence not implemented yet");
		}

		/**
		* "class" register type; disambiguate from virtual #registerType
		*/
		static void cRegisterType( ) {
			vcell_persist::Registrar::reg<ExpandingCircle>("expandingCircle");
			ExpandingCirclePoint::registerType( );
		}

		/**
		* implement virtual 
		*/
		void registerType( ) const {
			cRegisterType( );
		}

	protected:
		/*
		* constructor support; build SymbolTable 
		*/
		static SimpleSymbolTable* buildSymTable( ) {
			std::string syms[] = { "x","y","t"};
			SimpleSymbolTable* sst = new SimpleSymbolTable(syms, sizeof(syms)/sizeof(syms[0]));
			return sst;
		}


		/**
		* close polygon
		*/
		void close( ) {
			baseFront.push_back(baseFront.front( ));
		}


	};

	/**
	* base class / factory for alternate front providers
	*/
	struct Builder {
		Builder(Builder *sub) {
			builders[bIndex++] = sub;
		}
		virtual ~Builder( ) {}
		/**
		* return provider correct XML present
		* @return nullptr if top level node doesn't match
		*/
		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * build(const tinyxml2::XMLElement &node) = 0; 
		virtual std::string token( ) = 0; 
		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * restore(std::istream &is) = 0; 


		static spatial::FrontProvider<moving_boundary::CoordinateType> * construct( const tinyxml2::XMLElement &node) {
			for (size_t i = 0; i < builders.size( ) ; i++) {
				Builder * b = builders[i];
				if (b == nullptr) { //no match
					return nullptr;
				}
				spatial::FrontProvider<moving_boundary::CoordinateType> * fp = b->build(node); 
				if (fp != nullptr) {
					return fp;
				}
			}
			return nullptr;
		}

		static spatial::FrontProvider<moving_boundary::CoordinateType> * construct(std::istream &is) { 
			FixedBoundary::cRegisterType( );
			vcell_persist::Token::check<FixedBoundary>(is);
			std::string type = vcell_persist::StdString<>::restore(is);
			for (size_t i = 0; i < builders.size( ) ; i++) {
				Builder * b = builders[i];
				if (b == nullptr) { //no match
					break;
				}
				if (type == b->token( )) {
					return b->restore(is);
				}
			}
			VCELL_EXCEPTION(invalid_argument,"Invalid FixedBoundary restoration key " << type);
		}
	protected:
		static std::array<Builder*,10> builders;
		static size_t bIndex;
	};
	std::array<Builder*,10> Builder::builders;
	size_t Builder::bIndex;


	struct CircleBuild : public Builder {
		CircleBuild( )
			:Builder(this) {}
		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * build(const tinyxml2::XMLElement &root) {
			const tinyxml2::XMLElement *n = root.FirstChildElement(token( ).c_str( ));
			if (n != nullptr) {
				const tinyxml2::XMLElement &node = *n; 
				double theta = vcell_xml::convertChildElement<double>(node,"thetaIncrement");
				double vx = vcell_xml::convertChildElement<double>(node,"velocityx");

				double x = vcell_xml::convertChildElementWithDefault<double>(node,"originx",0);
				double y = vcell_xml::convertChildElementWithDefault<double>(node,"originy",0);
				double radius = vcell_xml::convertChildElementWithDefault<double>(node,"radius",1);
				return new Circle(x,y,radius,theta,vx);
			}
			return nullptr;
		}
		virtual std::string token( ) {
			return Circle::token( );
		}
		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * restore(std::istream &is) {
			return Circle::restore(is);
		}
	} Cb;

	struct RectangleBuild : public Builder {
		RectangleBuild( )
			:Builder(this) {}
		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * build(const tinyxml2::XMLElement &root) {
			const tinyxml2::XMLElement *n = root.FirstChildElement(token( ).c_str( ));
			if (n != nullptr) {
				const tinyxml2::XMLElement &node = *n; 
				double width = vcell_xml::convertChildElement<double>(node,"width");
				double height = vcell_xml::convertChildElement<double>(node,"height");
				double vx = vcell_xml::convertChildElement<double>(node,"velocityx");

				double x = vcell_xml::convertChildElementWithDefault<double>(node,"originx",0);
				double y = vcell_xml::convertChildElementWithDefault<double>(node,"originy",0);
				return new Rectangle(x,y,width,height,vx); 
			}
			return nullptr;
		}

		virtual std::string token( ) {
			return Rectangle::token( );
		}

		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * restore(std::istream &is) {
			return 0;
		}
	} Rb;

	struct ExpandingCircleBuild : public Builder {
		ExpandingCircleBuild( )
			:Builder(this) {}
		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * build(const tinyxml2::XMLElement &root) {
			const tinyxml2::XMLElement *n = root.FirstChildElement(token( ).c_str( ));
			if (n != nullptr) {
				const tinyxml2::XMLElement &node = *n; 
				double theta = vcell_xml::convertChildElement<double>(node,"theta");
				const char * exp  = vcell_xml::convertChildElement<const char *>(node,"radiusExpression");
				return new ExpandingCircle(theta,exp); 
			}
			return nullptr;
		}
		virtual std::string token( ) {
			return "expandingCircle"; 
		}

		virtual spatial::FrontProvider<moving_boundary::CoordinateType> * restore(std::istream &is) {
			return 0;
		}
	} Ecb;

}

/**
* @param originx
* @param originy
* @param radius
* @param step (degrees to step to create circle)
* @param velocityx 
*/
spatial::FrontProvider<moving_boundary::CoordinateType> * moving_boundary::circleFront(double originx, double originy, double radius, double step, double velocityx) {
	return new Circle(originx,originy,radius,step,velocityx);
}
spatial::FrontProvider<moving_boundary::CoordinateType> *moving_boundary::expandingCircle(double theta, const char * const expression) {
	return new ExpandingCircle(theta,expression); 
}


spatial::FrontProvider<moving_boundary::CoordinateType> * moving_boundary::frontFromXML(const tinyxml2::XMLElement &node) {
	spatial::FrontProvider<moving_boundary::CoordinateType> * fp = Builder::construct(node); 
	if (fp != nullptr) {
		return fp;
	}
	VCELL_EXCEPTION(invalid_argument,"Invalid XML for alternate front");
}

spatial::FrontProvider<moving_boundary::CoordinateType> * moving_boundary::restoreFrontProvider(std::istream &is) {
	return Builder::construct(is); 
}
