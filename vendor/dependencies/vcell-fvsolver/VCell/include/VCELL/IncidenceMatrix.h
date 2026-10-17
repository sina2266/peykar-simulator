#ifndef INCIDENCEMATRIX
#define INCIDENCEMATRIX

/*----------------------------------------------------------------------------
	Sparse matrix stored in PCGPAK2 form
	IncidenceMatrix  contains all the diagonal elements and all the non-zero off diagonal elements
	numNonZeros is the number of diagonal elements plus non-zero off diagonal elements

	If IncidenceMatrix  needs to be passed to Fortran, each element of ija must be incremented by 1.
 --------------------------------------------------------------------------------------*/
template<class TYPE> class IncidenceMatrix 
{
public:
    IncidenceMatrix(long N, long NonZ, int flg, int fixedRowSize=-1);
	IncidenceMatrix(IncidenceMatrix * sm);
    ~IncidenceMatrix();

    long getN() { return N; }
	long getNumNonZeros() {return numNonZeros;};
    int  getSymmetricFlag() { return symmflag; }    
    TYPE* getValue(long i, long j); // return the index of the sparsematrix;
	void setValue(long i, long j, TYPE value); // return the index of the sparsematrix;
	void addNewRow();
	void setCol(long col, TYPE value);
	void setDiag(long row, TYPE value);
	void close();
	TYPE* getsa() { return sa; };
	int getColumns(long i, int32*& columns, TYPE*& values);

private:
	long N;
	long numNonZeros;
	int symmflag;
	TYPE *sa;
	int32 *ija;
	long currentRow;
	long currentCol;
	long currentIndex;
	long fixedNonZeroesPerRow;
};


/*----------------------------------------------------------------------------
	IncidenceMatrix is actually varied sparse matrix. 
	1. it is template class
	2. we can insert values. To do that, for each row, there are fixed number(should be greater than the maximum non zeros) of 
		elements which might not be used (correpsonding ija is -1).
 --------------------------------------------------------------------------------------*/
template<class TYPE> IncidenceMatrix<TYPE>::IncidenceMatrix(long arg_N, long arg_numNonZeros, int arg_symmflag, int fixedRowSize)
{
	ASSERTION(arg_N > 0);
	ASSERTION(arg_numNonZeros > 0);

	N = arg_N;	
	symmflag = arg_symmflag;

	numNonZeros = arg_numNonZeros;
	fixedNonZeroesPerRow = fixedRowSize;

	try {
		sa = new TYPE[numNonZeros + 1];
		ija = new int32[numNonZeros + 1];
	} catch (...) {
		throw "IncidenceMatrix<TYPE>::IncidenceMatrix() : Out of memory";
	}

	memset(sa, 0, (numNonZeros + 1) * sizeof(TYPE));
	ija[0] = N + 1;	
	for (int i = 1; i < numNonZeros + 1; i ++) {
		if (fixedNonZeroesPerRow != -1 && i < N + 1) {
			ija[i] = ija[i - 1] + fixedNonZeroesPerRow; // first set every number to be an invalid number	
		} else {
			ija[i] = -1; // first set every number to be an invalid number	
		}
	}	
	currentRow = -1;
	currentCol = -1;
	currentIndex = N;
}

template<class TYPE>IncidenceMatrix<TYPE>::IncidenceMatrix(IncidenceMatrix* sm)
{
	N = sm->N;
	numNonZeros = sm->numNonZeros;
	symmflag = sm->symmflag;
	fixedNonZeroesPerRow = sm->fixedNonZeroesPerRow;

	try {
		sa = new TYPE[numNonZeros + 1];
		ija = new int32[numNonZeros + 1];
	} catch (...) {
		throw "IncidenceMatrix<TYPE>::IncidenceMatrix() : Out of memory";
	}
	memcpy(sa, sm->sa, (numNonZeros + 1) * sizeof(TYPE));	
	memcpy(ija, sm->ija, (numNonZeros + 1) * sizeof(int32));
	currentRow = sm->currentRow;
	currentCol = sm->currentCol;
	currentIndex = sm->currentIndex;
}

template<class TYPE>IncidenceMatrix<TYPE>::~IncidenceMatrix()
{
	delete[] sa;
	delete[] ija;
}

