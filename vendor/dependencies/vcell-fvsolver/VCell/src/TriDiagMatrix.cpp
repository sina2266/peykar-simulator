/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/SimTypes.h>
#include <VCELL/TriDiagMatrix.h>

TriDiagMatrix::TriDiagMatrix()
{
	size = 0;
	a = NULL;
	b = NULL;
	c = NULL;
	r = NULL;
	u = NULL;
	gam = NULL;
}

TriDiagMatrix::TriDiagMatrix(int N)
{
	size = 0;
	a = NULL;
	b = NULL;
	c = NULL;
	r = NULL;
	u = NULL;
	gam = NULL;
	init(N);
}

TriDiagMatrix::~TriDiagMatrix()
{
	//    delete[] a;
	//    delete[] b;
	//    delete[] c;
	//    delete[] r;
	//    delete[] u;
}

void TriDiagMatrix::init(int N)
{
	size = N;
	try {
		a = new double[size];
		b = new double[size];
		c = new double[size];
		r = new double[size];
		u = new double[size];
		gam = new double[size];
	} catch (...) {
		throw "TriDiagMatrix::init() : Out of memory";
	}
	for (int i=0;i<size;i++){
		a[i] = 0.0;
		b[i] = 0.0;
		c[i] = 0.0;
		r[i] = 0.0;
		u[i] = 0.0;
	}
}

void TriDiagMatrix::show()
{
	printf("TriDiagMatrix and solution\n");
	printf("a              b              c              r              u\n");
	for (int i=0;i<size;i++){
		printf("%-14lg %-14lg %-14lg %-14lg %-14lg\n", a[i],b[i],c[i],r[i],u[i]);
	}
}

void TriDiagMatrix::showVerify()
{
	printf("TriDiagMatrix and solution and check\n");
	printf("a              b              c              r              u              checked_r\n");
	for (int i=0;i<size;i++){
		double rr=0;
		if (i>0){
			rr+=a[i]*u[i-1];
		}
		rr+=b[i]*u[i];
		if (i<size-1){
			rr+=c[i]*u[i+1];
		}
		printf("%-14lg %-14lg %-14lg %-14lg %-14lg %-14lg\n", a[i],b[i],c[i],r[i],u[i],rr);
	}
}

bool TriDiagMatrix::Solve()
{
	int    j;
	double   bet;
	   
	for (j=0;j<size;j++){
		u[j] = 0.0;
	}
	   
	if (b[0] == 0.0) { 
		printf("TriDiagMatrix::solve(), rewrite equ. as order N-1\n");
		show();
		return false;
	}
	   
	u[0] = r[0] / b[0];
	bet = b[0];
	//
	// decomposition and forward substitution
	//
	for (j=1;j<size;j++){
		gam[j] = c[j-1]/bet;
		bet = b[j] - a[j]*gam[j];
		if (bet == 0.0) {
			printf("TriDiagMatrix::solve(), Algorithm fails\n");
			show();
			return false;
		}
		u[j] = (r[j] - a[j]*u[j-1]) / bet;
	}
	//
	// back substitution
	//
	for (j=(size-2);j>=0;j--){
		u[j] -= gam[j+1]*u[j+1];
	}
	   
	return true;
}

void TriDiagMatrix::setA(int index, double value)
{
	ASSERTION(index>0 && index<size);
	a[index]=value;
}

void TriDiagMatrix::setB(int index, double value)
{
	ASSERTION(index>=0 && index<size);
	b[index]=value;
}

void TriDiagMatrix::setC(int index, double value)
{
	ASSERTION(index>=0 && index<size-1);
	c[index]=value;
}

void TriDiagMatrix::setR(int index, double value)
{
	ASSERTION(index>=0 && index<size);
	r[index]=value;
}

double TriDiagMatrix::getU(int index)
{
	ASSERTION(index>=0 && index<size);
	return u[index];
}

void TriDiagMatrix::setA(double *value, int length)
{
	ASSERTION(length==size);
	memcpy(a, value, sizeof(double)*length);
}

void TriDiagMatrix::setB(double *value, int length)
{
	ASSERTION(length==size);
	memcpy(b, value, sizeof(double)*length);
}

void TriDiagMatrix::setC(double *value, int length)
{
	ASSERTION(length==size);
	memcpy(c, value, sizeof(double)*length);
}

void TriDiagMatrix::setR(double *value, int length)
{
	ASSERTION(length==size);
	memcpy(r, value, sizeof(double)*length);
}

void TriDiagMatrix::getU(double *value, int length)
{
	ASSERTION(length==size);
	memcpy(value, u, sizeof(double)*length);
}
