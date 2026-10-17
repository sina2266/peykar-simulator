#include <CoordVect.h>
#include <IndexVect.h>
#include <math.h>
#include <istream>
#include <limits>
#include <World.h>
#include <MeshElementNode.h>
#include <MovingBoundaryTypes.h>

const CoordVect CoordVect::Unit(1.0, 1.0);
const CoordVect CoordVect::Zero(0.0, 0.0);

CoordVect::CoordVect ()
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] = 0.0;
	}
}

CoordVect::CoordVect (double i, double j)
{
	vect[0] = i;
	vect[1] = j;
}

CoordVect::CoordVect (const double* v)
{
	vect[0] = v[0];
	vect[1] = v[1];
}

CoordVect::CoordVect (const int* v)
{
	vect[0] = v[0];
	vect[1] = v[1];
}

CoordVect::CoordVect(const std::array<double, DIM>& p)
{
	vect[0] = p[0];
	vect[1] = p[1];
}

CoordVect::CoordVect(const moving_boundary::MeshElementNode& me)
{
	double worldValues[DIM] = {(double)me(spatial::cX), (double)me(spatial::cY)};
	moving_boundary::World<moving_boundary::CoordinateType, 2>::get().toProblemDomain(worldValues, vect);
}

CoordVect::CoordVect (const CoordVect &iv)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] = iv.vect[i];
	}
}

CoordVect& CoordVect::operator= (const CoordVect &iv)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] = iv.vect[i];
	}
	return *this;
}

const double& CoordVect::operator[] (int i) const
{
  return vect[i];
}

double& CoordVect::operator[] (int i)
{
  return vect[i];
}

CoordVect CoordVect::operator+ (const CoordVect& p) const
{
	CoordVect v(vect[0] + p[0], vect[1] + p[1]);
  return v;
}

CoordVect CoordVect::operator+(const std::array<double, DIM>& p) const
{
	CoordVect v(vect[0] + p[0], vect[1] + p[1]);
	return v;
}

CoordVect& CoordVect::operator+= (const CoordVect& p)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] += p.vect[i];
	}
  return *this;
}

CoordVect CoordVect::operator- (const CoordVect& p) const
{
	CoordVect v(vect[0] - p[0], vect[1] - p[1]);
  return v;
}

inline CoordVect& CoordVect::operator-= (const CoordVect& p)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] -= p.vect[i];
	}
  return *this;
}

CoordVect CoordVect::operator* (double s) const
{
  CoordVect v(vect[0] * s, vect[1] * s);
  return v;
}

CoordVect& CoordVect::operator*= (double s)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] *= s;
	}
	return *this;
}

CoordVect CoordVect::operator/ (double s) const
{
  CoordVect v(vect[0] / s, vect[1] / s);
  return v ;
}

CoordVect& CoordVect::operator/= (double s)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] /= s;
	}
	return *this;
}

CoordVect CoordVect::operator/ (const CoordVect& p) const
{
	CoordVect v(vect[0] / p.vect[0], vect[1] / p.vect[1]);
	return v;
}

CoordVect CoordVect::operator/ (const IndexVect& p) const
{
	CoordVect v(vect[0] / p[0], vect[1] / p[1]);
	return v;
}

CoordVect CoordVect::normalize () const
{
	double len = length();
	return (*this)/len;
}

double CoordVect::length() const
{
	return sqrt(vect[0] * vect[0] + vect[1] * vect[1]);
}

double CoordVect::distance2(CoordVect& p) const
{
	return (vect[0] - p.vect[0]) * (vect[0] - p.vect[0])
			+ (vect[1] - p.vect[1]) * (vect[1] - p.vect[1]);
}

bool CoordVect::withinWorld() const
{
	const std::array<spatial::TGeoLimit<moving_boundary::CoordinateType>, 2>& limits = moving_boundary::World<moving_boundary::CoordinateType, DIM>::get( ).limits();
	for (int idir = 0; idir < DIM; ++ idir)
	{
		if (vect[idir] < limits[idir].low() || vect[idir] > limits[idir].high() )
		{
			return false;
		}
	}
	return true;
}

CoordVect CoordVect::toProblemDomain() const
{
	moving_boundary::World<moving_boundary::CoordinateType, DIM>& world = moving_boundary::World<moving_boundary::CoordinateType, DIM>::get( );
	return world.toProblemDomain(*this);
}

double CoordVect::min() const
{
	double d = std::numeric_limits<double>::max();
	for (int i = 0; i < DIM; ++ i)
	{
		d = std::min(d, vect[i]);
	}
	return d;
}

double CoordVect::max() const
{
	double d = std::numeric_limits<double>::min();
	for (int i = 0; i < DIM; ++ i)
	{
		d = std::max(d, vect[i]);
	}
	return d;
}

std::ostream& operator<< (std::ostream& ostr, const CoordVect& p)
{
  ostr << "[" << p[0] << " " << p[1] << "]" ;
  return ostr;
}