template<class TYPE> TYPE* IncidenceMatrix<TYPE>::getValue(long i, long j) {
	if (i < 0 || j < 0 || i >= N || j >= N) {
		throw "IncidenceMatrix<TYPE>::getValue() : index out of bound";
	}
	if (i == j) {
		return sa + i;
	}
	if (ija[i] >= numNonZeros + 1) {
		return 0;
	}
	for (int32 k = ija[i]; k < ija[i + 1]; k ++) {
		if (ija[k] == j) {
			return sa + k;
		} else if (ija[k] == -1) {
			return 0;
		}
	}
	return 0;
}

template<class TYPE> void IncidenceMatrix<TYPE>::setValue(long i, long j, TYPE value) {
	if (i < 0 || j < 0 || i >= N || j >= N) {
		throw "IncidenceMatrix<TYPE>::getValue() : index out of bound";
	}
	if (fixedNonZeroesPerRow == -1 && currentRow != N-1) {
		throw "IncidenceMatrix<TYPE>::getValue() : can't insert value";
	}
	if (i == j) {
		sa[i] = value;
	}
	if (ija[i] >= numNonZeros + 1) {
		throw "IncidenceMatrix<TYPE>::setValue() : index out of bound";
	}

	bool bVacant = false;
	long insPos = -1;
	for (int32 k = ija[i]; k < ija[i + 1]; k ++) {
		if (ija[k] == -1) {
			bVacant = true;		
			if (insPos == -1) {
				insPos = k;
			}
			break;
		} else if (ija[k] == j) {
			sa[k] = value;
		} else if (ija[k] > j && insPos == -1) {
			insPos = k;
		}
	}

	if (insPos == -1 || !bVacant) {
		throw "IncidenceMatrix<TYPE>::setValue() : failed";
	}

	for (int32 k = ija[i + 1] - 2; k >= insPos; k --) {
		if (ija[k] == -1) {
			continue;
		}
		ija[k + 1] = ija[k];
		sa[k + 1] = sa[k];
	}
	ija[insPos] = j;
	sa[insPos] = value;
}

template<class TYPE> void IncidenceMatrix<TYPE>::addNewRow() {
	if (currentIndex >= numNonZeros) {
		throw "IncidenceMatrix<TYPE>::addNewRow() : index out of bound";
	}
	if (fixedNonZeroesPerRow != -1 && currentRow != -1) {
		ija[currentRow + 1] = ija[currentRow] + fixedNonZeroesPerRow;
		currentIndex = ija[currentRow + 1] - 1;
	} else {
		ija[currentRow + 1] = currentIndex + 1;
	}

	currentRow ++;
	currentCol = -1;
	if (currentRow >= N) {
		throw "IncidenceMatrix<TYPE>::addNewRow() : index out of bound";
	}
}

template<class TYPE> void IncidenceMatrix<TYPE>::setDiag(long row, TYPE value) {
	if (row >= N || row < 0) {
		throw "IncidenceMatrix<TYPE>::setDiag() : index out of bound";
	}
	sa[row] = value;
}

template<class TYPE> void IncidenceMatrix<TYPE>::setCol(long col, TYPE value) {
	if (col >= N || col < 0) {
		throw "IncidenceMatrix<TYPE>::setCol() : index out of bound";
	}
	if (col == currentRow) {
		sa[currentRow] = value;
	} else {
		if (currentIndex >= numNonZeros) {
			throw "IncidenceMatrix<TYPE>::setCol() : index out of bound";
		}
		if (col <= currentCol) {
			throw "IncidenceMatrix<TYPE>::setCol() : index out of order";
		}
		currentCol = col;
		currentIndex ++;
		sa[currentIndex] = value;
		ija[currentIndex] = col;
	}
}

template<class TYPE> void IncidenceMatrix<TYPE>::close() {
	if (currentRow >= N) {
		throw "IncidenceMatrix<TYPE>::close() : index out of bound";
	}
	if (fixedNonZeroesPerRow == -1) {
		ija[currentRow + 1] = currentIndex + 1;	
		for (int i = currentRow + 2; i <= N; i ++) {
			ija[i] = ija[currentRow + 1];
		}
	}
	currentRow = N - 1;
}


template<class TYPE> int IncidenceMatrix<TYPE>::getColumns(long i, int32*& columns, TYPE*& values) {
	if (i >= N || i < 0) {
		throw "IncidenceMatrix<TYPE>::getColumns() : index out of bound";
	}
	columns = ija + ija[i];
	values = sa + ija[i];
	return ija[i+1] - ija[i];
}

#endif
