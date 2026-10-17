#ifndef Angle_h
#define Angle_h
#include <cmath>
namespace spatial {
	const double pi = 3.14159265358979323846;

	inline const double degToRad(double deg) {
		return deg * pi/ 180.0;
	}
	inline const double radToDeg(double rad) {
		return rad / pi * 180.0;
	}

	/**
	* angle representation which is always positive and less
	* than 360 / 2Pi
	* not in use 5/2016
	*/
	struct Angle {
		double radians( ) const {
			return radValue;
		}
		double degrees( ) const {
			return radToDeg(radValue); 
		}
		double sin( ) const {
			return ::sin(radValue);
		}
		double cos( ) const {
			return ::cos(radValue);
		}
		double tan( ) const {
			return ::tan(radValue);
		}
		Angle & operator +=(const Angle & rhs) {
			radValue += rhs.radValue;
			adjust( );
			return *this;
		}

		Angle & operator -=(const Angle & rhs) {
			radValue -= rhs.radValue;
			adjust( );
			return *this;
		}

	protected:
		double radValue;
		Angle(double r)
			:radValue(r) {
				adjust( );
		}
		/**
		* ensure in rangle 0 to 2Pi
		*/
		void adjust( ) { 
			while (radValue < 0) {
				radValue += 2 *pi;
			}
			while (radValue > 2*pi) {
				radValue -= 2 *pi;
			}
		}
	};

	struct DegAngle : public Angle {
		DegAngle(double degrees = 0)
			:Angle(degToRad(degrees)) {}
		DegAngle &operator=(double deg) {
			radValue = degToRad(deg);
			return *this;
		}
	};

	struct RadAngle : public Angle {
		RadAngle(double radians = 0)
			:Angle(radians) {}
		RadAngle &operator=(double radians) {
			radValue = radians; 
			return *this;
		}
	};

	inline Angle operator+(const Angle &lhs, const Angle &rhs) {
		Angle a(lhs);
		a += rhs;
		return a;
	}
	inline Angle operator-(const Angle &lhs, const Angle &rhs) {
		Angle a(lhs);
		a -= rhs;
		return a;
	}

	inline bool operator==(const Angle &lhs, const Angle &rhs) {
		double d = fabs(lhs.radians( ) - rhs.radians( ));
		return d < 1e-10; //allow for roundoff error
	}
	/**
	* taking the inside angle, is lhs to the left of rhs?
	* that means rhs - lhs < halfCircle 
	* not the negation of > due to half circle case being ambiguous
	*/
	inline bool operator<(const Angle &lhs, const Angle &rhs) {
		Angle diff = rhs - lhs;
		return diff.radians( ) < pi;
	}
	/**
	* taking the inside angle, is lhs to the right of rhs?
	* that means lhs - rhs < halfCircle 
	* not the negation of < due to half circle case being ambiguous
	*/
	inline bool operator>(const Angle &lhs, const Angle &rhs) {
		Angle diff = rhs - rhs;
		return diff.radians( ) < pi;
	}
}
#endif
