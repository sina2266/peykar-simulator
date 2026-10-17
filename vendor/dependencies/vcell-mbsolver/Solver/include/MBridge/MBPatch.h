#ifndef MBPatch_h
#define MBPatch_h
#include <limits>
#include <vector>
#include <iostream>
#include <MBridge/MBMatlabGenerator.h>
namespace matlabBridge {

	/**
	* MATLAB patch command
	*/
	struct Patch :public MatlabGenerator{
		Patch(const char * const label_ = 0,const char * const checkfunction = 0,
				const char * const tolerance_ = "1e-3") 
			:MatlabGenerator("patchMatrix"),
			value(0),
			points( ),
			lo(std::numeric_limits<double>::max( )),
			high(std::numeric_limits<double>::min( )),
			label(label_ == 0 ? "value" : label_),
			checkFunc(checkfunction == 0 ? "" : checkfunction),
			tolerance(tolerance_ == 0 ? "1e-5" : tolerance_),
			check(checkfunction != 0)

		{}

		bool &checkControl( ) {
			return check;
		}

		/**
		* add vertex of bounding polygon
		*/
		void add(double x, double y) {
			points.push_back(DataPoint(x,y));
		}

		/**
		* specify value, reset polygon vertices
		*/
		void specifyValueAndClear(double newValue, int sequenceNumber_ = 0) {
			points.clear( );
			value = newValue;
			lo = std::min(lo, value);
			high = std::max(high, value);
			sequenceNumber  = sequenceNumber_;
		}

		/**
		* write patch command
		* note plot scale is <b>not</b> set automatically
		* @param os stream to write to
		*/
		void write(std::ostream &os) const {
			using std::endl;
			using std::string;
			string matrixName(vName( ));

			os << "hold on;" << endl;
			os << "clear " << matrixName << ';' << endl; 
			int i = 1;
			for (DataVector::const_iterator iter = points.begin( ); iter != points.end( );++iter, ++i) {
				const DataPoint & dp = *iter;

				os << matrixName << '(' << i << ", 1) = " << dp.x <<  ';' << endl; 
				os << matrixName << '(' << i << ", 2) = " << dp.y <<  ';' << endl; 
			}
			os << "patch(" << matrixName<< "(:,1)," << matrixName << "(:,2)," << value << ",'CDataMapping','scaled');" << endl;
			if (check) {
				os << matrixName << label << " = " << value << ';' << endl;
				std::string checkName(matrixName);
				checkName.append("check");
				os << checkName << " = "  << checkFunc << '(' << matrixName<< "(:,1)," << matrixName << "(:,2));" << endl;
				os << "assert( abs(" <<  checkName << " - " << value << ") < " << tolerance << ");" << endl;
			}
		}

		/**
		* set scale based on vertices which have been set 
		* @param os stream to write to
		*/
		void setScale(std::ostream &os) {
			os << "caxis( [" << lo << ' ' << high << "]);";
		}

	private:
		
		struct DataPoint {
			double x;
			double y;
			DataPoint(double x_ , double y_)
				:x(x_),
				y(y_)
			{}
		};
		typedef std::vector<DataPoint> DataVector; 

		double value;
		DataVector points;
		double lo;
		double high;
		std::string label;
		std::string checkFunc; 
		std::string tolerance; 
		bool check;
	};

	inline std::ostream & operator<<(std::ostream & in, const Patch & patch) {
		patch.write(in);
		return in;
	}
}

#endif
