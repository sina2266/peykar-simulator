#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#include <cmath>
#include <cstdlib>
#include <cstring>
#include "SPACE.H"

#include "AllRegularService.H"
#include "Box.H"
#include "BoxIterator.H"
#include "CONSTANTS.H"
#include "DataIterator.H"
#include "DebugDump.H"
#include "EBCellFAB.H"
#include "EBFaceFAB.H"
#include "IrregFAB.H"
#include "IrregFABFactory.H"
#include "EBIndexSpace.H"
#include "FArrayBox.H"
#include "LayoutIterator.H"
#include "LevelData.H"
#include "Misc.H"
#include "REAL.H"
#include "SPMD.H"
#include "SlabService.H"
#include "UGIO.H"
#include "Vector.H"
#include "BRMeshRefine.H"
#include "GeometryShop.H"
#include "SphereIF.H"
#include "BaseIVFactory.H"
#include "UsingNamespace.H"

void baseivfab_fun(const Box &  a_domain)
{
  int max_grid_size = 16;
  int numghost = 4;
  Vector<Box> vbox;
  Vector<int> proc;
  domainSplit( a_domain, vbox, max_grid_size);
  LoadBalance(proc, vbox);
  
  DisjointBoxLayout dbl(vbox, proc);
  
  pout() << "layout = " << dbl << endl;
  
  EBISLayout ebisl;
  EBIndexSpace *ebisPtr = Chombo_EBIS::instance();
  ebisPtr->fillEBISLayout( ebisl, dbl, a_domain, numghost+2);
  //this makes an  set of BaseIFABs over all irregular cells
  // You can also specify a LayoutData<IntVectSet> but you
  //have to make sure that if a cell in one grid is in the set,
  //it is also in any other overlapping box
  BaseIVFactory<Real> fact(ebisl);
  LevelData<BaseIVFAB<Real> > irregDat(dbl, 1, numghost*IntVect::Unit, fact);
  //first set the cells to their box number on this proc, -1 in ghost cells
  Real boxnum = 0.;
  for(DataIterator  dit = dbl.dataIterator(); dit.ok(); ++dit)
    {
      Box validBox = dbl[dit()];
      Box grownBox = validBox;
      grownBox.grow(numghost);
      irregDat[dit()].setVal(-1.);

      const EBISBox& ebisBox =  ebisl[dit()];
      //select only the valid cells (not ghost cells)
      IntVectSet ivs = ebisBox.getIrregIVS(validBox);

      for(VoFIterator vofit(ivs, ebisBox.getEBGraph());  vofit.ok(); ++vofit)
        {
          irregDat[dit()](vofit(), 0) = boxnum;
        }

      boxnum++;
    }

  //now  exchange ghost cell information.   You can cache the copier if you want
  //this to go faster
  irregDat.exchange();
  
  //now let us see the result
  pout() << "ghost irregular cells values:" << endl;
  int ibox= 0;
  for(DataIterator  dit = dbl.dataIterator(); dit.ok(); ++dit)
    {
      //select only the valid cells (not ghost cells)
      //valid cells
      Box validBox = dbl[dit()];
      Box grownBox = validBox;
      grownBox.grow(numghost);
      
      IntVectSet ivsIrregGhost = ebisl[dit()].getIrregIVS(grownBox);
      ivsIrregGhost -= validBox;

      pout() << "box number = " << ibox << endl;
      for(VoFIterator vofit(ivsIrregGhost, ebisl[dit()].getEBGraph());  vofit.ok(); ++vofit)
        {
          Real value = irregDat[dit()](vofit(), 0);
          pout() << "vof = " << vofit() << ", value = "  << value << endl;
        }

      ibox++;
    }
}

void do_geo( Box &domain, Real &dx )
{
  int N = 64;
  Real xdom = 1.0;

  dx = xdom/N;
  const IntVect hi = (N-1)*IntVect::Unit;
  domain=  Box(IntVect::Zero, hi);

  Real radius = 0.2;
  RealVect center= 0.5*RealVect::Unit;
  SphereIF sphere(radius, center, false);

  RealVect vectDx = RealVect::Unit;
  vectDx *= dx;

  GeometryShop geom(sphere, 0, vectDx );

  RealVect origin = RealVect::Zero;

  EBIndexSpace *ebisPtr = Chombo_EBIS::instance();

  ebisPtr->define(domain, origin, dx, geom);

  return;
}


int
main(int argc, char** argv)
{

#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif
  //begin forever present scoping trick
  {
    // Set up some geometry.
    Box domain;
    Real dx;

    do_geo( domain, dx );

    baseivfab_fun(domain);

    EBIndexSpace *ebisPtr = Chombo_EBIS::instance();
    ebisPtr->clear();

  }//end scoping trick
#ifdef CH_MPI
  MPI_Finalize();
#endif
  return 0;
}

