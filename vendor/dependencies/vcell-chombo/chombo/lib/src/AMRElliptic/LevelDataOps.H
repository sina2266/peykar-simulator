#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

// BVS, June 26, 2003

// We can assume that template class T has null construction.

#ifndef _LEVELDATAOPS_H_
#define _LEVELDATAOPS_H_

#ifdef CH_MPI
#include <string>
#include <sstream>
#endif
#include "LevelData.H"
#include "RefCountedPtr.H"
#include "SPMD.H"
#include "Copier.H"
#include "NamespaceHeader.H"


// default copy constructor and assign are fine.

template <class T>
class LevelDataOps
{
public:
  LevelDataOps()
    :m_levelFactory( new DefaultDataFactory<T>() )
  {
  }

  LevelDataOps(RefCountedPtr<DataFactory<T> > a_factoryPtr)
    :m_levelFactory(a_factoryPtr)
  {
  }

  virtual ~LevelDataOps()
  {
  }

  virtual void define(const RefCountedPtr<DataFactory<T> >& a_factoryPtr)
  {
    m_levelFactory = a_factoryPtr;
  }

  virtual void define(DataFactory<T>* a_rawPointer)
  {
    m_levelFactory = RefCountedPtr<DataFactory<T> >(a_rawPointer);
  }

  virtual void create(    LevelData<T>& a_lhs, const LevelData<T>& a_rhs);

  virtual void assign(    LevelData<T>& a_lhs, const LevelData<T>& a_rhs) ;

  virtual Real dotProduct(const LevelData<T>& a_1, const LevelData<T>& a_2) ;

  virtual void mDotProduct(const LevelData<T>& a_1, const int a_sz, const  LevelData<T> a_2arr[], Real a_mdots[]);

  virtual void incr( LevelData<T>& a_lhs, const LevelData<T>& a_x, Real a_scale) ;

  virtual void mult( LevelData<T>& a_lhs, const LevelData<T>& a_x);

  virtual void axby( LevelData<T>& a_lhs, const LevelData<T>& a_x,
                     const LevelData<T>& a_y, Real a_a, Real a_b) ;

  virtual void scale(LevelData<T>& a_lhs, const Real& a_scale) ;

  virtual void plus(LevelData<T>& a_lhs, const Real& a_inc) ;

  virtual void setToZero(LevelData<T>& a_lhs);

  virtual void setVal(LevelData<T>& a_lhs, const Real& a_val);

  virtual void copyToZero(LevelData<T>& a_lhs, const Copier& a_copier);


protected:
  RefCountedPtr<DataFactory<T> > m_levelFactory;
};

//*******************************************************
// LevelDataOps Implementation
//*******************************************************
// BVS, June 26, 2003

#define ITER(a) for (DataIterator dit = a.dataIterator(); dit.ok(); ++dit) \
                {                                                          \
                   DataIndex d = dit();

#define ENDFOR(a)                                                          \
                }

template <class T>
void LevelDataOps<T>:: create(LevelData<T>& a_lhs, const LevelData<T>& a_rhs)
{
  // a_lhs.define(a_rhs, *m_levelFactory);
  a_lhs.define(a_rhs.disjointBoxLayout(), a_rhs.nComp(),
               a_rhs.ghostVect(), *m_levelFactory);
}

template <class T>
void LevelDataOps<T>:: assign(LevelData<T>& a_lhs, const LevelData<T>& a_rhs)
{
  Interval interv(0, a_rhs.nComp()-1);
  a_rhs.copyTo(interv, a_lhs, interv);
}

template <class T>
Real  LevelDataOps<T>::dotProduct(const LevelData<T>& a_1, const LevelData<T>& a_2)
{
  const DisjointBoxLayout& dbl = a_1.disjointBoxLayout();
  Real val = 0.0;
  ITER(a_1)
    val += a_1[d].dotProduct(a_2[d], dbl.get(d));
  ENDFOR(a_1);

#ifdef CH_MPI
  Real recv;
  int result = MPI_Allreduce(&val, &recv, 1, MPI_CH_REAL,
                             MPI_SUM, Chombo_MPI::comm);
  if ( result != 0 )
  {
    std::ostringstream msg;
    msg << "LevelDataOps::dotProduct() called MPI_Allreduce() which returned error code " << result ;
    MayDay::Warning( msg.str().c_str() );
  }
  val = recv;
#endif
  return val;

}

