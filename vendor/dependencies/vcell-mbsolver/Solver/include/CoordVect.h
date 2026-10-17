#ifndef _COORDVECT_H_
#define _COORDVECT_H_

#include <array>
#include <ostream>
#include <MovingBoundaryTypes.h>
#include <IndexVect.h>
namespace moving_boundary
{
    struct MeshElementNode;
}

class CoordVect
{
public:
  static const CoordVect Zero;
  static const CoordVect Unit;

  CoordVect ();
  CoordVect (double i, double j);
  CoordVect (const CoordVect& rhs);
  CoordVect (const double* v);
  CoordVect (const int* v);
  CoordVect(const std::array<double,DIM>& p);
  CoordVect(const moving_boundary::MeshElementNode& me);

  CoordVect& operator= (const CoordVect& rhs);

  const double& operator[] (int i) const;
  double& operator[] (int i);

  CoordVect operator+ (const CoordVect& p) const;
  CoordVect operator+ (const std::array<double, DIM>& p) const;
  CoordVect& operator+= (const CoordVect& p);

  CoordVect operator- (const CoordVect& p) const;
  CoordVect& operator-= (const CoordVect& p);

  CoordVect operator* (double s) const;
  CoordVect& operator*= (double s);

  CoordVect operator/ (double s) const;
  CoordVect& operator/= (double s);
  CoordVect scale (double s) const;

  CoordVect operator/ (const CoordVect& s) const;
  CoordVect operator/ (const IndexVect& s) const;

  double length() const;
  double distance2(CoordVect& p) const;
  CoordVect normalize() const;

  friend std::ostream& operator<< (std::ostream& ostr,
                                   const CoordVect& p);
  const double* data() const
  {
  	return vect;
  }
  double min() const;
  double max() const;

  bool withinWorld() const;
  CoordVect toProblemDomain() const;

private:

  double vect[MAX_DIM];

};

#endif
