#ifndef MBPolygon_h
#define MBPolygon_h
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <MBridge/MBMatlabGenerator.h>
#include <MBridge/FigureLimits.h>
namespace matlabBridge {
	//forward
	template <typename T> struct FigureLimitFinder; 

	template <class T>
	struct TPolygon :public MatlabGenerator {
		TPolygon(const char * const lineSpec_ , int lineWidth_ = 0, int sequenceNumber_ = 0) 
			:MatlabGenerator("plotPolygonMatrix", sequenceNumber_),
			points( ),
			lineSpec(lineSpec_),
			lineWidth(lineWidth_),
			precision(6)
		{}

		void add(T x, T y) {
			points.push_back(PPair(x,y));
		}

		void clear( ) {
			points.clear( );
		}

		void write(std::ostream &os) const {
			using std::endl;
			const std::string variableName = vName( );
			if (points.empty( )) {
				os << '%' << variableName << " empty" << std::endl;
				return;
			}
			os << "hold on;" << endl;
			os << "clear " << variableName << ';' << endl; 
			int i = 1;
			for (typename PVector::const_iterator iter = points.begin( ); iter != points.end( );++iter) {
				os << variableName << '(' << i   << ",1) = " << std::setprecision(precision) << iter->first  << ';' << endl; 
				os << variableName << '(' << i++ << ",2) = " << std::setprecision(precision) << iter->second << ';' << endl; 
			}
			os << "plot(" << variableName<< "(:,1)," << variableName << "(:,2),'"
				<< lineSpec << "'";
			if (lineWidth > 0) {
				os << ",'LineWidth'," << lineWidth;
			}

			os << ");"<< endl;
		}

		void close( ) {
			if (points.size( ) > 0) {
				if (points.front( ) != points.back( )) {
					points.push_back(points.front( ));
				}
				return;
			}
			throw std::domain_error("close( ):no points");
		}

		/**
		* set precision of numeric output (see std::setprecision)
		*/
		void setPrecision(int p) {
			precision = p;
		}

		FigureLimits<T> figureLimits( ) const {
			FigureLimitFinder<T> f;
			f = std::for_each(points.begin( ), points.end( ),f);
			return FigureLimits<T>(f.minX,f.minY,f.maxX,f.maxY);
		}

	private:
		typedef std::pair<T,T> PPair;
		typedef std::vector<PPair> PVector; 
		PVector points;
		const char * const lineSpec;
		const int lineWidth;
		int precision;
	};

	template <class T>
	struct TPolygons :public MatlabGenerator {
		TPolygons(const char * const lineSpec_ , int lineWidth_ = 0) 
			:MatlabGenerator("plotPolygonsMatrix", 0), //no sequence for now
			collection( ),
			points( ),
			lineSpec(lineSpec_),
			lineWidth(lineWidth_),
			precision(6),
			rainbow( )
			//polyAreaName( )
		{}

		void add(T x, T y) {
			points.push_back(PPair(x,y));
		}

		void nextPolygon( ) {
			collection.push_back(points);
			points.clear( );
		}

		void clear( ) {
			collection.clear( );
		}

		void write(std::ostream &os) const {
			using std::endl;
			int areaCount = 0;
			if (!points.empty( )) {
				const_cast<TPolygons<T> *>(this)->nextPolygon( );
			}
			const std::string variableName = vName( );
			if (collection.empty( )) {
				os << '%' << variableName << " collection empty" << std::endl;
				return;
			}
			os << "hold on;" << endl;
			int polygonCount = 0;
			const size_t nColors = rainbow.size( );
			for (typename std::vector<PVector>::const_iterator vIter = collection.begin( ); vIter != collection.end( );++vIter) {
				const PVector & current = *vIter;
				if (current.empty( )) {
					os << '%' << variableName << " element empty" << std::endl;
					continue;
				}
				os << "clear " << variableName << ';' << endl; 
				int i = 1;
				for (typename PVector::const_iterator iter = current.begin( ); iter != current.end( );++iter) {
					os << variableName << '(' << i   << ",1) = " << std::setprecision(precision) << iter->first  << ';' << endl; 
					os << variableName << '(' << i++ << ",2) = " << std::setprecision(precision) << iter->second << ';' << endl; 
				}
				os << "plot(" << variableName<< "(:,1)," << variableName << "(:,2),'"
					<< lineSpec;
				if (nColors > 0) { //if colors provided, use next one
					os << rainbow[polygonCount++];
					polygonCount %= nColors; //modulo nColors
				}

				os << "'";
				if (lineWidth > 0) {
					os << ",'LineWidth'," << lineWidth;
				}

				os << ");"<< endl;
				if (nColors> 0) {
					os << "% polygon number " <<polygonCount << std::endl;
				}
				if (polyAreaName.length( ) > 0) {
					os << polyAreaName << areaCount << " = polyarea(" << variableName<< "(:,1)," << variableName << "(:,2));" << std::endl;
				}
			}
		}

		/**
		* set precision of numeric output (see std::setprecision)
		*/
		void setPrecision(int p) {
			precision = p;
		}

		/**
		* set variable name so that command to calculate area of poly is generated in #write
		*/
		void setPolyAreaName(const char *n) {
			polyAreaName = n;
		}

		void setRainbow(const std::vector<char> & colors) {
			rainbow = colors;
		}

		FigureLimits<T> figureLimits( ) const {
			FigureLimitFinder<T> f;

			for (auto iter = collection.begin( ); iter != collection.end( ); ++iter) {
				f = std::for_each(iter->begin( ), iter->end( ),f);
			}
			return FigureLimits<T>(f.minX,f.minY,f.maxX,f.maxY);
		}

	private:
		typedef std::pair<T,T> PPair;
		typedef std::vector<PPair> PVector; 
		/**
		* total collection of polygons
		*/
		std::vector<PVector> collection;
		/**
		* current polygon
		*/
		PVector points;
		const char * const lineSpec;
		const int lineWidth;
		int precision;
		std::string polyAreaName;
		std::vector<char> rainbow; 
	};

	template <typename T>
	struct FigureLimitFinder {
		FigureLimitFinder( )
			:minX(0),
			minY(0),
			maxX(0),
			maxY(0) {}
		void operator( )(const std::pair<T,T> &pp) {
			minX = std::min(minX,pp.first);
			minY = std::min(minY,pp.second);
			maxX = std::max(maxX,pp.first);
			maxY = std::max(maxY,pp.second);
		}

		T minX;
		T minY;
		T maxX;
		T maxY;
	};



	struct Polygon : public TPolygon<double> {
		Polygon(const char * const lineSpec_ , int lineWidth_ = 0, int sequenceNumber_ = 0) 
			:TPolygon<double>(lineSpec_,lineWidth_,sequenceNumber_) {}
	};

	struct Polygons : public TPolygons<double> {
		Polygons(const char * const lineSpec_ , int lineWidth_ = 0)
			:TPolygons<double>(lineSpec_,lineWidth_) {}
	};



	template <class T>
	inline std::ostream & operator<<(std::ostream & in, const TPolygon<T> & pgon) {
		pgon.write(in);
		return in;
	}

	template <class T>
	inline std::ostream & operator<<(std::ostream & in, const TPolygons<T> & pgon) {
		pgon.write(in);
		return in;
	}


}
#endif