/* multiple dot products (for GMRES) */
template <class T>
void LevelDataOps<T>::mDotProduct(const LevelData<T>& a_1, const int a_sz, const LevelData<T> a_2arr[], Real a_mdots[])
{
  const DisjointBoxLayout& dbl = a_1.disjointBoxLayout();

  for (int ii=0;ii<a_sz;ii++)
    {
      Real val = 0.0;
      const LevelData<T> &a_2 = a_2arr[ii];
      ITER(a_1)
        val += a_1[d].dotProduct(a_2[d], dbl.get(d));
      ENDFOR(a_1);
      a_mdots[ii] = val;
    }

#ifdef CH_MPI
  Real *recv = new Real[a_sz];

  int result = MPI_Allreduce(a_mdots, recv, a_sz, MPI_CH_REAL,
                             MPI_SUM, Chombo_MPI::comm);
  if ( result != 0 )
  {
    std::ostringstream msg;
    msg << "LevelDataOps::mDotProduct() called MPI_Allreduce() which returned error code " << result ;
    MayDay::Warning( msg.str().c_str() );
  }
  for (int ii=0;ii<a_sz;ii++)
    {
      a_mdots[ii] = recv[ii];
    }

  delete [] recv;
#endif
}

template <class T>
void LevelDataOps<T>:: incr( LevelData<T>& a_lhs, const LevelData<T>& a_rhs, Real a_scale)
{
  //  CH_assert(a_lhs.disjointBoxLayout() == a_rhs.disjointBoxLayout());
  int numcomp = a_lhs.nComp();
  int  startcomp = 0;
  ITER(a_lhs)
    Box subbox = a_lhs.disjointBoxLayout()[d];
    a_lhs[d].plus(a_rhs[d],  subbox, subbox, a_scale, startcomp, startcomp, numcomp);
  ENDFOR(a_lhs);
}

template <class T>
void LevelDataOps<T>:: mult( LevelData<T>& a_lhs, const LevelData<T>& a_rhs)
{
  //  CH_assert(a_lhs.disjointBoxLayout() == a_rhs.disjointBoxLayout());
  ITER(a_lhs)
    a_lhs[d] *= a_rhs[d];
  ENDFOR(a_lhs);
}

template <class T>
void LevelDataOps<T>:: setToZero( LevelData<T>& a_lhs)
{
  ITER(a_lhs)
    a_lhs[d].setVal(0.0);
  ENDFOR(a_lhs);
}
template <class T>
void LevelDataOps<T>:: setVal( LevelData<T>& a_lhs, const Real& a_val)
{
  ITER(a_lhs)
    a_lhs[d].setVal(a_val);
  ENDFOR(a_lhs);
}

template <class T>
void LevelDataOps<T>:: copyToZero( LevelData<T>& a_lhs, const Copier& a_copier)
{
  int nComp = a_lhs.nComp();
  for (CopyIterator it(a_copier, CopyIterator::LOCAL); it.ok(); ++it)
    {
      const MotionItem& item = it();
      a_lhs[item.toIndex].setVal(0, item.toRegion, 0, nComp);

    }
  for (CopyIterator it(a_copier, CopyIterator::TO); it.ok(); ++it)
    {
      const MotionItem& item = it();
      a_lhs[item.toIndex].setVal(0, item.toRegion, 0, nComp);
    }
}

template <class T>
void LevelDataOps<T>:: axby( LevelData<T>& a_lhs, const LevelData<T>& a_x,
                             const LevelData<T>& a_y, Real a, Real b)
{
  //  CH_assert(a_lhs.disjointBoxLayout() == a_x.disjointBoxLayout());
  ITER(a_lhs)
    T& data = a_lhs[d];
  data.copy(a_x[d]);
  data.mult(a);
  data.plus(a_y[d], b);
  ENDFOR(a_lhs);
}

template <class T>
void LevelDataOps<T>:: scale(LevelData<T>& a_lhs, const Real& a_scale)
{
  ITER(a_lhs)
    T& data = a_lhs[d];
  data.mult(a_scale);
  ENDFOR(a_lhs);
}

template <class T>
void LevelDataOps<T>:: plus(LevelData<T>& a_lhs, const Real& a_inc)
{
  ITER(a_lhs)
    T& data = a_lhs[d];
  data += a_inc;
  ENDFOR(a_lhs);
}

#undef ITER
#undef ENDFOR

#include "NamespaceFooter.H"
#endif
