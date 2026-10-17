/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
//-----------------------------
// TriDiagMatrix.h
//
//    solves equations of the form     A u = r
//
//           where A is a tri-diagonal matrix
//                 u is the unknown vector
//                 r is a constant vector
//
//-----------------------------

#ifndef TRIDIAGMATRIX
#define TRIDIAGMATRIX

class TriDiagMatrix
{
public:
	TriDiagMatrix(int N);
	~TriDiagMatrix();

	TriDiagMatrix();  // default constructor for instantiating lists
	void init(int N);

	void setA(int index, double value);
	void setB(int index, double value);
	void setC(int index, double value);
	void setR(int index, double value);
	double getU(int index);

	int getSize() { return size; }

	void setA(double *value, int length);
	void setB(double *value, int length);
	void setC(double *value, int length);
	void setR(double *value, int length);
	void getU(double *value, int length);

	double *getA() { return a; }
	double *getB() { return b; }
	double *getC() { return c; }
	double *getR() { return r; }

	bool Solve();
	void show();
	void showVerify();
  
private:
	int size;
	double *a;
	double *b;
	double *c;
	double *r;
	double *u;
	double *gam;
};

#endif
