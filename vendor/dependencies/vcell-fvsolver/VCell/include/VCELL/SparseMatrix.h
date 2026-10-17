//-----------------------------
// SparseMatrix.h
//
//    Sparse Matrix Class: sparse mtrix stored in PCGPAK2 form
//
//-----------------------------

#ifndef SPARSEMATRIX
#define SPARSEMATRIX

class SparseMatrix
{
  public:
    SparseMatrix(long sizeSystem, long NonZ, long Nwork, int flg);
    ~SparseMatrix();

    SparseMatrix();  // default constructor for instantiating lists
    void init(long sizeSystem, long NonZ, long Nwork, int flg);

    void setAm(long index, double value);
    void setRhs(long index, double value);
    void setija(long index, long value);
    void setSameija(long index1, long index2);
    
    long getSize() { return size; }
    long getSizeA() { return sizeA; }
    long getSizeRSP() { return sizeRSP; }

    void setAm(double *value, long length);
    void setRhs(double *value, long length);

    double *getAm() { return Am; }
    double *getRhs() { return RHS; }
    double *getRSP() { return RSP; }
    // was long /// NOTE: ija type must be half of a double precission 
    //                    real because of pcgpak work array requirements

    INT32   *getIja() { return ija; }
    int     getSymflg() { return symflg; }
    
    double SparseMatrix::getAm(long irow, long jcol);

    void show();
    void show(int N1show, int N2show);
  
  private:
    long size;
    long sizeA;
    long sizeRSP; // size of the work array for PCG
    int symflg;
    double *Am;
    double *RHS;
    double *RSP;
    INT32 *ija;
};

#endif
