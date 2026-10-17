#include <VCELL/SparseMatrixPCG.h>
#include <VCELL/FVUtils.h>
#include <string.h>
#include <stdio.h>


/*----------------------------------------------------------------------------
	SparseMatrixPCG contains all the diagonal elements and all the non-zero off diagonal elements
	numNonZeros is the number of diagonal elements plus non-zero off diagonal elements
 --------------------------------------------------------------------------------------*/
SparseMatrixPCG::SparseMatrixPCG(long arg_N, long arg_numNonZeros, int arg_symmflag)
{
	ASSERTION(arg_N > 0);
	ASSERTION(arg_numNonZeros > 0);

	N = arg_N;
	numNonZeros = arg_numNonZeros;
	symmflag = arg_symmflag;

	try {
		sa = new double[numNonZeros + 1];
		ija = new int32[numNonZeros + 1];
	} catch (...) {
		throw "Out of memory";
	}	

	clear();
	fortranIJA = NULL;
}

SparseMatrixPCG::SparseMatrixPCG(SparseMatrixPCG* sm)
{
	N = sm->N;
	numNonZeros = sm->numNonZeros;
	symmflag = sm->symmflag;

	try {
		sa = new double[numNonZeros + 1];
		ija = new int32[numNonZeros + 1];
	} catch (...) {
		throw "Out of memory";
	}

	memcpy(sa, sm->sa, (numNonZeros + 1) * sizeof(double));
	memcpy(ija, sm->ija, (numNonZeros + 1) * sizeof(int32));
	currentRow = sm->currentRow;
	currentCol = sm->currentCol;
	currentIndex = sm->currentIndex;

	fortranIJA = NULL;
}

SparseMatrixPCG::~SparseMatrixPCG()
{
	delete[] sa;
	delete[] ija;
	delete[] fortranIJA;
}

void SparseMatrixPCG::clear() {
	memset(sa, 0, (numNonZeros + 1) * sizeof(double));
	memset(ija, 0, (numNonZeros + 1) * sizeof(int32));
	ija[0] = N + 1;
	currentRow = -1;
	currentCol = -1;
	currentIndex = N;	
}

void SparseMatrixPCG::show()
{
	printf("SparseMatrixPCG\n");
	printf("%10s %10s %14s\n", "k", "ija[k]", "sa[k]");
	for (int i = 0; i < ija[N]; i ++){
		printf("%10d %10d %14.10lg\n", i, ija[i], sa[i]);		
	}	
	if (N < 20) {
		printf("\nMatrix:\n");
		for (int i = 0; i < N; i ++){
			for (int j = 0; j < N; j ++){
				printf("%8lg ",getValue(i, j));
			}
			printf("\n");
		}
	}
}

double SparseMatrixPCG::getValue(long i, long j) {
	if (i < 0 || j < 0 ||  i >= N ||  j >= N) {
		throw "SparseMatrixPCG::getValue() : index out of bound";
	}
	if (i == j) {
		return sa[i];
	}
	if (ija[i] >= numNonZeros + 1) {
		return 0.0;
	}
	for (int32 k = ija[i]; k < ija[i + 1]; k ++) {
		if (ija[k] == j) {
			return sa[k];
		}
	}
	return 0.0;
}

void SparseMatrixPCG::setValue(long i, long j, double value) {
	if (i < 0 || j < 0 || i >= N || j >= N || currentRow != N-1) {
		throw "SparseMatrixPCG::setValue() : index out of bound";
	}
	if (i == j) {
		sa[i] = value;
		return;
	}
	if (ija[i] >= numNonZeros + 1) {
		ASSERTION(0);
	}
	for (int32 k = ija[i]; k < ija[i + 1]; k ++) {
		if (ija[k] == j) {
			sa[k]=value;
			return;
		}
	}
	ASSERTION(0);
}

void SparseMatrixPCG::addNewRow() {
	ija[currentRow + 1] = currentIndex + 1;
	currentRow ++;
	currentCol = -1;
	if (currentRow >= N) {
		throw "SparseMatrixPCG::addNewRow() : index out of bound";
	}
}

void SparseMatrixPCG::setDiag(long row, double value) {
	if (row >= N || row < 0) {
		throw "SparseMatrixPCG::setDiag() : index out of bound";
	}
	sa[row] = value;
}

void SparseMatrixPCG::setCol(long col, double value) {
	if (col >= N || col < 0) {
		throw "SparseMatrixPCG::setCol() : index out of bound";
	}

	if (col == currentRow) {
		sa[currentRow] = value;
	} else {
		if (col <= currentCol) {
			throw "SparseMatrixPCG::setCol() : index out of order";
		}
		currentCol = col;
		currentIndex ++;
		sa[currentIndex] = value;
		ija[currentIndex] = col;
	}
}

void SparseMatrixPCG::setRow(double diag, int numCols, int* cols, double* values) {
	sa[currentRow] = diag;
	if (numCols > 0) {
		if (cols != 0) {
			memcpy(ija + currentIndex + 1, cols, numCols * sizeof(int32));
			currentCol = cols[numCols - 1];
		}
		if (values != 0) {
			memcpy(sa + currentIndex + 1, values, numCols * sizeof(double));
		}
		currentIndex += numCols;
	}
}

void SparseMatrixPCG::close() {
	if (currentRow >= N) {
		throw "SparseMatrixPCG::close() : index out of bound";
	}
	ija[currentRow + 1] = currentIndex + 1;
	for (int i = currentRow + 2; i <= N; i ++) {
		ija[i] = ija[currentRow + 1];
	}
	currentRow = N - 1;
}

int32* SparseMatrixPCG::getFortranIJA() {
	if (fortranIJA == NULL) {
		fortranIJA  = new int32[numNonZeros + 1];
	}
	for (int i = 0; i < numNonZeros + 1; i ++) {
		fortranIJA[i] = ija[i] + 1;
	}
	return fortranIJA;
}

int SparseMatrixPCG::getColumns(long i, int32*& columns, double*& values) {
	if (i < 0 || i >= N) {
		throw "SparseMatrixPCG::getColumns() : index out of bound";
	}
	columns = ija + ija[i];
	values = sa + ija[i];
	return ija[i+1] - ija[i];
}

void SparseMatrixPCG::scaleOffDiagonals(double gamma) {
	int size = ija[N] - ija[0];
	int incr = 1;
	DSCAL(&size, &gamma, sa + ija[0],  &incr);
}

void SparseMatrixPCG::shiftDiagonals(double gamma){
	for (int i = 0; i < N; i ++) {
		sa[i] = 1 + gamma * (sa[i] - 1);
	}
}
