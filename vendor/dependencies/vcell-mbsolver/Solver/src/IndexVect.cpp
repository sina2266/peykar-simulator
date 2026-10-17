#include <IndexVect.h>
#include <istream>

const IndexVect IndexVect::Unit(1, 1);
const IndexVect IndexVect::Zero(0, 0);

IndexVect::IndexVect ()
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] = 0;
	}
}

IndexVect::IndexVect (int a, int b)
{
	vect[0] = a;
	vect[1] = b;
}

IndexVect::IndexVect (const IndexVect &iv)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] = iv.vect[i];
	}
}

IndexVect::IndexVect(std::array<std::size_t, 2> p)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] = p[i];
	}
}

IndexVect& IndexVect::operator= (const IndexVect &iv)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] = iv.vect[i];
	}
	return *this;
}

const int& IndexVect::operator[] (int i) const
{
  return vect[i];
}

int& IndexVect::operator[] (int i)
{
  return vect[i];
}

IndexVect IndexVect::operator+ (const IndexVect& p) const
{
	IndexVect v(vect[0] + p[0], vect[1] + p[1]);
  return v;
}

IndexVect& IndexVect::operator+= (const IndexVect& p)
{
	for (int i = 0; i < 1; ++ i)
	{
		vect[i] += p.vect[i];
	}
  return *this;
}

IndexVect IndexVect::operator- (const IndexVect& p) const
{
	IndexVect v(vect[0] - p[0], vect[1] - p[1]);
  return *this;
}

inline IndexVect& IndexVect::operator-= (const IndexVect& p)
{
	for (int i = 0; i < 1; ++ i)
	{
		vect[i] -= p.vect[i];
	}
  return *this;
}

IndexVect IndexVect::operator* (int s) const
{
  IndexVect v(vect[0] * s, vect[1] * s);
  return v;
}

IndexVect& IndexVect::operator*= (int s)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] *= s;
	}
	return *this;
}

IndexVect IndexVect::operator/ (int s) const
{
  IndexVect v(vect[0] / s, vect[1] / s);
  return v ;
}

IndexVect& IndexVect::operator/= (int s)
{
	for (int i = 0; i < DIM; ++ i)
	{
		vect[i] /= s;
	}
	return *this;
}

std::ostream& operator<< (std::ostream& ostr, const IndexVect& p)
{
  ostr << "[" << p[0] << " " << p[1] << "]" ;
  return ostr;
}
