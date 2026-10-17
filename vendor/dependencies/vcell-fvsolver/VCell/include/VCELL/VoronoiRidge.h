/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VORONOIRIDGE_H
#define VORONOIRIDGE_H

struct VoronoiRidge {
	bool bSymmetrized;
	double di;
	double si;

	friend ostream& operator<<(ostream& os, const VoronoiRidge& vr) {
		os << "[" << vr.bSymmetrized << "," << vr.di << "," << vr.si << "]";
		return os;
	}

	VoronoiRidge() {
		bSymmetrized = false;
		di = 0.0;
		si = 0.0;
	}

	VoronoiRidge(double arg_di, double arg_si) {
		bSymmetrized = false;
		di = arg_di;
		si = arg_si;
	}

	VoronoiRidge operator+(const VoronoiRidge& vr) {
		return VoronoiRidge(di + vr.di, si + vr.si);
	}

	VoronoiRidge average(const VoronoiRidge& vr) {
		return VoronoiRidge((di + vr.di)/2, (si + vr.si)/2);
	}

	VoronoiRidge half() {
		return VoronoiRidge(di/2, si/2);
	}	
};

#endif
