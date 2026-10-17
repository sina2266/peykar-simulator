#include <VCELL/SimTypes.h>
#include <VCELL/SparseMatrix.h>

SparseMatrix::SparseMatrix()
{
	// A linear system stored in sparse format
	size    = 0;    // size of system
	sizeA   = 0;    // size of array Am 
	sizeRSP = 0;    // size of work space (for pcgpack)
	symflg  = 0;    // symmetric format flag
	Am      = NULL;  // contains non-zero elements of matrix (or upper triangle)
	RHS     = NULL;  // contains RHS vector
	RSP     = NULL;  // work-array - used by pcgpack
	ija     = NULL;  // index array for sparse format
}

SparseMatrix::SparseMatrix(long sizeSystem, long NonZ, long Nwork, int flg)
{
	size    = 0;
	sizeA   = 0;
	sizeRSP = 0;
	symflg  = 0;
	Am      = NULL;
	RHS     = NULL;
	RSP     = NULL;
	ija     = NULL;
	init(sizeSystem, NonZ, Nwork, flg);
}

SparseMatrix::~SparseMatrix()
{
	delete[] Am;
	delete[] RHS;
	delete[] ija;
}

// void SparseMatrix::init(int Nx, int Ny, int Nz, long Nwork, int flg)
void SparseMatrix::init(long sizeSystem, long NonZ, long Nwork, int flg)
{
	size    = sizeSystem;
	sizeA   = NonZ;
	sizeRSP = Nwork;
	// Define storage flag:
	//   flg = 0  : general storage 
	//   flg != 0 : symmetric storage, 
	//   1: upper triangle,   -1:lower triangle(not implemented)
	symflg = flg;

	// Initialize arrays	
	try {
		Am      = new double[sizeA];
		ija     = new INT32[sizeA];
		RHS     = new double[size];
		RSP     = new double[sizeRSP];
	} catch (...) {
		throw "SparseMatrix::init() : Out of Memory";
	}

	// NOTE: it may be not necessary to initialize RSP. Check this out
	memset(Am, 0, sizeA * sizeof(double));
	memset(ija, 0, sizeA * sizeof(INT32));
	memset(RHS, 0, size * sizeof(double));
	memset(RSP, 0, sizeRSP * sizeof(double));
}

void SparseMatrix::show( )
{
	printf("SparseMatrix and RHS\n");
	printf("%10s %10s %14s %14s\n", "k", "IJA[k]","sa[k]");	
	for (long i = 0; i < ija[size] - 1; i ++){
		printf("%10ld %10ld %14.10lg\n", i, ija[i]-1, Am[i]);
	}
}



void SparseMatrix::show(int N1show, int N2show )
{
	printf("SparseMatrix and RHS\n");
	printf(" Am              IJA              RHS\n");
	int i=0;
	for (i=N1show;i<N2show+1;i++){
		printf("%ld %14g %14ld %14g\n",i, Am[i],ija[i],RHS[i]);
	}
	for (i=ija[N1show];i<ija[N2show+1];i++){
		printf("%ld %14g %14ld \n", i, Am[i],ija[i]);
	}
}

void SparseMatrix::setAm(long index, double value)
{
	ASSERTION(index>0 && index<sizeA);
	Am[index]=value;
}

void SparseMatrix::setija(long index, long value)
{
	ASSERTION(index>0 && index<sizeA);
	ija[index]=value;
}

void SparseMatrix::setSameija(long index1, long index2)
{
	// set all elements of ija array from index1 to index2+1
	// equal to ija[index1]
	ASSERTION(index2>=index1);
	ASSERTION(index2+1<sizeA);
	ASSERTION(ija[index1]);
	INT32 i0 = ija[index1];
	for (long i=index1+1;i<index2+2;i++){
		ija[i] = i0;
	}
}


void SparseMatrix::setRhs(long index, double value)
{
	ASSERTION(index>=0 && index<size);
	RHS[index]=value;
}

/*
void SparseMatrix::setCurrSol(long index, double value)
{
   ASSERTION(index>=0 && index<size);
   CurrSol[index]=value;
}
*/

/*
void SparseMatrix::setIja(int index, long value)
{
   ASSERTION(index>=0 && index<sizeA);
   ija[index]=value;
}
*/

void SparseMatrix::setAm(double *value, long length)
{
	ASSERTION(length==sizeA);
	memcpy(Am, value, sizeof(double)*length);
}

void SparseMatrix::setRhs(double *value, long length)
{
	ASSERTION(length==size);
	memcpy(RHS, value, sizeof(double)*length);
}
