#ifndef Scatter_h
#define Scatter_h
#include <vector>
#include <iostream>
#include <MBridge/MBMatlabGenerator.h>
namespace matlabBridge {

	/**
	* scatter plot of points
	* @tparam T coordinate type of points
	*/
	template <typename T>
	struct TScatter : public MatlabGenerator {
		/**
		* @param color_ color of marker
		* @param markerSize_ size of circle
		* @param fill_ fill in the circle?
		* @param sequenceNumber_ necessary when more than one scatter plot in same matlab script
		*/
		TScatter(char color_, int markerSize_, bool fill_ = false, int sequenceNumber_= 0)
			:MatlabGenerator("scatterPlotMatrix", sequenceNumber_),
			points( ),
			color(color_),
			size(markerSize_),
			fill(fill_) {}

		void add(T x, T y) {
			points.push_back(PPair(x,y));
		}

		void write(std::ostream &os) const {
			using std::endl;
			const std::string variableName = vName( );
			if (points.size( ) > 0 ) {
				os << "hold on;" << endl;
				os << "clear " << variableName << ';' << endl; 
				int i = 1;
				for (typename PVector::const_iterator iter = points.begin( ); iter != points.end( );++iter) {
					os << variableName << '(' << i   << ",1) = " << iter->first  << ';' << endl; 
					os << variableName << '(' << i++ << ",2) = " << iter->second << ';' << endl; 
				}
				os << "scatter(" << variableName<< "(:,1)," << variableName << "(:,2),"
					<< size << ",'"<<color  << '\'';
				if (fill) {
					os << ", 'fill'";
				}

				os << ");"<< endl;
			}
			else {
				os << '%' << variableName << ' ' << size << ", " << color  << ( fill ? " fill" : " open" ) << " empty" << std::endl;
			}
		}

	private:
		typedef std::pair<T,T> PPair;
		typedef std::vector<PPair> PVector; 
		PVector points;
		char color;
		int size;
		bool fill;
	};

	template <typename T>
	inline std::ostream & operator<<(std::ostream & in, const TScatter<T> & scatter) {
		scatter.write(in);
		return in;
	}

	typedef TScatter<double> Scatter;

}
#endif
