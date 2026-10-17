#ifndef VCellChrono_h
#define VCellChrono_h
/* Copyright (C) 2014 UConn Health 
*
* Licensed under the MIT License (the "License").
* You may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
*  http://www.opensource.org/licenses/mit-license.php
*/
#include <chrono>
#include <iomanip>
namespace vcell_util {

	/**
	* bit mapped values
	*/
	namespace HoursMinutesSecondsFormat {
		/**
		* compact as possible
		*/
		static const int COMPACT = 0;
		/**
		* all (include hours and minutes if 0) 
		*/
		static const int ALL = 1;
		/**
		* zero pad so all the same
		*/
		static const int FIXED = 2; 
		/**
		* include short units (e.g. H:M:S)
		*/
		static const int SHORT_UNITS = 4;  
		/**
		* include long units (e.g. hours minutes seconds)
		* overrides SHORT_UNITS
		*/
		static const int LONG_UNITS = 8;  
	};


	/**
	* @tparam D std::chrono::duration
	*/
	template <class D, int F = HoursMinutesSecondsFormat::COMPACT>
	struct HoursMinutesSeconds {
		HoursMinutesSeconds(const D &d)
			:period(d){}

		/**
		* write value to output; note stream operator is provided for convenience
		* @param os destination
		*/
		void writeValue(std::ostream &os) const {
			using namespace std::chrono;
			seconds copy = duration_cast<seconds>(period);
			hours h = duration_cast<hours>(copy);
			copy -= h;
			minutes m = duration_cast<minutes>(copy);
			copy -= m;
			int hv = h.count( );
			int mv = m.count( );
			const bool all  = isSet(HoursMinutesSecondsFormat::ALL);
			if (all || hv > 0) {
				setformat(os)<< hv << ':'; 
			}
			if (all || mv > 0) {
				setformat(os)<< mv << ':'; 
			}
			setformat(os)<< copy.count( ); 
			if (isSet(HoursMinutesSecondsFormat::SHORT_UNITS)|isSet(HoursMinutesSecondsFormat::LONG_UNITS))  {
				os.put(' ');
				streamUnits(os,all,hv,mv);
			}
		}

		/**
		* output units
		* @param os destination
		*/
		void writeUnits(std::ostream &os) const {
			using namespace std::chrono;
			const bool all  = isSet(HoursMinutesSecondsFormat::ALL);
			streamUnits(os,all,duration_cast<hours>(period).count( ),duration_cast<minutes>(period).count( ));
		}

	private:
		/**
		* common implementation
		* @param os destination
		* @param all is ALL flag set?
		*/
		void streamUnits(std::ostream &os, bool all, int hours, int minutes) const {
			if (isSet(HoursMinutesSecondsFormat::LONG_UNITS))  {
				if ( all ||  hours > 0)  {
					os << "hours:";
				}
				if ( all ||  minutes > 0)  {
					os << "minutes:";
				}
				os << "seconds";
			}
			if (isSet(HoursMinutesSecondsFormat::SHORT_UNITS))  {
				if ( all ||  hours > 0)  {
					os << "H:";
				}
				if ( all ||  minutes > 0)  {
					os << "M:";
				}
				os << "S";
			}
		}
		bool isSet(int bit) const {
			return (F&bit) == bit;
		}

		std::ostream &setformat(std::ostream &os) const {
			using HoursMinutesSecondsFormat::FIXED;
			if ((F&FIXED) == FIXED) {
				os.width(2);
				os.fill('0');
			}
			return os;
		}
		const D &period;

	};

	template <class D, int F>
	std::ostream & operator<<(std::ostream &os, const HoursMinutesSeconds<D,F> &hms) {
		hms.writeValue(os);
		return os;
	}

}
#endif
