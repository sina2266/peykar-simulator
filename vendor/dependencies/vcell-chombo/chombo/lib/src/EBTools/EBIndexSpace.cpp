#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#include "parstream.H"
#include "memtrack.H"
#include "CH_Attach.H"
#include "IndexTM.H"
#include "BoxIterator.H"
#include "LoadBalance.H"
#include "LayoutIterator.H"
#include "BRMeshRefine.H"
#include "AMRIO.H"

#include "EBCFCopy.H"
#include "EBIndexSpace.H"
#include "EBGraphFactory.H"
#include "EBDataFactory.H"
#include "EBISLayout.H"
#include "VoFIterator.H"
#include "IrregNode.H"
#include "AllRegularService.H"
#include "PolyGeom.H"
#include "EBLevelDataOps.H"

#include "NamespaceHeader.H"

Real EBIndexSpace::s_tolerance = 1.0e-12;
Real    EBISLevel::s_tolerance = 1.0e-12;
bool EBIndexSpace::s_verbose   = false;
bool    EBISLevel::s_verbose   = false;
bool EBIndexSpace::s_MFSingleBox=false;

int countIrreg(const GeometryService& a_geoserver,
               const Box&             a_region,
               const ProblemDomain&   a_domain,
               const RealVect&        a_origin,
               const Real&            a_dx)
{
  int count = 0;
  int longdir;
  int length = a_region.longside(longdir);
  if (length <= 2)
    {
      return 1;
    }
  else
    {
      GeometryService::InOut  inout = a_geoserver.InsideOutside(a_region, a_domain, a_origin, a_dx);
      if (inout == GeometryService::Irregular)
        {
          int n = length/2;
          //CH_assert(n*2==length);
          Box low(a_region), high;
          high = low.chop(longdir, a_region.smallEnd(longdir)+n);
          count += countIrreg(a_geoserver, low,  a_domain, a_origin, a_dx);
          count += countIrreg(a_geoserver, high, a_domain, a_origin, a_dx);
        }
      else
        {
          return 0;
        }

    }
  return count;
}

/******************/
int EBIndexSpace::numVoFs(const ProblemDomain& a_domain) const
{
  CH_TIME("EBIndexSpace::numVoFs");
  int ilev = getLevel(a_domain);
  const  EBISLevel&  ebisLevel = *m_ebisLevel[ilev];
  int numPtsLocal = ebisLevel.numVoFsOnProc();
  int  numPtsTot = EBLevelDataOps::parallelSum(numPtsLocal);
  return numPtsTot;
}

/******************/
int EBISLevel::numVoFsOnProc() const
{
  CH_TIME("EBISLevel::numVoFsOnProc");
  int retval = 0;
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      const EBGraph& ebgraph = m_graph[dit()];
      int numVoFsBox = ebgraph.numVoFs(m_grids.get(dit()));
      retval += numVoFsBox;
    }

  return retval;
}

/******************/
void EBISLevel::makeBoxes(Vector<Box>&               a_boxes,
                          Vector<long>&              a_loads,
                          const Box&                 a_region,
                          const ProblemDomain&       a_domain,
                          const GeometryService&     a_geoserver,
                          const RealVect&            a_origin,
                           const Real&               a_dx,
                           const int                 a_ncellmax,
                           const EBIndexSpace* const a_ebisPtr)
{
  if (EBIndexSpace::s_MFSingleBox)
    {
      a_boxes.resize(1);
      a_loads.resize(1);
      a_boxes[0] = a_region;
      a_loads[0] = 1;
      return;
    }
  domainSplit(a_domain, a_boxes, a_ncellmax, 1);
  mortonOrdering(a_boxes);
  a_loads.resize(a_boxes.size(), 1);
  /**
  std::list<Box> boxes;
  makeBoxes(boxes, a_region, a_domain, a_geoserver, a_origin, a_dx, a_ncellmax);
  a_boxes.resize(boxes.size());
  a_loads.resize(boxes.size());
  std::list<Box>::iterator it = boxes.begin();
  for (int i = 0; i < a_boxes.size(); ++i, ++it)
    {
      a_boxes[i]=*it;
    }
  mortonOrdering(a_boxes);
  for (int i = 0; i < a_boxes.size(); i++)
    {
      if (a_boxes[i].ixType() ==  IndexType::TheNodeType())
        {
          a_boxes[i].convert(IndexType::TheCellType());
          a_loads[i] = 8;
        }
      else
        {
          a_loads[i] = 1;
        }
    }
  **/
}

void EBISLevel::makeBoxes(std::list<Box>&        a_boxes,
                          const Box&             a_region,
                          const ProblemDomain&   a_domain,
                          const GeometryService& a_geoserver,
                          const RealVect&        a_origin,
                          const Real&            a_dx,
                          const int              a_ncellmax)
{
  int longdir;
  int length = a_region.longside(longdir);

  if (length > a_ncellmax)
    {
      int n = length/2;
      //CH_assert(n*2==length);
      Box low(a_region), high;
      high = low.chop(longdir, a_region.smallEnd(longdir)+n);
      makeBoxes(a_boxes, low,  a_domain,
                a_geoserver, a_origin, a_dx, a_ncellmax);
      makeBoxes(a_boxes, high, a_domain,
                a_geoserver, a_origin, a_dx, a_ncellmax);
    }
  else
    {
      if (a_geoserver.InsideOutside(a_region, a_domain, a_origin, a_dx) == GeometryService::Irregular)
        {
          Box n = a_region;
          n.convert(IndexType::TheNodeType());
          a_boxes.push_back(n);
        }
      else
        {
          a_boxes.push_back(a_region);
        }
    }
}

/******************/
#ifdef CH_USE_HDF5
EBISLevel::EBISLevel(HDF5Handle& a_handle)
{
  CH_TIME("EBISLevel::EBISLevel_hdf5");
  m_cacheMisses = 0;
  m_cacheHits   = 0;
  m_cacheStale  = 0;

  HDF5HeaderData header;
  header.readFromFile(a_handle);
  m_origin = header.m_realvect["EBIS_origin"];
  m_domain = header.m_box     ["EBIS_domain"];
  m_dx =     header.m_real    ["EBIS_dx"]    ;
  m_tolerance = m_dx*1E-4;

  //read in the grids
  Vector<Box> boxes;
  read(a_handle,boxes);
  Vector<int> procAssign;
  LoadBalance(procAssign, boxes);
  m_grids.define(boxes, procAssign);//this should use m_domain for periodic...
  EBGraphFactory graphfact(m_domain);
  m_graph.define(m_grids, 1, IntVect::Zero, graphfact);

  //read the graph  in from the file
  std::string graphName("EBIS_graph");
  int eekflag = read(a_handle, m_graph, graphName, m_grids, Interval(), false);
  if (eekflag != 0)
    {
      MayDay::Error("error in writing graph");
    }

  //need a ghosted layout so that the data can be defined properly
  LevelData<EBGraph> ghostGraph(m_grids, 1, IntVect::Unit, graphfact);
  Interval interv(0,0);
  m_graph.copyTo(interv, ghostGraph, interv);

  //now the data for the graph
  EBDataFactory dataFact;
  m_data.define(m_grids, 1, IntVect::Zero, dataFact);
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      m_data[dit()].defineVoFData(ghostGraph[dit()], m_grids.get(dit()));
      m_data[dit()].defineFaceData(ghostGraph[dit()], m_grids.get(dit()));
    }
  //read the data  in from the file
  std::string  dataName("EBIS_data");
  eekflag = read(a_handle, m_data ,  dataName, m_grids, Interval(), false);
  if (eekflag != 0)
    {
      MayDay::Error("error in writing data");
    }
}

/******************/
void EBISLevel::write(HDF5Handle& a_handle) const
{
  CH_TIME("EBISLevel::write");
  HDF5HeaderData header;
  //this naming stuff kinda depends on the fact
  //that we are only outputting the finest level.
  //we could be slick and incorporate the
  //level number in there if we wanted.
  header.m_int["num_levels"] = 1;
  header.m_int["num_components"] = 1;
  header.m_string["component_0"] = "phi0";
  header.m_realvect["EBIS_origin"] = m_origin;
  header.m_box     ["EBIS_domain"] = m_domain.domainBox();
  header.m_real    ["EBIS_dx"]     = m_dx;
  header.writeToFile(a_handle);
  //write the grids to the file
  CH_XD::write(a_handle, m_grids);

  std::string graphName("EBIS_graph");
  std::string  dataName("EBIS_data");
  int eekflag = CH_XD::write(a_handle, m_graph, graphName);
  if (eekflag != 0)
    {
      MayDay::Error("error in writing graph");
    }
  eekflag = CH_XD::write(a_handle, m_data ,  dataName);
  if (eekflag != 0)
    {
      MayDay::Error("error in writing data");
    }
}
#endif

/******************/
EBISLevel::EBISLevel(const ProblemDomain   & a_domain,
                     const RealVect        & a_origin,
                     const Real            & a_dx,
                     const GeometryService & a_geoserver,
                     const EBIndexSpace    * const a_ebisPtr,
                     const bool            & a_distributedData,
                     const bool            & a_fixRegularNextToMultiValued)
{
  // this is the method called by EBIndexSpace::buildFirstLevel
  CH_TIME("EBISLevel::EBISLevel_geoserver_domain");
  m_cacheMisses = 0;
  m_cacheHits   = 0;
  m_cacheStale  = 0;

  m_domain = a_domain;
  m_dx = a_dx;
  m_tolerance = a_dx*1E-4;
  m_origin = a_origin;

  if (!a_distributedData)
    { // this is the original code

      //divide up the domain into a layout
      Vector<Box> vbox;
      Vector<long> irregCount;
      {
        CH_TIME("EBISLevel::EBISLevel_makeboxes");
        makeBoxes(vbox,
                  irregCount,
                  a_domain.domainBox(),
                  a_domain,
                  a_geoserver,
                  a_origin,
                  a_dx,
                  a_ebisPtr->getNCellMax(),
                  a_ebisPtr);
      }

      // pout()<<vbox<<"\n\n";
      //load balance the boxes
      Vector<int> procAssign;
      LoadBalance(procAssign, irregCount, vbox);
      //   pout()<<irregCount<<std::endl;
      //   pout()<<procAssign<<std::endl;
      m_grids.define(vbox, procAssign,a_domain);//this should use a_domain for periodic

    }
  else
    {
      // permit the geometry service to construct a layout
      int nCellMax = a_ebisPtr->getNCellMax();
      (const_cast<GeometryService*>(&a_geoserver))->makeGrids(a_domain, m_grids, nCellMax, 15);
    }

  RealVect dx2D;
  for (int i = 0; i < SpaceDim; i++)
    {
      dx2D[i]=a_dx;
    }

  (const_cast<GeometryService*>(&a_geoserver))->postMakeBoxLayout(m_grids,dx2D);



  {
    //both domain and box set by the factory when leveldata construction is used
    EBGraphFactory graphfact(m_domain);
    m_graph.define(m_grids, 1, IntVect::Unit, graphfact);
    LayoutData<Vector<IrregNode> > allNodes(m_grids);

    //define the graph stuff
    for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
      {
        Box region = m_grids.get(dit());
        region.grow(1);
        Box ghostRegion = grow(region,1);
        ghostRegion &= m_domain;
        region &= m_domain;

        EBGraph& ebgraph = m_graph[dit()];
        GeometryService::InOut inout;
        if (!a_distributedData)
        {
          inout = a_geoserver.InsideOutside(region, m_domain, m_origin, m_dx);
        }
        else
        {
          inout = a_geoserver.InsideOutside(region, m_domain, m_origin, m_dx, dit());
        }
        if (inout == GeometryService::Regular)
          {
            ebgraph.setToAllRegular();
          }
        else if (inout == GeometryService::Covered)
          {
            ebgraph.setToAllCovered();
          }
        else
          {
            BaseFab<int>       regIrregCovered(ghostRegion, 1);
            Vector<IrregNode>&  nodes = allNodes[dit()];

            if (!a_distributedData)
            {
              a_geoserver.fillGraph(regIrregCovered, nodes, region,
                                    ghostRegion, m_domain,
                                    m_origin, m_dx);
            }
            else
            {
              a_geoserver.fillGraph(regIrregCovered, nodes, region,
                                    ghostRegion, m_domain,
                                    m_origin, m_dx, dit());
            }
            //pout()<<nodes<<"\n";
#ifndef NDEBUG
            EBGraphImplem::checkGraph(regIrregCovered, nodes, region, m_domain);
#endif
            ebgraph.buildGraph(regIrregCovered, nodes, region, m_domain);

          }
      }

    //overallMemoryUsage();
    //need a ghosted graph to make data so we can define baseiffabs
    //LevelData<EBGraph> ghostGraph(m_grids, 1, IntVect::Unit, graphfact);
    //overallMemoryUsage();
    //Interval interv(0,0);
    //m_graph.copyTo(interv, ghostGraph, interv);
    //overallMemoryUsage();
    //now the data for the graph
    EBDataFactory dataFact;
    m_data.define(m_grids, 1, IntVect::Zero, dataFact);
    for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
      {
        //m_data[dit()].define(ghostGraph[dit()], allNodes[dit()], m_grids.get(dit()));
        m_data[dit()].define(m_graph[dit()], allNodes[dit()], m_grids.get(dit()));

      }
    //overallMemoryUsage();
  }

  //
  if (a_geoserver.canGenerateMultiCells())
    {
      if (a_fixRegularNextToMultiValued)
        {
          fixRegularNextToMultiValued();
        }
    }
}

/************/
//now fix the multivalued next to regular thing for the graph and the data
//the oldgraph/newgraph thing is necessary because the graphs are
//reference counted and they have to be kept consistent with the data
/***********/
void EBISLevel::fixRegularNextToMultiValued()
{
  CH_TIME("EBISLevel::fixRegularNextToMultiValued");
  EBGraphFactory graphfact(m_domain);
  LayoutData<IntVectSet> vofsToChange(m_grids);
  LevelData<EBGraph> oldGhostGraph(m_grids, 1, 2*IntVect::Unit, graphfact);
  Interval interv(0,0);
  m_graph.copyTo(interv, oldGhostGraph, interv);
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      m_graph[dit()].getRegNextToMultiValued(vofsToChange[dit()],
                                             oldGhostGraph[dit()]);

      m_graph[dit()].addFullIrregularVoFs(vofsToChange[ dit()],
                                          oldGhostGraph[dit()]);
    }

  EBDataFactory datafact;
  LevelData<EBGraph> newGhostGraph(m_grids, 1, 2*IntVect::Unit, graphfact);
  LevelData<EBData>  newGhostData(m_grids, 1, IntVect::Unit, datafact);
  m_graph.copyTo(interv, newGhostGraph, interv);
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      Box localBox = m_grids.get(dit());
      localBox.grow(1);
      localBox &= m_domain;
      newGhostData[dit()].defineVoFData(oldGhostGraph[dit()],  localBox);
      newGhostData[dit()].defineFaceData(oldGhostGraph[dit()], localBox);
    }
  m_data.copyTo(interv,  newGhostData,  interv);
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      m_data[dit() ].addFullIrregularVoFs(vofsToChange[dit()],
                                          newGhostGraph[dit()],
                                          newGhostData[dit()].getVolData(),
                                          oldGhostGraph[dit()]);
    }
}

/******************/
//checks to see the vofs are in the correct cells.
//checks to see that the faces are over the correct cells
//checks that volume fractions, area fractions are positive
//bail out with MayDay::Error if anything fails
/******************/
void EBISLevel::sanityCheck(const EBIndexSpace* const a_ebisPtr)
{
#if 0
  pout() << "EBISLevel::sanityCheck" << endl;
  CH_TIME("EBISLevel::sanityCheck");
  EBISLayout ghostLayout;

  a_ebisPtr->fillEBISLayout(ghostLayout, m_grids, m_domain, 1);

  Real maxcentval = 0.5+ s_tolerance;
  for (DataIterator dit = m_grids.dataIterator();  dit.ok(); ++dit)
    {
      const Box& thisBox = m_grids.get(dit());
      const EBISBox& ebisBox = ghostLayout[dit()];
      for (BoxIterator bit(thisBox); bit.ok(); ++bit)
        {
          const IntVect& iv = bit();
          Vector<VolIndex> vofs = ebisBox.getVoFs(iv);
          for (int ivof = 0; ivof < vofs.size(); ivof++)
            {
              const VolIndex& vof = vofs[ivof];
              if (vof.gridIndex() != iv)
                {
                  pout() << "EBISLevel::sanityCheck: Error" << endl;
                  pout() << "VoF at Intvect = " << iv
                         << "has grid index = " << vof.gridIndex() << endl;
                  MayDay::Error("EBISLevel::sanityCheck Error 1");
                }
              if (vof.cellIndex() < 0)
                {
                  pout() << "EBISLevel::sanityCheck: Error" << endl;
                  pout() << "VoF at Intvect = " << iv
                         << "has negative cell index = " << vof.cellIndex() << endl;
                  MayDay::Error("EBISLevel::sanityCheck Error 2");
                }
              Real volFrac = ebisBox.volFrac(vof);
              if (volFrac < 0.0)
                {
                  pout() << "EBISLevel::sanityCheck: Error" << endl;
                  pout() << "VoF at Intvect = " << iv
                         << "has invalid volume fraction = " << volFrac << endl;
                  MayDay::Error("EBISLevel::sanityCheck Error 5");
                }
              RealVect volCentroid = ebisBox.centroid(vof);
              for (int idir = 0; idir < SpaceDim; idir++)
                {
                  Real volcentdir = volCentroid[idir];
                  if (volFrac > s_tolerance)
                    {
                      if (volcentdir > maxcentval || volcentdir < -maxcentval)
                        {
                          pout() << "EBISLevel::sanityCheck: Error" << endl;
                          pout() << "VoF at Intvect = " << iv
                                 << " has invalid vol centroid = " << volcentdir
                                 << " at direction "<< idir << endl;
                          MayDay::Error("EBISLevel::sanityCheck Error 51");
                        }
                    }
                }
              for (int idir = 0; idir < SpaceDim; idir++)
                {
                  for (SideIterator sit; sit.ok(); ++sit)
                    {
                      Vector<FaceIndex> faces = ebisBox.getFaces(vof, idir, sit());
                      IntVect iv2 = iv + sign(sit())*BASISV(idir);
                      ////check for regular next to covered and multivalued next to regular
                      if (m_domain.contains(iv2))
                        {
                          if (ebisBox.isRegular(iv))
                            {
                              if (ebisBox.isCovered(iv2))
                                {
                                  pout() << iv << " is regular and " <<  iv2 << " is covered" << endl;
                                  MayDay::Error("EBISLevel::sanityCheck error 420 ");
                                }
                              else
                                {
                                  Vector<VolIndex> otherVoFs = ebisBox.getVoFs(iv2);
                                  if (otherVoFs.size() > 1)
                                    {
                                      pout() << iv << " is regular and " <<  iv2 << " is multivalued" << endl;
                                      MayDay::Error("EBISLevel::sanityCheck error 420.2 ");
                                    }
                                }
                            }
                        }
                      IntVect ivlo, ivhi;
                      if (sit() == Side::Lo)
                        {
                          ivlo = iv2;
                          ivhi = iv;
                        }
                      else
                        {
                          ivlo = iv;
                          ivhi = iv2;
                        }
                      for (int iface = 0; iface < faces.size(); iface++)
                        {
                          const FaceIndex& face = faces[iface];
                          if (face.gridIndex(Side::Lo) != ivlo)
                            {
                              pout() << "EBISLevel::sanityCheck: Error" << endl;
                              pout() << "face at IntVects = " << ivlo << "  " << ivhi
                                     << "has low IntVect  = " << face.gridIndex(Side::Lo)
                                     << endl;
                              MayDay::Error("EBISLevel::sanityCheck Error 3");
                            }
                          if (face.gridIndex(Side::Hi) != ivhi)
                            {
                              pout() << "EBISLevel::sanityCheck: Error" << endl;
                              pout() << "face at IntVects = " << ivlo << "  " << ivhi
                                     << "has high IntVect = " << face.gridIndex(Side::Hi)
                                     << endl;
                              MayDay::Error("EBISLevel::sanityCheck Error 4");
                            }
                          Real areaFrac = ebisBox.areaFrac(face);
                          if (areaFrac  < 0.0)
                            {
                              pout() << "EBISLevel::sanityCheck: Error" << endl;
                              pout() << "VoF at Intvect = " << iv
                                     << "has invalid area fraction = " << areaFrac << endl;
                              MayDay::Error("EBISLevel::sanityCheck Error 51");
                            }
                          if (areaFrac  >  s_tolerance)
                            {
                              RealVect faceCentroid = ebisBox.centroid(face);
                              for (int idir = 0; idir < SpaceDim; idir++)
                                {
                                  Real facecentdir = faceCentroid[idir];
                                  if (facecentdir > maxcentval || facecentdir < -maxcentval)
                                    {
                                      pout() << "EBISLevel::sanityCheck: Error" << endl;
                                      pout() << "VoF at Intvect = " << iv
                                             << " has invalid face centroid = " << facecentdir
                                             << " at direction "<< idir << endl;
                                      MayDay::Error("EBISLevel::sanityCheck Error 51");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif
}

EBISLevel::EBISLevel()
{
  m_cacheMisses = 0;
  m_cacheHits   = 0;
  m_cacheStale  = 0;
}

/******************/
//steps to coarsen an ebislevel:
//1. coarsen vofs
//1a. make a layout over refine(mydbl,2) and copy
//    fine layout into it
//1b.do connectivity bizbaz to make my vofs, volfrac, vof->fineVofs
//2. make faces doing connectivity jive
//2.23 make coarse geometric stuff (centroids and all that) from fine
//3. make coarse layout from coarsen(finelayout). and copy my data into it
//   to make fine->coarserVoF
// (so finer ebislevel does change in this function)
/******************/
void EBISLevel::dumpDebug(const string& a_string)
{
  if (m_domain.domainBox() == EBGraphImplem::s_doDebug)
    {
      pout() << a_string <<  ": ";
      for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
        {
          if (m_grids[dit()].contains(EBGraphImplem::s_ivDebug))
            {
              pout() << "EBIS1: " << EBGraphImplem::s_ivDebug;
              if (m_graph[dit()].isRegular(EBGraphImplem::s_ivDebug))
                {
                  pout() << " is regular" << endl;
                }
              else if (m_graph[dit()].isCovered(EBGraphImplem::s_ivDebug))
                {
                  pout() << " is covered" << endl;
                }
              else
                {
                  pout() << " is irregular" << endl;
                }
            }
        }

    }
}

void EBISLevel::coarsenVoFs(EBISLevel& a_fineEBIS)
{
  CH_TIME("EBISLevel::coarsenVoFs");
  //so that i can do the vofs and faces of this level.
  DisjointBoxLayout fineFromCoarDBL;
  refine(fineFromCoarDBL, m_grids, 2);
  fineFromCoarDBL.close();

  //no need for ghost cells here except to define the face data
  //you need the graph to be one bigger
  EBGraphFactory ebgraphfactFine(a_fineEBIS.m_domain);
  LevelData<EBGraph> fineFromCoarEBGraph(fineFromCoarDBL,1, IntVect::Unit, ebgraphfactFine);

  Interval interv(0,0);
  a_fineEBIS.m_graph.copyTo(interv, fineFromCoarEBGraph, interv);

  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      const EBGraph& fineEBGraph = fineFromCoarEBGraph[dit()];
      const Box& coarRegion      = m_grids.get(dit());
      EBGraph& coarEBGraph = m_graph[dit()];
      coarEBGraph.coarsenVoFs(fineEBGraph, coarRegion);
    }
  EBGraphFactory ebgraphfactCoar(m_domain);
  LevelData<EBGraph> coarGhostEBGraph(m_grids,1, IntVect::Unit, ebgraphfactCoar);
  m_graph.copyTo(interv, coarGhostEBGraph, interv);

  //dumpDebug(string("EBIS::coarsenVoFs"));

  EBDataFactory ebdatafact;
  LevelData<EBData> fineFromCoarEBData(fineFromCoarDBL,1, IntVect::Zero, ebdatafact);
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      const Box& localBox = fineFromCoarDBL.get(dit());
      fineFromCoarEBData[dit()].defineVoFData(fineFromCoarEBGraph[dit()],  localBox);
      fineFromCoarEBData[dit()].defineFaceData(fineFromCoarEBGraph[dit()], localBox);
    }
  a_fineEBIS.m_data.copyTo(interv, fineFromCoarEBData, interv);

  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      const EBGraph& fineEBGraph =  fineFromCoarEBGraph[dit()];
      const EBData& fineEBData = fineFromCoarEBData[dit()];
      const EBGraph& coarEBGraph = coarGhostEBGraph[dit()];

      m_data[dit()].coarsenVoFs(fineEBData, fineEBGraph, coarEBGraph, m_grids.get(dit()));
    }
}

/******************/
void EBISLevel::fixFineToCoarse(EBISLevel& a_fineEBIS)
{
  CH_TIME("EBISLevel::fixFineToCoarse");
  // make a coarse layout from the fine layout so that we
  //can fix the fine->coarseVoF thing.
  DisjointBoxLayout coarFromFineDBL;

  coarsen(coarFromFineDBL, a_fineEBIS.m_graph.getBoxes(), 2);
  coarFromFineDBL.close();
  EBGraphFactory ebgraphfact(m_domain);
  LevelData<EBGraph> coarFromFineEBGraph(coarFromFineDBL,1, IntVect::Zero, ebgraphfact);
  Interval interv(0,0);
  m_graph.copyTo(interv, coarFromFineEBGraph, interv);

  for (DataIterator dit = a_fineEBIS.m_grids.dataIterator(); dit.ok(); ++dit)
    {
      EBGraph& fineEBGraph       =  a_fineEBIS.m_graph[dit()];
      const EBGraph& coarEBGraph = coarFromFineEBGraph[dit()];

      coarEBGraph.fixFineToCoarse(fineEBGraph);
    }

}

/******************/
void EBISLevel::coarsenFaces(EBISLevel& a_fineEBIS)
{
  CH_TIME("EBISLevel::coarsenFaces");
  //now make a fine ebislayout with two ghost cell
  //on the same mapping as m_dbl
  //so that i can do the vofs and faces of this level.
  //this one will have the fine from coarse stuff fixed
  DisjointBoxLayout fineFromCoarDBL;
  refine(fineFromCoarDBL, m_grids, 2);
  fineFromCoarDBL.close();

  //no need for ghost cells here
  EBGraphFactory ebgraphfactfine(a_fineEBIS.m_domain);
  EBGraphFactory ebgraphfactcoar(m_domain);
  LevelData<EBGraph> fineEBGraphGhostLD(fineFromCoarDBL,1,3*IntVect::Unit, ebgraphfactfine);
  Interval interv(0,0);
  a_fineEBIS.m_graph.copyTo(interv, fineEBGraphGhostLD, interv);
  LevelData<EBGraph> coarEBGraphGhostLD(m_grids,        1,  IntVect::Unit, ebgraphfactcoar);
  m_graph.copyTo(           interv, coarEBGraphGhostLD, interv);

  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      const EBGraph& fineEBGraphGhost = fineEBGraphGhostLD[dit()];
      const EBGraph& coarEBGraphGhost = coarEBGraphGhostLD[dit()];
      EBGraph& coarEBGraph = m_graph[dit()];
      coarEBGraph.coarsenFaces(coarEBGraphGhost, fineEBGraphGhost);
    }
  //redefine coarebghostgraphld so i can use the faces for the ebdata
  coarEBGraphGhostLD.define(m_grids, 1,  IntVect::Unit, ebgraphfactcoar);
  m_graph.copyTo(interv, coarEBGraphGhostLD, interv);

  EBDataFactory ebdatafact;
  LevelData<EBData> fineEBDataGhostLD(fineFromCoarDBL,1, 2*IntVect::Unit, ebdatafact);
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      Box localBox = grow(fineFromCoarDBL.get(dit()), 2);
      localBox &= a_fineEBIS.m_domain;
      fineEBDataGhostLD[dit()].defineVoFData(fineEBGraphGhostLD[dit()], localBox);;
      fineEBDataGhostLD[dit()].defineFaceData(fineEBGraphGhostLD[dit()], localBox);
    }
  a_fineEBIS.m_data.copyTo(interv, fineEBDataGhostLD, interv);

  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      const EBData&   fineEBData      = fineEBDataGhostLD[dit()];
      const EBGraph& fineEBGraphGhost = fineEBGraphGhostLD[dit()];
      const EBGraph& coarEBGraphGhost = coarEBGraphGhostLD[dit()];

      EBData& coarEBData   = m_data[dit()];
      coarEBData.coarsenFaces(fineEBData,  fineEBGraphGhost, coarEBGraphGhost, m_grids.get(dit()));
    }

}

/******************/
EBISLevel::EBISLevel(EBISLevel             & a_fineEBIS,
                     const GeometryService & a_geoserver,
                     const EBIndexSpace    * const a_ebisPtr,
                     const bool            & a_distributedData,
                     const bool            & a_fixRegularNextToMultiValued)
{ // method used by EBIndexSpace::buildNextLevel
  CH_TIME("EBISLevel::EBISLevel_fineEBIS");

  m_cacheMisses = 0;
  m_cacheHits   = 0;
  m_cacheStale  = 0;

  m_domain = coarsen(a_fineEBIS.m_domain,2);
  m_dx = 2.*a_fineEBIS.m_dx;
  m_tolerance = 2.*a_fineEBIS.m_tolerance;
  m_origin = a_fineEBIS.m_origin;

  if (!a_distributedData)
    { // this is the original method

      Vector<Box> vbox;
      Vector<long> irregCount;
      {
        CH_TIME("EBISLevel::EBISLevel_fineEBIS_makeboxes 2");
        makeBoxes(vbox, irregCount, m_domain.domainBox(), m_domain, a_geoserver,
                  m_origin, m_dx, a_ebisPtr->getNCellMax(), a_ebisPtr);
      }

      //pout()<<vbox<<"\n\n";
      //load balance the boxes
      Vector<int> procAssign;
      LoadBalance(procAssign, irregCount, vbox);

      //pout()<<procAssign<<std::endl;
      //define the layout.  this includes the domain and box stuff
      m_grids.define(vbox, procAssign);//this should use m_domain for periodic

    }
  else
    {
      // permit the geometry service to construct a layout
      int nCellMax = a_ebisPtr->getNCellMax();
      (const_cast<GeometryService*>(&a_geoserver))->makeGrids(m_domain, m_grids, nCellMax, 15);
    }


  EBGraphFactory ebgraphfact(m_domain);
  m_graph.define(m_grids, 1, IntVect::Zero, ebgraphfact);

  EBDataFactory ebdatafact;
  m_data.define(m_grids, 1, IntVect::Zero, ebdatafact);

  //create coarsened vofs from fine.
  coarsenVoFs(a_fineEBIS);

  //overallMemoryUsage();
  //create coarse faces from fine
  coarsenFaces(a_fineEBIS);
  //overallMemoryUsage();
  //fix the regular next to the multivalued cells
  //to be full irregular cells
  //  dumpDebug(string("EBIS::before FRNTM"));
  if (a_fixRegularNextToMultiValued)
    {
      fixRegularNextToMultiValued();
    }
  //  dumpDebug(string("EBIS::after FRNTM"));

  //overallMemoryUsage();
  // fix the fine->coarseVoF thing.
  fixFineToCoarse(a_fineEBIS);
}

/******************/
EBISLevel::~EBISLevel()
{
}

/****************/
void EBISLevel::fillEBISLayout(EBISLayout&              a_ebisLayout,
                               const DisjointBoxLayout& a_grids,
                               const int&               a_nghost) const
{
  CH_assert(a_nghost >= 0);
a_ebisLayout.define(m_domain, a_grids, a_nghost, m_graph, m_data);
return;
  dmap& dcache = m_cache[a_nghost];
  EBISLayout& l = dcache[a_grids];
  if (!l.isDefined())
    {
      l.define(m_domain, a_grids, a_nghost, m_graph, m_data);
      m_cacheMisses++;
      m_cacheStale++;
    }
  else
    {
      m_cacheHits++;
    }
  a_ebisLayout = l;
  if (m_cacheStale == 1)
    {
      refreshCache();
      m_cacheStale = 0;
    }
}

void EBISLevel::refreshCache() const
{
  for (std::map<int, dmap>::iterator p = m_cache.begin(); p!= m_cache.end(); ++p)
    {
      for (dmap::iterator d = p->second.begin(); d != p->second.end(); ++d)
        {
          if (d->first.refCount() == 1)
            {
              p->second.erase(d);
            }
        }
    }
}

void EBISLevel::clearMultiBoundaries()
{
  CH_TIME("EBISLevel::clearMultiBoundaries");
  DataIterator dit = m_grids.dataIterator();
  for (dit.begin(); dit.ok(); ++dit)
    {
      EBData& data = m_data[dit];
      data.clearMultiBoundaries();
    }
}

void EBISLevel::setBoundaryPhase(int phase)
{
  CH_TIME("EBISLevel::setBoundaryPhase");
  DataIterator dit = m_grids.dataIterator();
  for (dit.begin(); dit.ok(); ++dit)
    {
      EBData& data = m_data[dit];
      data.setBoundaryPhase(phase);
    }
}

//throughout this routine A refers to objects belonging to this
//ebindexspace,  B refers to the other one.
void setOtherVoFSingleValued(VolData&        a_vol,
                             const VolIndex& a_sourceVoF,
                             int&            a_otherPhase,
                             EBData&         a_otherFluidData,
                             bool&           a_sourceIvContainedInOtherFluid,
                             bool&           a_skipCell)
{
  VolIndex vother = a_sourceVoF;
  a_skipCell = false;
  bool fixBoundaryData = false;
  VolData refVol;
  //if a cell is full and has a unit boundary area, it means that the irregular
  //boundary lives on a cell boundary.  We have to link to a cell over.
  // In this case, in the presence of multi valued cells, to quote Yeats:
  //Mere anarchy is loosed upon the world,
  //The blood-dimmed tide is loosed, and everywhere
  //The ceremony of innocence is drowned.
  if ((a_vol.m_volFrac == 1) && (a_vol.m_averageFace.m_bndryArea == 1))
    {
      a_skipCell = true;
      //figure out which cell we are moving to. Then we use cellIndex = 0
      //because the cells involved are single valued.
      int whichWayToGo = -23;
      int sign = 1;
      bool found = false;
      for (int idir = 0; idir < SpaceDim; idir++)
        {
          Real normDir = a_vol.m_averageFace.m_normal[idir];
          if (Abs(normDir) == 1)
            {
              found = true;
              whichWayToGo = idir;
              sign = -1;
              if (normDir < 0)
                {
                  sign = 1;
                }
            }
        }
      if (!found)
        {
          MayDay::Error("EBIndexSpace::setOtherVoFSingleValued - Unit normal direction not found");
        }
      IntVect ivOther = sign*BASISV(whichWayToGo);
      ivOther += a_sourceVoF.gridIndex();
      vother = VolIndex(ivOther, 0);
    }
  else if ((a_vol.m_volFrac == 0) &&
           (a_vol.m_averageFace.m_bndryArea == 1) &&
           (a_sourceIvContainedInOtherFluid))
    {
      a_skipCell = true;
      refVol = a_otherFluidData.getVolData()(a_sourceVoF, 0);
      // use -1*normal of the VoF with this IV in the other fluid
      RealVect normal = -refVol.m_averageFace.m_normal;
      //figure out which cell we are moving to. Then we use cellIndex = 0
      //because the cells involved are single valued.
      int whichWayToGo = -23;
      int sign = 1;
      bool found = false;
      for (int idir = 0; idir < SpaceDim; idir++)
        {
          Real normDir = normal[idir];
          if (Abs(normDir) == 1)
            {
              found = true;
              whichWayToGo = idir;
              sign = 1;
              if (normDir < 0)
                {
                  sign = -1;
                }
            }
        }
      if (found)
        {
          IntVect ivOther = sign*BASISV(whichWayToGo);
          ivOther += a_sourceVoF.gridIndex();
          vother = VolIndex(ivOther, 0);
          fixBoundaryData = true;
        }
      else
        {
          MayDay::Warning("EBIndexSpace::setOtherVoFSingleValued - Unit normal direction not found; EB probably intersects cell corner");
        }
    }
  a_vol.m_phaseFaces[0].m_volIndex   = vother;
  a_vol.m_phaseFaces[0].m_bndryPhase = a_otherPhase;
  if (fixBoundaryData)
    {
      Real eps = 1e-6;
      BoundaryData&    boundaryData0 = a_vol.m_phaseFaces[0];
      BoundaryData& avgBoundaryData0 = a_vol.m_averageFace;
      BoundaryData&    boundaryData1 = refVol.m_averageFace;
      if ((boundaryData0.m_normal.vectorLength() < eps) &&
          (boundaryData1.m_normal.vectorLength() > 1-eps) &&
          (boundaryData1.m_normal.vectorLength() < 1+eps))
        {
          // fix boundary at phaseFace
          boundaryData0.m_normal = -1.0*(boundaryData1.m_normal);
          boundaryData0.m_bndryArea = boundaryData1.m_bndryArea;
          boundaryData0.m_bndryCentroid = boundaryData1.m_bndryCentroid;
          // make average face reflect new phaseFace data
          avgBoundaryData0.m_normal = boundaryData0.m_normal;
          avgBoundaryData0.m_bndryArea = boundaryData0.m_bndryArea;
          avgBoundaryData0.m_bndryCentroid = boundaryData0.m_bndryCentroid;
        }
    }
}

void testAndFixBoundaryData(VolData&       a_volData0,
                            const VolData& a_volData1)
{
  Real eps = 1e-6;
  BoundaryData&    boundaryData0 = a_volData0.m_phaseFaces[0];
  BoundaryData& avgBoundaryData0 = a_volData0.m_averageFace;
  const BoundaryData&    boundaryData1 = a_volData1.m_phaseFaces[0];
  //  const BoundaryData& avgBoundaryData1 = a_volData1.m_averageFace;
  if ((boundaryData0.m_bndryArea > 0) &&
      (boundaryData0.m_normal.vectorLength() < eps) &&
      (boundaryData1.m_normal.vectorLength() > 1-eps) &&
      (boundaryData1.m_normal.vectorLength() < 1+eps))
    {
      // fix boundary at phaseFace
      boundaryData0.m_normal = -1.0*(boundaryData1.m_normal);
      boundaryData0.m_bndryArea = boundaryData1.m_bndryArea;
      boundaryData0.m_bndryCentroid = boundaryData1.m_bndryCentroid;
      // make average face reflect new phaseFace data
      avgBoundaryData0.m_normal = boundaryData0.m_normal;
      avgBoundaryData0.m_bndryArea = boundaryData0.m_bndryArea;
      avgBoundaryData0.m_bndryCentroid = boundaryData0.m_bndryCentroid;
    }
}
void EBISLevel::reconcileIrreg(EBISLevel& a_otherPhase)
{
  CH_TIME("EBISLevel::reconcileIrreg");

  //both layouts are made independently so will not work
  //with the same data iterator but they ought to because they
  //are both breaking up the domain the same way.   This is yet
  //another subtle thing thing that will break when the
  //number of fluids is greater than two.

  //define ghosted coarse graph so i can use the faces for the ebdata
  EBGraphFactory ebgraphfactcoarA(m_domain);
  LevelData<EBGraph> ghostedEBGraphLDCoarA(m_grids, 1, IntVect::Unit,
                                           ebgraphfactcoarA);
  Interval interv(0, 0);
  m_graph.copyTo(interv, ghostedEBGraphLDCoarA, interv);

  EBGraphFactory ebgraphfactcoarB(a_otherPhase.m_domain);
  LevelData<EBGraph> ghostedEBGraphLDCoarB(a_otherPhase.m_grids,
                                           1,
                                           IntVect::Unit,
                                           ebgraphfactcoarB);
  a_otherPhase.m_graph.copyTo(interv, ghostedEBGraphLDCoarB, interv);

  DataIterator dita = m_grids.dataIterator();
  DataIterator ditb= a_otherPhase.m_grids.dataIterator();
  dita.begin(); ditb.begin();
  for ( ; dita.ok(); ++dita, ++ditb)
    {
      EBGraph& ebgrapCoarA =              m_graph[dita];
      EBGraph& ebgrapCoarB = a_otherPhase.m_graph[ditb];
      EBData&  ebdataCoarA =              m_data[dita];
      EBData&  ebdataCoarB = a_otherPhase.m_data[ditb];
      Box region = ebgrapCoarA.getRegion();

      // ghosted graphs for filling faces for ebdata
      EBGraph& ghostedEBGraphCoarA = ghostedEBGraphLDCoarA[dita];
      EBGraph& ghostedEBGraphCoarB = ghostedEBGraphLDCoarB[ditb];

      CH_assert(ebgrapCoarB.getRegion() == region);
      IntVectSet seta = ebgrapCoarA.getIrregCells(region);
      IntVectSet setb = ebgrapCoarB.getIrregCells(region);

      IntVectSet abDifference = seta - setb;
      IntVectSet baDifference = setb - seta;
      //this should only happen when one side changed a regular to an
      //irregular.   This means that the other has to change a covered
      //to an irregular
      ebgrapCoarB.addEmptyIrregularVoFs(abDifference);
      ebgrapCoarA.addEmptyIrregularVoFs(baDifference);
      // repeat add for ghosted graphs
      ghostedEBGraphCoarB.addEmptyIrregularVoFs(abDifference);
      ghostedEBGraphCoarA.addEmptyIrregularVoFs(baDifference);
      // use ghosted graphs to add to ebdata
      ebdataCoarB.addEmptyIrregularVoFs(abDifference, ghostedEBGraphCoarB);
      ebdataCoarA.addEmptyIrregularVoFs(baDifference, ghostedEBGraphCoarA);
    } //end loop over grids
}

void EBISLevel::levelStitch(EBISLevel&       a_otherPhase,
                            const EBISLevel* a_finePtrA,
                            const EBISLevel* a_finePtrB)
{
  CH_TIME("EBISLevel::levelStitch");

  //I have no idea what to do if only one of the inputs is null.
  //either both or neither makes sense
  CH_assert(((a_finePtrA != NULL) && (a_finePtrB != NULL)) ||
            ((a_finePtrA == NULL) && (a_finePtrB == NULL)));
  EBISLayout ebislFineA, ebislFineB;
  DisjointBoxLayout dblFineA, dblFineB;
  if (a_finePtrA != NULL)
    {
      int nghost = 0; //should not need any as this is all about connectivity within a coarse cell
      refine(dblFineA,              m_grids, 2);
      refine(dblFineB, a_otherPhase.m_grids, 2);
      a_finePtrA->fillEBISLayout(ebislFineA, dblFineA, nghost);
      a_finePtrB->fillEBISLayout(ebislFineB, dblFineB, nghost);
    }

  //both layouts are made independently so will not work
  //with the same data iterator but they ought to because they
  //are both breaking up the domain the same way.   This is yet
  //another subtle thing thing that will break when the
  //number of fluids is greater than two.
  DataIterator dita = m_grids.dataIterator();
  DataIterator ditb= a_otherPhase.m_grids.dataIterator();
  dita.begin(); ditb.begin();
  for ( ; dita.ok(); ++dita, ++ditb)
    {
      EBGraph& ebgrapCoarA =              m_graph[dita];
      EBGraph& ebgrapCoarB = a_otherPhase.m_graph[ditb];
      EBData&  ebdataCoarA =              m_data[dita];
      EBData&  ebdataCoarB = a_otherPhase.m_data[ditb];
      // Box region = ebgrapCoarA.getRegion();
      Box region = m_grids[dita];

      // CH_assert(ebgrapCoarB.getRegion() == region);
      CH_assert(a_otherPhase.m_grids[ditb] == region);
      IntVectSet seta = ebgrapCoarA.getIrregCells(region);
      IntVectSet setb = ebgrapCoarB.getIrregCells(region);

      //different EBIS's can (correctly) disagree about which cells are multivalued
      IntVectSet setMultiA = ebgrapCoarA.getMultiCells(region);
      IntVectSet setMultiB = ebgrapCoarB.getMultiCells(region);
      IntVectSet setMulti = setMultiA | setMultiB;
      int aphase =              m_phase;
      int bphase = a_otherPhase.m_phase;

      IntVectSet setANoMulti = seta - setMulti;
      IntVectSet setBNoMulti = setb - setMulti;
      {
        //EBIndexSpace does the right thing with all the geometric information
        //in the case of two single valued vofs.   All that needs to be set is
        //the phase on the other side of the irregular face.
        for (IVSIterator ita(setANoMulti); ita.ok(); ++ita)
          {
            VolIndex v(ita(), 0); //0 because we know this whole set is single valued

            VolData& volDatA = ebdataCoarA.getVolData()(v,0);
            volDatA.m_phaseFaces.resize(1);
            //this sets boundary area and normal and centroid to
            //whatever the ebindexspace set it to.
            volDatA.m_phaseFaces[0]=volDatA.m_averageFace;
            bool skip = false;
            bool vContainedInOtherIVS = setBNoMulti.contains(v.gridIndex());
            setOtherVoFSingleValued(volDatA, v, bphase, ebdataCoarB, vContainedInOtherIVS, skip);
            if (skip && setMultiB.contains(volDatA.m_phaseFaces[0].m_volIndex.gridIndex()))
              {
                MayDay::Error("coordinate face boundary also a multi valued one");
              }
          }//end loop over cells  where both phases are singlevalued

        for (IVSIterator itb(setBNoMulti); itb.ok(); ++itb)
          {
            VolIndex v(itb(), 0); //0 because we know this whole set is single valued

            VolData& volDatB = ebdataCoarB.getVolData()(v,0);
            volDatB.m_phaseFaces.resize(1);
            //this sets boundary area and normal and centroid to
            //whatever the ebindexspace set it to.
            volDatB.m_phaseFaces[0]=volDatB.m_averageFace;
            bool skip = false;
            bool vContainedInOtherIVS = setANoMulti.contains(v.gridIndex());
            setOtherVoFSingleValued(volDatB, v, aphase, ebdataCoarA, vContainedInOtherIVS, skip);
            if (skip && setMultiA.contains(volDatB.m_phaseFaces[0].m_volIndex.gridIndex()))
              {
                MayDay::Error("coordinate face boundary also a multi valued one");
              }
          }//end loop over cells where both phases are singlevalued

        // now fix incorrect boundary data, where possible
        // first, for the phase a volData
//         for (IVSIterator ita(setANoMulti); ita.ok(); ++ita)
        // hack to get around mismatching sets
        IntVectSet abIntersectionNoMulti = setANoMulti & setBNoMulti;
        for (IVSIterator ita(abIntersectionNoMulti); ita.ok(); ++ita)
          {
            VolIndex v(ita(), 0);

            VolData& volDatA = ebdataCoarA.getVolData()(v,0);
            VolIndex vother = volDatA.m_phaseFaces[0].m_volIndex;
            // need to fix the normal
            VolData& volDatB = ebdataCoarB.getVolData()(vother,0);
            // only send the first boundary data, since single valued cell
            testAndFixBoundaryData(volDatA, volDatB);
          }
        // next, for the phase b volData
//         for (IVSIterator itb(setBNoMulti); itb.ok(); ++itb)
        // hack to get around mismatching sets
        for (IVSIterator itb(abIntersectionNoMulti); itb.ok(); ++itb)
          {
            VolIndex v(itb(), 0); //0 because we know this whole set is single valued

            VolData& volDatB = ebdataCoarB.getVolData()(v,0);
            VolIndex vother = volDatB.m_phaseFaces[0].m_volIndex;
            // need to fix the normal
            VolData& volDatA = ebdataCoarA.getVolData()(vother,0);
            // only send the first boundary data, since single valued cell
            testAndFixBoundaryData(volDatB, volDatA);
          }
      }
      if (!setMulti.isEmpty())
        {
          //now have to do the union of the each set
          //of multivalued cells.   either phase being multivalued can
          //make either ebindex space get the wrong answer for
          //the geometric information
          const EBISBox& ebisbxFineA = ebislFineA[dita()];
          const EBISBox& ebisbxFineB = ebislFineB[ditb()];

          IVSIterator it(setMulti); //see above derivation.
          for (it.begin(); it.ok(); ++it)
            {
              cellStitch(ebdataCoarA, ebdataCoarB,
                         ebgrapCoarA, ebgrapCoarB,
                         ebisbxFineA, ebisbxFineB,
                         it(), aphase, bphase);
            }//end loop over multivalued cells
        }
    } //end loop over grids
}

/*****/
void getFinerBoundaryData(Vector<Vector<BoundaryData> > & a_bndryDataFineA,
                          Vector<VolIndex>&               a_otherCellIndex,
                          const VolIndex&                 a_coarVoFA,
                          const EBGraph&                  a_ebgrapCoarA,
                          const EBGraph&                  a_ebgrapCoarB,
                          const EBISBox&                  a_ebisbxFineA)
{
  //divdes fine boundary datas a by which coarse vof b they are connected to.
  // to which they are connected?

  const IntVect ivCoar = a_coarVoFA.gridIndex();

  Vector<VolIndex>          coarVoFsB = a_ebgrapCoarB.getVoFs(ivCoar);
  Vector<Vector<VolIndex> > fineVoFsB(coarVoFsB.size());
  a_bndryDataFineA.resize(coarVoFsB.size());
  a_otherCellIndex.resize(coarVoFsB.size());
  for (int ib = 0; ib < coarVoFsB.size(); ib++)
    {
      fineVoFsB[ib] = a_ebgrapCoarB.refine(coarVoFsB[ib]);
      a_otherCellIndex[ib] = coarVoFsB[ib];
    }

  Vector<VolIndex> allFinerVoFsA = a_ebgrapCoarA.refine(a_coarVoFA);
  for (int ib = 0; ib < coarVoFsB.size(); ib++)
    {
      a_bndryDataFineA[ib].resize(0);
      for (int ifineB = 0; ifineB < fineVoFsB[ib].size(); ifineB++)
        {
          const VolIndex& vofFineB = fineVoFsB[ib][ifineB];
          for (int ifineA = 0; ifineA < allFinerVoFsA.size(); ifineA++)
            {
              const VolIndex& vofFineA = allFinerVoFsA[ifineA];
              if (vofFineA.gridIndex() == vofFineB.gridIndex())
                {
                  if (a_ebisbxFineA.isIrregular(vofFineA.gridIndex()))
                    {
                      const VolData& voldat = a_ebisbxFineA.getEBData().getVolData()(vofFineA, 0);
                      const Vector<BoundaryData>& boundaryDat = voldat.m_phaseFaces;
                      for (int iface = 0; iface < boundaryDat.size(); iface++)
                        {
                          if (boundaryDat[iface].m_volIndex.cellIndex() == vofFineB.cellIndex())
                            {
                              a_bndryDataFineA[ib].push_back(boundaryDat[iface]);
                            }
                        }
                    }
                }
            }
        }
    }
}

/*****/
void coarsenBoundaryInfo(EBData&                                a_ebdataCoar,
                         const EBISBox&                         a_ebisbxFine,
                         const Vector< Vector<BoundaryData > >& a_finerBoundaryData,
                         const VolIndex&                        a_vof,
                         const int&                             a_otherPhase,
                         const Vector<VolIndex>&                a_otherCellIndex)
{
  //this coarsening factor for boundary area fractions can be thought of this way
  //(1) take fine area and multiply by dxf^{SD-1}
  //(2) add up areas (now we have the real coarse area).
  //(3) divide by dxc^{SD-1} = dxf{SD-1}*2{SD-1}
  Real faceCoarsenFactor = D_TERM(1.0, * 2.0,* 2.0);

  //the point of this routine is to fix the boundary area and normal
  //stuff in this object
  VolData& coarData = a_ebdataCoar.getVolData()(a_vof, 0);
  coarData.m_phaseFaces.resize(a_finerBoundaryData.size());
  for (int ib = 0; ib < a_finerBoundaryData.size(); ib++)
    {
      //initialization is important  in both of these things.
      //since i am taking the average
      RealVect aveNormal  = RealVect::Zero;
      RealVect aveCentro  = RealVect::Zero;
      Real sumArea=0;
      Real aveArea=0;
      int numfaces=0;
      for (int jb = 0; jb < a_finerBoundaryData[ib].size(); jb++)
        {
          const BoundaryData& boundaryDat = a_finerBoundaryData[ib][jb];
          const Real    & fineArea = boundaryDat.m_bndryArea;
          const RealVect& fineNorm = boundaryDat.m_normal;
          const RealVect& fineCent = boundaryDat.m_bndryCentroid;
          sumArea += fineArea;
          for (int idir = 0; idir < SpaceDim; idir++)
            {
              aveNormal[idir] += fineArea*fineNorm[idir];
              aveCentro[idir] += fineArea*fineCent[idir];
            }
          numfaces++;
        }

      //we are taking an average-weighted average
      //of each normal and centroid components so we need to divide by the area
      if (sumArea > 1.0e-12)
        {
          aveNormal /= sumArea;
          aveCentro /= sumArea;
        }
      Real sumSq;
      PolyGeom::unifyVector(aveNormal, sumSq);
      //this takes into account the fact that boundary areas are normalized
      //by dx^(SD-1).  See note above.
      aveArea  = sumArea/faceCoarsenFactor;

      coarData.m_phaseFaces[ib].m_normal            = aveNormal;
      coarData.m_phaseFaces[ib].m_bndryCentroid     = aveCentro;
      coarData.m_phaseFaces[ib].m_bndryArea         = aveArea;
      coarData.m_phaseFaces[ib].m_bndryPhase        = a_otherPhase;
      coarData.m_phaseFaces[ib].m_volIndex          = a_otherCellIndex[ib];
    }

}

/***/
void EBISLevel::cellStitch(EBData&        a_ebdataCoarA,
                           EBData&        a_ebdataCoarB,
                           const EBGraph& a_ebgrapCoarA,
                           const EBGraph& a_ebgrapCoarB,
                           const EBISBox& a_ebisbxFineA,
                           const EBISBox& a_ebisbxFineB,
                           const IntVect& a_iv,
                           const int&     a_phaseA,
                           const int&     a_phaseB)
{
  //pout() << "entering cellStitch" << endl;
  Vector<VolIndex> coarVoFsA = a_ebgrapCoarA.getVoFs(a_iv);
  Vector<VolIndex> coarVoFsB = a_ebgrapCoarB.getVoFs(a_iv);
  for (int ivof = 0; ivof < coarVoFsA.size(); ivof++)
    {
      const VolIndex& vof = coarVoFsA[ivof];
      // each element in finerVoFs is a list of finer vofs whose
      // opposing number on the other side of the irregular boundary
      // are connected to each other.   This will correspond with
      // the Vector<VolIndex> of the coarse vofs in the *other* phase.
      Vector<Vector< BoundaryData> > finerBndryData;
      Vector<VolIndex>  cellIndexB;
      getFinerBoundaryData(finerBndryData, cellIndexB, vof, a_ebgrapCoarA, a_ebgrapCoarB, a_ebisbxFineA);


      //once the above list is made, we can correctly coarsen the boundary
      //information using only the information in this phase.
      coarsenBoundaryInfo(a_ebdataCoarA, a_ebisbxFineA, finerBndryData, vof, a_phaseB, cellIndexB);
    }

  for (int ivof = 0; ivof < coarVoFsB.size(); ivof++)
    {
      const VolIndex& vof = coarVoFsB[ivof];
      // each element in finerVoFs is a list of finer vofs whose
      // opposing number on the other side of the irregular boundary
      // are connected to each other.   This will correspond with
      // the Vector<VolIndex> of the coarse vofs in the *other* phase.
      Vector<Vector< BoundaryData> > finerBndryData;
      Vector<VolIndex>  cellIndexA;
      getFinerBoundaryData(finerBndryData, cellIndexA, vof, a_ebgrapCoarB, a_ebgrapCoarA, a_ebisbxFineB);


      //once the above list is made, we can correctly coarsen the boundary
      //information using only the information in this phase.
      coarsenBoundaryInfo(a_ebdataCoarB, a_ebisbxFineB, finerBndryData, vof, a_phaseA, cellIndexA);
    }
}

/******************/
bool EBIndexSpace::isDefined() const
{
  return m_isDefined;
}

/******************/
int EBIndexSpace::getNCellMax() const
{
  return m_nCellMax;
}

/******************/
#ifdef CH_USE_HDF5
void EBIndexSpace::define(HDF5Handle & a_handle,
                          int          a_maxCoarsenings)
{
  CH_TIME("EBIndexSpace::define_hdf5handle");

  clear();

  //read in ncellmax
  HDF5HeaderData header;
  header.readFromFile(a_handle);
  int nCellMax = header.m_int["EBIS_ncellMax"];

  //read in finest level
  //coarser levels are derived from graph coarsening
  EBISLevel* level0 = new EBISLevel(a_handle);

  define(level0, nCellMax, a_maxCoarsenings);
}

/******************/
void EBIndexSpace::write(HDF5Handle&   a_handle,
                         ProblemDomain a_outputLevel) const
{
  CH_TIME("EBIndexSpace::write");
  //read in ncellmax
  HDF5HeaderData header;
  std::string filedescriptor("EBIndexSpace");
  header.m_string ["filetype"]      = filedescriptor;
  header.m_int["EBIS_ncellMax"] = m_nCellMax;
  header.writeToFile(a_handle);

  int ilev = 0;
  if (!a_outputLevel.isEmpty())
    {
      ilev = getLevel(a_outputLevel);
    }
  //write out finest level
  //coarser levels are derived from graph coarsening
  m_ebisLevel[ilev]->write(a_handle);
}
#endif

void EBIndexSpace::define(EBISLevel * a_level0,
                          int         a_nCellMax,
                          int         a_maxCoarsenings)
{
  CH_TIME("EBIndexSpace::define_ebislevel0");

  m_nCellMax = a_nCellMax;
  m_isDefined = true;

  const ProblemDomain& fineDomain = a_level0->getDomain();

  //figure out how deep we can go
  m_nlevels = 1;
  bool canref = (fineDomain == refine(coarsen(fineDomain,2), 2));
  CH_assert(!fineDomain.isEmpty());
  ProblemDomain refbox = fineDomain;
  while (canref)
    {
      ProblemDomain refcoarbox = coarsen(refbox,2);
      refcoarbox.refine(2);
      if (refcoarbox != refbox)
        {
          canref = false;
        }
      else
        {
          m_nlevels++;
          refbox.coarsen(2);
        }
    }
  if (a_maxCoarsenings != -1)
    {
      CH_assert(a_maxCoarsenings >= 0);
      if (m_nlevels > a_maxCoarsenings+1) m_nlevels = a_maxCoarsenings + 1;
    }

  m_ebisLevel.resize(m_nlevels, NULL);
  m_domainLevel.resize(m_nlevels);

  ProblemDomain  domLevel = fineDomain;
  a_level0->clearMultiBoundaries();
  m_ebisLevel[0] = a_level0;
  m_domainLevel[0] = domLevel;
  AllRegularService dummy;
  for (int ilev = 1; ilev < m_nlevels; ilev++)
    {
      domLevel.coarsen(2);
      m_domainLevel[ilev] = domLevel;
      m_ebisLevel[ilev] = new EBISLevel(*m_ebisLevel[ilev-1], dummy, this);
      m_ebisLevel[ilev]->clearMultiBoundaries();
    }

#ifndef NDEBUG
  for (int ilev = 0; ilev < m_nlevels; ilev++)
    {
      m_ebisLevel[ilev]->sanityCheck(this);
    }
#endif
}

/******************/
void EBIndexSpace::define(const ProblemDomain    & a_domain,
                          const RealVect         & a_origin,
                          const Real             & a_dx,
                          const GeometryService  & a_geoserver,
                          int                      a_nCellMax,
                          int                      a_maxCoarsenings)
{
  CH_TIME("EBIndexSpace::define_geoserver_domain0");

  buildFirstLevel(a_domain, a_origin, a_dx, a_geoserver, a_nCellMax, a_maxCoarsenings);
  m_ebisLevel[0]->clearMultiBoundaries();
  EBISLevel* n =  m_ebisLevel[0];
#ifdef CH_MPI
  {
    CH_TIME("barrier");
    MPI_Barrier(Chombo_MPI::comm);
  }
#endif
  while (n)
    {
      n->clearMultiBoundaries();
      n=buildNextLevel(a_geoserver);
    }

#ifndef NDEBUG
  for (int ilev = 0; ilev < m_nlevels; ilev++)
    {
      m_ebisLevel[ilev]->sanityCheck(this);
    }
#endif
}

void EBIndexSpace::define(const ProblemDomain                        & a_entireDomain,
                          const RealVect                             & a_origin,
                          const Real                                 & a_dx,
                          const Vector<RefCountedPtr<EBIndexSpace> > & a_patches,
                          const Vector<IntVect>                      & a_offsets,
                          int                                          a_maxCoarsenings)
{
  CH_TIME("EBIndexSpace::define_stitching");

  CH_assert(a_patches.size() == a_offsets.size());

  int numPatches = a_patches.size();

  if (numPatches > 0)
  {
    Vector<Box> allBoxes;
    Vector<int> allProcIDs;

    int numLevels = a_patches[0]->m_nlevels;

    for (int iPatch = 0; iPatch < numPatches; iPatch++)
    {
      const RefCountedPtr<EBIndexSpace> & curPatch  = a_patches[iPatch];
      const IntVect                     & curOffset = a_offsets[iPatch];

      const DisjointBoxLayout & curPatchDBL = curPatch->m_ebisLevel[0]->m_grids;

      Vector<Box> curPatchBoxes   = curPatchDBL.boxArray();
      // Vector<int> curPatchProcIDs = curPatchDBL.procIDs();

      int numBoxes = curPatchBoxes.size();

      for (int iBox = 0; iBox < numBoxes; iBox++)
      {
        Box curBox = curPatchBoxes[iBox] + curOffset;

        if (!a_entireDomain.contains(curBox))
        {
          MayDay::Error("EBIndexSpace::stitchPatches - Entire domain doesn't contain a shifted box from a sub-domain");
        }

        curPatchBoxes[iBox] = curBox;
      }

      allBoxes.append(curPatchBoxes);
    }

    pout() << "Total boxes: " << allBoxes.size() << endl;

    LoadBalance(allProcIDs,allBoxes);

    DisjointBoxLayout entireDBL(allBoxes,allProcIDs);

    this->m_nCellMax  = a_patches[0]->m_nCellMax;
    this->m_isDefined = a_patches[0]->m_isDefined;

    this->m_distributedData = a_patches[0]->m_distributedData;

    this->m_nlevels = numLevels;

    this->m_domainLevel.resize(numLevels);
    this->m_domainLevel[0] = a_entireDomain;

    this->m_ebisLevel.resize(numLevels);

    for (int iLevel = 0; iLevel < numLevels; iLevel++)
    {
      this->m_ebisLevel[iLevel] = NULL;
    }

    const EBISLevel* patchZeroEBISLevel = a_patches[0]->m_ebisLevel[0];
    EBISLevel* finestEBISLevel = new EBISLevel();

    finestEBISLevel->m_phase = patchZeroEBISLevel->m_phase;

    finestEBISLevel->m_grids = entireDBL;

    finestEBISLevel->m_domain = a_entireDomain;

    finestEBISLevel->m_origin = a_origin;
    finestEBISLevel->m_dx     = a_dx;

    finestEBISLevel->m_tolerance = patchZeroEBISLevel->m_tolerance;

    LevelData<EBGraph> & finestNewGraph = finestEBISLevel->m_graph;

    EBGraphFactory graphFactory(finestEBISLevel->m_domain);
    finestNewGraph.define(finestEBISLevel->m_grids,
                          patchZeroEBISLevel->m_graph.nComp(),
                          patchZeroEBISLevel->m_graph.ghostVect(),
                          graphFactory);

    for (int iPatch = 0; iPatch < numPatches; iPatch++)
    {
      const RefCountedPtr<EBIndexSpace> & curPatch  = a_patches[iPatch];
      const IntVect                     & curOffset = a_offsets[iPatch];

      const EBISLevel* curEBISLevel = curPatch->m_ebisLevel[0];
      const LevelData<EBGraph> & finestCurGraph = curEBISLevel->m_graph;

      EBGraphFactory curGraphFactory(curEBISLevel->m_domain);
      LevelData<EBGraph> offsetCurGraph;

      offsetCurGraph.define(finestCurGraph,curGraphFactory);

      for (DataIterator dit = offsetCurGraph.disjointBoxLayout().dataIterator(); dit.ok(); ++dit)
      {
        RefCountedPtr<EBGraphImplem> curGraphImplem = offsetCurGraph[dit()].m_implem;

        curGraphImplem->m_region += curOffset;
        curGraphImplem->m_domain = a_entireDomain;

        BaseFab<GraphNode> & graph = curGraphImplem->m_graph;

        graph.shift(curOffset);

        for (BoxIterator bit(graph.box()); bit.ok(); ++bit)
        {
          const IntVect & iv = bit();

          GraphNode & graphNode = graph(iv);

          if (graphNode.isIrregular())
          {
            int numVoFs = graphNode.m_cellList->size();

            if (numVoFs > 1)
            {
              MayDay::Error("EBIndexSpace::stitchPatches - EBIndexSpace's being stitched can't have multivalued cells");
            }

            GraphNodeImplem & graphNodeImplem = (*(graphNode.m_cellList))[0];

            for (int idir = 0; idir < SpaceDim; idir++)
            {
              for (SideIterator sit; sit.ok(); ++sit)
              {
                const Side::LoHiSide side = sit();

                IntVect ivOther(iv);
                ivOther.shift(idir,sign(side));

                int arcValue = -1;
                if (a_entireDomain.contains(ivOther))
                {
                  arcValue = 0;
                }

                Vector<int>& arcs = graphNodeImplem.m_arc[graphNodeImplem.index(idir,side)];
                int numArcs = arcs.size();

                if (numArcs > 1)
                {
                  MayDay::Error("EBIndexSpace::stitchPatches - EBIndexSpace's being stitched can't have multiple faces on a side");
                }

                if (numArcs == 1)
                {
                  arcs[0] = arcValue;
                }
              }
            }
          }
        }

        if (curGraphImplem->m_irregIVS != NULL)
        {
          curGraphImplem->m_irregIVS->shift(curOffset);
        }

        if (curGraphImplem->m_multiIVS != NULL)
        {
          curGraphImplem->m_multiIVS->shift(curOffset);
        }

        curGraphImplem->m_mask.shift(curOffset);
      }

      bool exchange = false;
      Copier offsetCopier(offsetCurGraph.disjointBoxLayout(),
                          finestNewGraph.disjointBoxLayout(),
                          exchange,
                          curOffset);
      offsetCurGraph.copyTo(finestNewGraph,offsetCopier);
    }

#if 0
    LevelData<EBGraph> ghostGraph(finestEBISLevel->m_grids,
                                  1,
                                  IntVect::Unit,
                                  graphFactory);

    Interval interval(0,0);
    finestEBISLevel->m_graph.copyTo(interval,ghostGraph,interval);

    EBDataFactory dataFactory;
    finestEBISLevel->m_data.define(finestEBISLevel->m_grids,
                                   1,
                                   IntVect::Zero,
                                   dataFactory);

    for (DataIterator dit = finestEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
    {
      finestEBISLevel->m_data[dit()].defineVoFData(ghostGraph[dit()],
                                                   finestEBISLevel->m_grids.get(dit()));
      finestEBISLevel->m_data[dit()].defineFaceData(ghostGraph[dit()],
                                                    finestEBISLevel->m_grids.get(dit()));
    }

    for (int iPatch = 0; iPatch < numPatches; iPatch++)
    {
      const RefCountedPtr<EBIndexSpace> & curPatch  = a_patches[iPatch];
      const IntVect                     & curOffset = a_offsets[iPatch];

      const EBISLevel* curEBISLevel = curPatch->m_ebisLevel[0];
      const LevelData<EBData> & finestCurData = curEBISLevel->m_data;

      EBGraphFactory curGraphFactory(curEBISLevel->m_domain);
      LevelData<EBGraph> offsetCurGraph;

      offsetCurGraph.define(finestCurGraph,curGraphFactory);
    }
#endif

    finestEBISLevel->m_cache.clear();

    finestEBISLevel->m_cacheMisses = 0;
    finestEBISLevel->m_cacheHits   = 0;
    finestEBISLevel->m_cacheStale  = 0;

    this->m_ebisLevel[0] = finestEBISLevel;
  }
}

EBISLevel* EBIndexSpace::buildFirstLevel(const ProblemDomain&   a_domain,
                                         const RealVect&        a_origin,
                                         const Real&            a_dx,
                                         const GeometryService& a_geoserver,
                                         int a_nCellMax,
                                         int a_maxCoarsenings,
                                         bool a_fixRegularNextToMultiValued)
{
  CH_TIME("EBIndexSpace::buildFirstLevel");
  clear();
  m_isDefined = true;

  if (a_nCellMax > 0)
    {
      m_nCellMax = a_nCellMax;
    }
  else
    {
      if (SpaceDim == 2)
        {
          m_nCellMax = 32;
        }
      else
        {
          m_nCellMax = 32;
        }
    }
  m_nlevels = 1;
  bool canref = (a_domain == refine(coarsen(a_domain,2), 2));

  CH_assert(!a_domain.isEmpty());
  ProblemDomain refbox = a_domain;

  while (canref)
    {
      ProblemDomain refcoarbox = coarsen(refbox,2);
      refcoarbox.refine(2);
      if (refcoarbox != refbox)
        {
          canref = false;
        }
      else
        {
          m_nlevels++;
          refbox.coarsen(2);
        }
    }
  if (a_maxCoarsenings != -1)
    {
      CH_assert(a_maxCoarsenings >= 0);
      if (m_nlevels > a_maxCoarsenings+1) m_nlevels = a_maxCoarsenings + 1;
    }

  m_ebisLevel.resize(m_nlevels, NULL);
  m_domainLevel.resize(m_nlevels);

  ProblemDomain  domLevel = a_domain;
  m_ebisLevel[0] = new EBISLevel(domLevel,
                                 a_origin,
                                 a_dx,
                                 a_geoserver,
                                 this,
                                 m_distributedData,
                                 a_fixRegularNextToMultiValued);
  m_domainLevel[0] = domLevel;
  return m_ebisLevel[0];
}

void EBIndexSpace::resetLevels(int nLevel)
{
  CH_TIME("EBIndexSpace::resetLevels");

  CH_assert(nLevel <= m_nlevels);

  for (int i = nLevel; i < m_nlevels; i++)
    {
      if (m_ebisLevel[i] != NULL) delete m_ebisLevel[i];
      m_ebisLevel[i] = NULL;
    }

  m_nlevels = nLevel;

}

EBISLevel* EBIndexSpace::buildNextLevel(const GeometryService & a_geoserver,
                                        bool                    a_fixRegularNextToMultiValued)
{
  CH_TIME("EBIndexSpace::buildNextLevel");
  int ilev=0;
  for ( ; ilev <m_ebisLevel.size(); ++ilev)
    {
      if (m_ebisLevel[ilev] == NULL) break;
    }
  if (ilev == m_ebisLevel.size()) return NULL;

  m_domainLevel[ilev] = m_domainLevel[ilev-1];
  m_domainLevel[ilev].coarsen(2);
  m_ebisLevel[ilev] = new EBISLevel(*m_ebisLevel[ilev-1],
                                    a_geoserver,
                                    this,
                                    m_distributedData,
                                    a_fixRegularNextToMultiValued);
  return m_ebisLevel[ilev];
}

/******************/
const ProblemDomain& EBISLevel::getDomain() const
{
  return m_domain;
}

/******************/
const Real& EBISLevel::getDX() const
{
  return m_dx;
}

/******************/
const RealVect& EBISLevel::getOrigin() const
{
  return m_origin;
}

/******************/
const DisjointBoxLayout& EBISLevel::getGrids() const
{
  return m_grids;
}

// All boxes containing an irregular cell
DisjointBoxLayout EBISLevel::getIrregGrids(const ProblemDomain& a_domain) const
{
  DisjointBoxLayout irregGrids;
  DataIterator dit =   m_graph.dataIterator();
  Vector<Box> localBoxes;

  for (dit.begin(); dit.ok(); ++dit)
    {
      const EBGraph& graph = m_graph[dit()];
      const Box& b         = m_graph.box(dit());
      if (graph.hasIrregular())
        {
          localBoxes.push_back(b);

        }
    }
  Vector<Vector<Box> > allBoxes;

  gather(allBoxes, localBoxes, 0);

  broadcast(allBoxes, 0);

  Vector<Box> boxes;
  for (int i = 0; i < allBoxes.size(); i++)
    {
      boxes.append(allBoxes[i]);
    }
  Vector<int> procs;
  LoadBalance(procs, boxes);
  irregGrids = DisjointBoxLayout(boxes, procs, a_domain);

  return irregGrids;
}

// All boxes with only irregular or regular cells
DisjointBoxLayout EBISLevel::getFlowGrids(const ProblemDomain& a_domain) const
{
  DisjointBoxLayout flowGrids;
  DataIterator dit =   m_graph.dataIterator();
  Vector<Box> localBoxes;

  for (dit.begin(); dit.ok(); ++dit)
    {
      const EBGraph& graph = m_graph[dit()];
      const Box& b         = m_graph.box(dit());
      if (graph.hasIrregular() || graph.isAllRegular())
        {
          localBoxes.push_back(b);

        }
    }
  Vector<Vector<Box> > allBoxes;

  gather(allBoxes, localBoxes, 0);

  broadcast(allBoxes, 0);
  Vector<Box> boxes;
  for (int i = 0; i < allBoxes.size(); i++)
    {
      boxes.append(allBoxes[i]);
    }
  Vector<int> procs;
  LoadBalance(procs, boxes);

  flowGrids = DisjointBoxLayout(boxes, procs, a_domain);
  return flowGrids;
}

// All boxes with an irregular or covered cell
DisjointBoxLayout EBISLevel::getCoveredGrids(const ProblemDomain& a_domain) const
{
  DisjointBoxLayout coveredGrids;
  DataIterator dit = m_graph.dataIterator();
  Vector<Box> localBoxes;

  for (dit.begin(); dit.ok(); ++dit)
    {
      const EBGraph& graph = m_graph[dit()];
      const Box& b         = m_graph.box(dit());
      if (graph.hasIrregular() || graph.isAllCovered())
        {
          localBoxes.push_back(b);
        }
    }
  Vector<Vector<Box> > allBoxes;

  gather(allBoxes, localBoxes, 0);

  broadcast(allBoxes, 0);

  Vector<Box> boxes;
  for (int i = 0; i < allBoxes.size(); i++)
    {
      boxes.append(allBoxes[i]);
    }
  Vector<int> procs;
  LoadBalance(procs, boxes);
  coveredGrids = DisjointBoxLayout(boxes, procs, a_domain);

  return coveredGrids;
}

IntVectSet EBISLevel::irregCells() const
{
  DataIterator dit =   m_graph.dataIterator();
  IntVectSet rtn;

  for (dit.begin(); dit.ok(); ++dit)
    {
      const EBGraph& graph = m_graph[dit()];
      const Box& b         = m_graph.box(dit());
      if (graph.hasIrregular())
        {
          rtn |= graph.getIrregCells(b);
        }
    }
  return rtn;
}

#ifdef CH_USE_HDF5
void EBIndexSpace::writeInfo(HDF5Handle& handle) const
{
  Vector<DisjointBoxLayout> vectGrids(m_nlevels);
  Vector<LevelData<FArrayBox>* >  vectData(m_nlevels);
  Vector<string> vectNames(1,"unknown");
  ProblemDomain domain = m_domainLevel[m_nlevels-1];
  const EBISLevel& level = *(m_ebisLevel[m_nlevels-1]);
  Real dx = level.getDX();
  Vector<int> vectRatio(m_nlevels, 2);

  for (int i = 0; i < m_nlevels; ++i)
    {
      const EBISLevel& level = *(m_ebisLevel[m_nlevels-1-i]);
      vectGrids[i] = level.getGrids();
      vectData[i] = new LevelData<FArrayBox>(level.getGrids(), 1);
      LevelData<FArrayBox>& ld = *(vectData[i]);
      for (DataIterator dit = ld.dataIterator(); dit.ok(); ++dit)
        {
          ld[dit].setVal(0.0);
        }
    }


  WriteAMRHierarchyHDF5(handle, vectGrids, vectData, vectNames, domain.domainBox(),
                        dx, 1.0, 0.0 , vectRatio, m_nlevels);

  for (int i = 0; i < m_nlevels; ++i)
    {

      delete vectData[i];

    }
}
#endif

/******************/
int EBIndexSpace::numLevels() const
{
  return m_nlevels;
}

void EBIndexSpace::clear()
{
  for (int ilev = 0; ilev < m_ebisLevel.size(); ilev++)
    {
      delete m_ebisLevel[ilev];
      m_ebisLevel[ilev] = NULL;
    }
  m_ebisLevel.resize(0);
  m_domainLevel.resize(0);
  m_nlevels = 0;
  m_isDefined = false;
}

/******************/
EBIndexSpace::EBIndexSpace()
{
  m_distributedData = false;
  //appalling hack to get stuff working again
  BaseIFFAB<FaceData>::setSurroundingNodeSemantic(false);
}

/******************/
EBIndexSpace::~EBIndexSpace()
{
  clear();
}

/******************/
int EBIndexSpace::getLevel(const ProblemDomain& a_domain) const
{
  bool found = false;
  int whichlev = -1;
  for (int ilev = 0; ilev < m_domainLevel.size() && !found; ilev++)
    {
      if (m_domainLevel[ilev].domainBox() == a_domain.domainBox())
        {
          found = true;
          whichlev = ilev;
        }
    }
  return whichlev;
}

/****************/
void EBIndexSpace::fillEBISLayout(EBISLayout&              a_ebisLayout,
                                  const DisjointBoxLayout& a_grids,
                                  const ProblemDomain&     a_domain,
                                  const int&               a_nghost) const
{
  CH_assert(isDefined());
  CH_TIME("EBIndexSpace::fillEBISLayout");

  //figure out which level we are on
  int whichlev = getLevel(a_domain);
  if (whichlev < 0)
    {
      pout() << "a_domain = " << a_domain
             << " does not correspond to any refinement of EBIS" << endl;
      MayDay::Error("Bad argument to EBIndexSpace::fillEBISLayout");
    }
  m_ebisLevel[whichlev]->fillEBISLayout(a_ebisLayout, a_grids, a_nghost);
  a_ebisLayout.setEBIS(this); //need for mf
}

// Divide the EBIndexSpace into connected components and return a Vector of
// disjoint EBIndexSpace's corresponding to each connected component.
Vector<RefCountedPtr<EBIndexSpace> > EBIndexSpace::findConnectedComponents(int        & a_numComponents,
                                                                           const bool & a_onlyBiggest)
{
  CH_TIME("EBIndexSpace::connectedComponents");

  // Begin by setting up some data holders and other infrastructure to number
  // the connected components and remember the renumbering of the VoFs.
  Vector<LevelData<EBCellFAB>* > numberedComponentses(m_nlevels,NULL);
  Vector<LevelData<EBCellFAB>* > newNumberings(m_nlevels,NULL);

  // EBISLayout's for each level
  Vector<EBISLayout> ebisLayouts(m_nlevels);

  // All our temporary data structures have one component
  int nComps = 1;

  // All our temporary data structures have one layer of ghostcells
  int nGhosts = 1;
  IntVect ghostCells = nGhosts * IntVect::Unit;

  // Fill all the EBISLayouts and allocate/define all the local data holders
  for (int ilev = 0; ilev < m_nlevels; ilev++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop1");

    EBISLevel* curEBISLevel = m_ebisLevel[ilev];

    DisjointBoxLayout curGrids = curEBISLevel->m_grids;

    EBISLayout& curEBISLayout = ebisLayouts[ilev];
    curEBISLevel->fillEBISLayout(curEBISLayout,curGrids,nGhosts);

    EBCellFactory curFactory(curEBISLayout);

    LevelData<EBCellFAB>* numberedComponents = new LevelData<EBCellFAB>();
    LevelData<EBCellFAB>* newNumbering       = new LevelData<EBCellFAB>();

    numberedComponents->define(curGrids,nComps,ghostCells,curFactory);
    newNumbering      ->define(curGrids,nComps,ghostCells,curFactory);

    numberedComponentses[ilev] = numberedComponents;
    newNumberings       [ilev] = newNumbering;
  }

  // Get the coarsest level and number the connected components at that level.
  // The current implementation assumes there is only one box at the coarsest
  // level - this is checked below.
  EBISLevel* coarEBISLevel = m_ebisLevel[m_nlevels-1];

  DisjointBoxLayout coarGrids = coarEBISLevel->m_grids;

  // This really only needs to be an "int" and not a "Real" but all our
  // infrastructure (e.g., I/O, debugging, library functions) is set up to
  // work with "EBCellFAB".  This can be changed if needed to be an "int" data
  // holder.
  LevelData<EBCellFAB>& coarNumberedComponents = *numberedComponentses[m_nlevels-1];

  // Initialize all the component numbers to -1 (invalid)
  EBLevelDataOps::setVal(coarNumberedComponents,-1.0);

  // Currently this code will only work if there is only on box at the
  // coarsest level of the geometry.  This will need to be fixed by merging
  // different numberings of the same component in different boxes.  This can
  // be done via ghost cells, exchanges, the building of equivalence classes,
  // and, finally, a renumbering operation.
  unsigned int numGrids = coarGrids.size();

  if (numGrids > 1)
  {
    pout() << "Number of coarsest grids: " << numGrids << endl;
    pout() << "Grids: " << coarGrids << endl;
    MayDay::Error("EBIndexSpace::connectedComponents - algorithm currently only works if there is only one box at the coarsest geometry level");
  }

  // For counting the new EBIndexspace's and remember the number assigned to
  // each
  int numEBIS = 0;
  Vector<int> valueEBIS;

  // Used if "a_onlyBiggest" is true to get the biggest connected component
  Real maxVolFrac = 0.0;
  int biggestEBIS = -1;

  // Go through all (one) boxes (see comments and MayDay above).
  for (DataIterator dit = coarGrids.dataIterator(); dit.ok(); ++dit)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop2");

    // Get a bunch of local references
    int boxIndex = coarGrids.index(dit());

    const Box& curBox = coarGrids.get(dit());
    const IntVectSet curIVS(curBox);

    EBCellFAB&     curEBCellFAB = coarNumberedComponents[dit()];
    const EBGraph& curEBGraph   = coarEBISLevel->m_graph[dit()];
    const EBISBox& curEBISBox   = curEBCellFAB.getEBISBox();

    unsigned int curNum = boxIndex;

    // Iterate through the box, VoF by VoF and number anything that isn't
    // already numbered.
    for (VoFIterator vofit(curIVS,curEBGraph); vofit.ok(); ++vofit)
    {
      // Current VoF
      const VolIndex& vof = vofit();

      Real totalVolFrac;

      // Examine the connected component starting at "vof".  If it hasn't been
      // numbered yet, return true otherwise false.  Also, return the
      // volume fraction associated with the connect component.
      if (setAllConnectedVoFs(totalVolFrac,curEBCellFAB,curEBGraph,curEBISBox,vof,vof,curNum))
      {
        if (totalVolFrac > 0.0)
        {
          // If a connected component was numbered, check to see for the
          // numbering can be incremented and then increment it.
          if (curNum + numGrids < curNum)
          {
            MayDay::Error("EBIndexSpace::connectedComponents - Component index overflow");
          }

          // Remember the component number and increment the number of
          // components
          valueEBIS.push_back(curNum);
          numEBIS++;

          // Find the biggest connected component
          if (totalVolFrac > maxVolFrac)
          {
            maxVolFrac = totalVolFrac;
            biggestEBIS = curNum;
          }

          // New number of the next component (if there is one)
          curNum += numGrids;
        }
        else
        {
          // If we're not counting the current component (because it has a total
          // volume fraction of 0.0, set the component number back to -1

          resetAllConnectedVoFs(curEBCellFAB,curEBGraph,curEBISBox,vof,vof);
        }
      }
    }
  }

#ifdef CH_MPI
  // Sum up the numEBIS on all processors
  int sumNumEBIS;

  int result = MPI_Allreduce(&numEBIS, &sumNumEBIS, 1, MPI_INT, MPI_SUM, Chombo_MPI::comm);

  if (result != MPI_SUCCESS)
  {
    MayDay::Error("Communication error summing 'numEBIS'");
  }

  numEBIS = sumNumEBIS;

  typedef struct
  {
    double maxVolFrac;
    int    biggestEBIS;
  } CH_MPI_MAXLOC;

  CH_MPI_MAXLOC localMax;
  CH_MPI_MAXLOC globalMax;

  // Get the connected component index of the largest connected component on
  // all processors
  localMax.maxVolFrac  = maxVolFrac;
  localMax.biggestEBIS = biggestEBIS;

  result = MPI_Allreduce(&localMax, &globalMax, 1, MPI_DOUBLE_INT, MPI_MAXLOC, Chombo_MPI::comm);

  if (result != MPI_SUCCESS)
  {
    MayDay::Error("Communication error maximizing 'maxVolFrac'");
  }

  maxVolFrac  = globalMax.maxVolFrac;
  biggestEBIS = globalMax.biggestEBIS;
#endif

  a_numComponents = numEBIS;

  int minEBIS;
  int maxEBIS;

  if (a_onlyBiggest)
  {
    // If we only want the biggest connected componnent, only iterate over
    // that components index
    if (biggestEBIS >= 0)
      {
        minEBIS = biggestEBIS;
        maxEBIS = biggestEBIS;
      }
    else
      {
        // This corresponds to an empty EBIS
        minEBIS =  0;
        maxEBIS = -1;
      }
  }
  else
  {
    // Otherwise, iterate over all the connected component indices
    minEBIS = 0;
    maxEBIS = numEBIS-1;
  }

  // Make sure all ghostcells are correct
  coarNumberedComponents.exchange();

  // At this point, renumber the connected components from 0 to numEBIS-1.
  // Currently, this is already the case so nothing happens here.

  // There is only one component in each temporary data holder
  int comp = 0;

  // The refinement ratio on the geometry size of things is always 2
  int nRef = 2;

  // Pass the numbering for connected components from coarse to fine levels.
  for (int ilev = m_nlevels-2; ilev >= 0; ilev--)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop3");

    // Get a lot of local references to get things ready to go
    EBISLevel* coarEBISLevel = m_ebisLevel[ilev+1];
    EBISLevel* fineEBISLevel = m_ebisLevel[ilev];

    DisjointBoxLayout coarGrids = coarEBISLevel->m_grids;
    DisjointBoxLayout fineGrids = fineEBISLevel->m_grids;

    const ProblemDomain& coarDomain = coarEBISLevel->m_domain;

    EBISLayout& coarEBISLayout = ebisLayouts[ilev+1];
    EBISLayout& fineEBISLayout = ebisLayouts[ilev];

    LevelData<EBCellFAB>& coarNumberedComponents = *numberedComponentses[ilev+1];
    LevelData<EBCellFAB>& fineNumberedComponents = *numberedComponentses[ilev];


    // Initial all the numberings on this level to -1 (invalid)
    EBLevelDataOps::setVal(fineNumberedComponents,-1.0);

    // Set up a piecewise constant interpolator from the current coarse level
    // to the current fine level
    EBCFCopy constantInterp(fineGrids,
                            coarGrids,
                            fineEBISLayout,
                            coarEBISLayout,
                            coarDomain,
                            nRef,
                            nComps,
                            this,
                            ghostCells);

    // Do the interpolation/copy
    Interval oneComp(comp,comp);
    constantInterp.copy(fineNumberedComponents,
                        coarNumberedComponents,
                        oneComp);

    // Make sure all ghostcells are correct
    fineNumberedComponents.exchange();
  }

  // The set of EBIndexSpace's that are each connected components of current
  // EBIndexSpace.
  Vector<RefCountedPtr<EBIndexSpace> > connectedEBIS(numEBIS);

  // Make copies of the main EBIndexSpace for each connected component.
  for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop4");

    // Make a new, empty EBIndexSpace for the current component
    RefCountedPtr<EBIndexSpace> copyEBIS(new EBIndexSpace());

    // Set the member data that is not level dependent
    copyEBIS->m_nCellMax  = m_nCellMax;
    copyEBIS->m_isDefined = m_isDefined;

    copyEBIS->m_distributedData = m_distributedData;

    copyEBIS->m_domainLevel = m_domainLevel;
    copyEBIS->m_nlevels     = m_nlevels;

    // Go through the levels setting the level dependent member data
    for (int ilev = 0; ilev < m_nlevels; ilev++)
    {
      // Original level dependent member data
      EBISLevel* origEBISLevel = m_ebisLevel[ilev];

      // New level dependent member data for this component
      EBISLevel* copyEBISLevel = new EBISLevel();

      // Copy everything except the EBData (this happens at the end)
      copyEBISLevel->m_phase = iEBIS;

      copyEBISLevel->m_grids  = origEBISLevel->m_grids;

      copyEBISLevel->m_domain = origEBISLevel->m_domain;

      copyEBISLevel->m_origin = origEBISLevel->m_origin;
      copyEBISLevel->m_dx     = origEBISLevel->m_dx;

      copyEBISLevel->m_tolerance = origEBISLevel->m_tolerance;

      EBGraphFactory graphFactory(origEBISLevel->m_domain);
      copyEBISLevel->m_graph.define(origEBISLevel->m_graph,graphFactory);

      // Clear this cache and all statistics associated with it
      copyEBISLevel->m_cache.clear();

      copyEBISLevel->m_cacheMisses = 0;
      copyEBISLevel->m_cacheHits   = 0;
      copyEBISLevel->m_cacheStale  = 0;

      // Remember this EBISLevel
      copyEBIS->m_ebisLevel.push_back(copyEBISLevel);
    }

    // Remember this EBIndexSpace
    connectedEBIS[iEBIS] = copyEBIS;
  }

  // Are there any cells which are regular with a multivalued parent
  bool anyRegularWithMultivaluedParent = false;

  // First pass to correct the EBIndexSpace for each conneceted component -
  // Remove AllRegular patches and regular cells that are in other connected
  // components, and mark all multiVoFs that are in other connected components
  for (int ilev = 0; ilev < m_nlevels; ilev++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop5");

    // The original EBISLevel
    EBISLevel* origEBISLevel = m_ebisLevel[ilev];

    // All the component numbers for this level
    const LevelData<EBCellFAB>& numberedComponents = *numberedComponentses[ilev];

    // All the EBISLevel's for the connected components
    Vector<EBISLevel*> copyEBISLevel(numEBIS);
    for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
    {
      copyEBISLevel[iEBIS] = connectedEBIS[iEBIS]->m_ebisLevel[ilev];
    }

    // Iterate through all the boxes
    for (DataIterator dit = origEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
    {
      // Get some local references
      const Box& curBox = origEBISLevel->m_grids.get(dit());

      const EBCellFAB& numberedComponentsFAB = numberedComponents[dit()];

      const EBGraphImplem& origEBGraph = *(origEBISLevel->m_graph[dit()].m_implem);

      // If the original graph for this box was all regular
      if (origEBGraph.m_tag == EBGraphImplem::AllRegular)
      {
        const IntVect& oneIVInside = curBox.smallEnd();
        const int vofID = 0;
        const VolIndex vof(oneIVInside,vofID);

        // Get the component number for this (all regular) box
        Real componentNumber = numberedComponentsFAB(vof,comp);

        // Go through all the connected components and mark their graphs for
        // this box as "all covered" if they don't match "componentNumber"
        for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
        {
          if (iEBIS != componentNumber)
          {
            EBGraphImplem& copyEBGraph = *(copyEBISLevel[iEBIS]->m_graph[dit()].m_implem);
            copyEBGraph.m_tag = EBGraphImplem::AllCovered;
          }
        }
      }
      // If the original graph has irregular cells in this box
      else if (origEBGraph.m_tag == EBGraphImplem::HasIrregular)
      {
        // Iterate through each IntVect in this box
        IntVectSet curIVS(curBox);
        for (IVSIterator ivsit(curIVS); ivsit.ok(); ++ivsit)
        {
          // Current IntVect and it's GraphNode
          const IntVect& iv = ivsit();
          const GraphNode& origGraphNode = origEBGraph.m_graph(iv);

          // If the original GraphNode is all regular with a single valued
          // parent
          if (origGraphNode.isRegularWithSingleValuedParent())
          {
            const int vofID = 0;
            const VolIndex vof(iv,vofID);

            // Get the component number for this "vof"
            Real componentNumber = numberedComponentsFAB(vof,comp);

            // Go through all the connected components and mark their "vof" as
            // covered if they don't match "componentNumber"
            for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
            {
              if (iEBIS != componentNumber)
              {
                EBGraphImplem& copyEBGraph = *(copyEBISLevel[iEBIS]->m_graph[dit()].m_implem);
                copyEBGraph.m_graph(iv).m_cellList = (Vector<GraphNodeImplem>*)0;
              }
            }
          }
          // If the original GraphNode is anything else except covered
          else if (origGraphNode.hasValidCellList())
          {
            // Iterate through all the cellIndex's in the original GraphNode
            for (int iVoF = 0; iVoF < origGraphNode.size(); iVoF++)
            {
              // The current VoF
              const VolIndex vof(iv,iVoF);

              // Get the component number for this "vof"
              Real componentNumber = numberedComponentsFAB(vof,comp);

              // For each new EBIndexSpace, go to the GraphNodeImplem
              // corresponding to the current VoF and mark it "valid" if it
              // matches the current "componentNumber", otherwise mark it
              // "invalid"
              for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
              {
                EBGraphImplem&   copyEBGraph         = *(copyEBISLevel[iEBIS]->m_graph[dit()].m_implem);
                GraphNode&       copyGraphNode       = copyEBGraph.m_graph(iv);
                GraphNodeImplem& copyGraphNodeImplem = (*(copyGraphNode.m_cellList))[iVoF];

                if (copyGraphNodeImplem.m_isRegular)
                {
                  anyRegularWithMultivaluedParent = true;
                }

                if (iEBIS != componentNumber)
                {
                  copyGraphNodeImplem.m_isValid = false;
                }
                else
                {
                  copyGraphNodeImplem.m_isValid = true;
                }
              }
            }
          }
        }
      }
    }
  }

  // Second pass to fill the data holder with the new numbering for each
  // multiVoF in each connected component.  These can coexist in one (old)
  // data holder because the connected components are disjoint subsets of
  // the old graph.
  for (int ilev = 0; ilev < m_nlevels; ilev++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop6");

    // Data holder for the new numbering on this level
    LevelData<EBCellFAB>& newNumbering = *newNumberings[ilev];

    // Initial all the numberings on this level to 0
    EBLevelDataOps::setVal(newNumbering,0.0);

    // Go through each of the connected components
    for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
    {
      // Get the current EBIndexSpace and EBISLevel
      EBIndexSpace& curEBIS = *(connectedEBIS[iEBIS]);
      EBISLevel* curEBISLevel = curEBIS.m_ebisLevel[ilev];

      // Iterate through all the boxes on this level
      for (DataIterator dit = curEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
      {
        // Get some local references
        const Box& curBox = curEBISLevel->m_grids.get(dit());

        EBCellFAB& newNumberingFAB = newNumbering[dit()];

        const EBGraphImplem& curEBGraph = *(curEBISLevel->m_graph[dit()].m_implem);

        // If the current graph has irregular cells then some renumbering may
        // be needed
        if (curEBGraph.m_tag == EBGraphImplem::HasIrregular)
        {
          // Iterate through all IntVect's in the current box
          IntVectSet curIVS(curBox);
          for (IVSIterator ivsit(curIVS); ivsit.ok(); ++ivsit)
          {
            const IntVect& iv = ivsit();
            const GraphNode& curGraphNode = curEBGraph.m_graph(iv);

            // If the graph has an explicit entry, i.e. a valid cellList, then
            // some changes may be needed
            if (curGraphNode.hasValidCellList())
            {
              // Go through the VoFs, if they are valid then record the
              // current validVoFCount (this VoF's new number in this
              // connected component) in the correct location in the data
              // holder based on the original graph and increment
              // validVoFCount.
              int validVoFCount = 0;
              for (int iVoF = 0; iVoF < curGraphNode.size(); iVoF++)
              {
                GraphNodeImplem& curGraphNodeImplem = (*(curGraphNode.m_cellList))[iVoF];
                if (curGraphNodeImplem.m_isValid == true)
                {
                  const VolIndex vof(iv,iVoF);

                  newNumberingFAB(vof,comp) = validVoFCount;
                  validVoFCount++;
                }
              }
            }
          }
        }
      }
    }

    // Make sure the ghostcells are correct on this level
    newNumbering.exchange();
  }

  // Third pass to correct the EBIndexSpace for each conneceted component on
  // each level (but not between levels) - Go through correcting all
  // references in the connected component using the map created in the second
  // pass.  Then prune away all parts of the graph that are not needed for
  // this connected component.
  for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop7");

    EBIndexSpace& curEBIS = *(connectedEBIS[iEBIS]);

    // Go through each level
    for (int ilev = 0; ilev < m_nlevels; ilev++)
    {
      // Get the EBISLevel and new numbering for this lelvel
      EBISLevel* curEBISLevel = curEBIS.m_ebisLevel[ilev];
      LevelData<EBCellFAB>& newNumbering = *newNumberings[ilev];

      // Go through all the boxes in this level
      for (DataIterator dit = curEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
      {
        const Box& curBox = curEBISLevel->m_grids.get(dit());
        EBCellFAB& newNumberingFAB = newNumbering[dit()];
        EBGraphImplem& curEBGraph = *(curEBISLevel->m_graph[dit()].m_implem);

        // If the graph in this box contain irregular cells, there may to
        // something to do
        if (curEBGraph.m_tag == EBGraphImplem::HasIrregular)
        {
          // Assume the new graph will be all covered unless we find a valid
          // VoF still in this box
          bool allCovered = true;

          // Iterator through all the IntVect's in this box looking for a
          // valid VoF
          IntVectSet curIVS(curBox);
          for (IVSIterator ivsit(curIVS); ivsit.ok(); ++ivsit)
          {
            const IntVect& iv = ivsit();
            GraphNode& curGraphNode = curEBGraph.m_graph(iv);

            // Nothing to fix but the graph isn't all covered
            if (curGraphNode.isRegularWithSingleValuedParent())
            {
              allCovered = false;
            }
            // If there are VoF's, go through them looking for valid ones and
            // correct the arcs connected that VoF to adjacent VoF's
            else if (curGraphNode.hasValidCellList())
            {
              // Get the old cellList and start building the new cellList
              Vector<GraphNodeImplem>* oldCellList = curGraphNode.m_cellList;
              Vector<GraphNodeImplem>* newCellList = new Vector<GraphNodeImplem>;

              // Go through all the VoF's
              for (int iVoF = 0; iVoF < curGraphNode.size(); iVoF++)
              {
                GraphNodeImplem& oldGraphNodeImplem = (*oldCellList)[iVoF];

                // If this VoF is valid correct the arcs to adjacent VoF's
                if (oldGraphNodeImplem.m_isValid == true)
                {
                  // Look in each direction at the lo and hi sides
                  for (int idir = 0; idir < SpaceDim; idir++)
                  {
                    for (SideIterator sit; sit.ok(); ++sit)
                    {
                      // Correct (in place) all the arcs for all the faces
                      // on all the sides
                      int index = oldGraphNodeImplem.index(idir,sit());

                      IntVect otherIV = iv;
                      otherIV.shift(idir,sign(sit()));

                      Vector<int>& curArcs = oldGraphNodeImplem.m_arc[index];
                      int numArcs = curArcs.size();

                      for (int iFace = 0; iFace < numArcs; iFace++)
                      {
                        if (curArcs[iFace] != -1)
                        {
                          VolIndex otherVoF(otherIV,curArcs[iFace]);
                          curArcs[iFace] = newNumberingFAB(otherVoF,comp);
                        }
                      }
                    }
                  }

                  // Save the new VoF
                  newCellList->push_back(oldGraphNodeImplem);
                }
              }

              // Delete the old cellList and save the new one
              delete oldCellList;
              oldCellList = newCellList;

              // Check the new number of VoF's
              int numCells = oldCellList->size();

              // If there are no VoF's, delete the cellList, mark the IntVect
              // location as covered, and remove the IntVect from the
              // irregular and multivalued IntVectSet's
              if (numCells == 0)
              {
                delete oldCellList;
                oldCellList = ((Vector<GraphNodeImplem>*) 0);

                (*curEBGraph.m_irregIVS) -= iv;
                (*curEBGraph.m_multiIVS) -= iv;
              }
              // If there are some VoF's
              else
              {
                // The graph in this box isn't all covered
                allCovered = false;

                // If there is only one VoF
                if (numCells == 1)
                {
                  // And it's not regular
                  if ((*oldCellList)[0].m_isRegular == false)
                  {
                    // Add it to the irregular IntVectSet and remove it from
                    // the multivalued IntVectSet
                    (*curEBGraph.m_irregIVS) |= iv;
                    (*curEBGraph.m_multiIVS) -= iv;
                  }
                }
              }

              // Save the updated cellList
              curGraphNode.m_cellList = oldCellList;
            }
          }

          // If the graph in this box (for this connected component) has
          // become all covered clean things up
          if (allCovered)
          {
            // Mark it as all covered
            curEBGraph.m_tag = EBGraphImplem::AllCovered;

            // Clear all the GraphNode's
            curEBGraph.m_graph.clear();

            // Delete the irregular and multivalued IntVectSet's
            if (curEBGraph.m_irregIVS != NULL)
            {
              delete curEBGraph.m_irregIVS;
              curEBGraph.m_irregIVS = NULL;
            }

            if (curEBGraph.m_multiIVS != NULL)
            {
              delete curEBGraph.m_multiIVS;
              curEBGraph.m_multiIVS = NULL;
            }
          }
        }
      }
    }
  }

  // Fourth pass to correct all the cell indices for the parent of each VoF.
  // To do this it is necessary to make a coarsened version of the current
  // (fine) level to place the data from the next coarser level (which may
  // have an incompatible DisjointBoxLayout).
  for (int ilev = 0; ilev < m_nlevels-1; ilev++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop8");

    // Get the new numberings for the parents of the current (fine) level
    LevelData<EBCellFAB>& newCoarNumbering = *newNumberings[ilev+1];

    // Coarsen the fine DisjointBoxLayout, create a temporary data holder
    // based on that, and copy the new numberings of the parents into this new
    // data holder
    const DisjointBoxLayout& fineGrids = newNumberings[ilev]->disjointBoxLayout();

    DisjointBoxLayout coarGrids;
    coarsen(coarGrids,fineGrids,nRef);

    EBISLayout coarEBISLayout;
    m_ebisLevel[ilev+1]->fillEBISLayout(coarEBISLayout,coarGrids,nGhosts);

    EBCellFactory coarFactory(coarEBISLayout);

    LevelData<EBCellFAB> newCoarsenedFineNumbering(coarGrids,nComps,ghostCells,coarFactory);

    newCoarNumbering.copyTo(newCoarsenedFineNumbering);

    // Go through all the connected components and fix the references to the
    // parents in their graphs
    for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
    {
      EBIndexSpace& curEBIS = *(connectedEBIS[iEBIS]);
      EBISLevel* fineEBISLevel = curEBIS.m_ebisLevel[ilev];

      // Iterate through all the boxes on this level making the corrections
      for (DataIterator dit = fineEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
      {
        const Box& fineBox = fineEBISLevel->m_grids.get(dit());
        EBCellFAB& newNumberingFAB = newCoarsenedFineNumbering[dit()];
        EBGraphImplem& fineEBGraph = *(fineEBISLevel->m_graph[dit()].m_implem);

        // If there are irregular cells, there may be something to correct
        if (fineEBGraph.m_tag == EBGraphImplem::HasIrregular)
        {
          // Go through all the IntVect's in the current box
          IntVectSet fineIVS(fineBox);
          for (IVSIterator ivsit(fineIVS); ivsit.ok(); ++ivsit)
          {
            const IntVect& fineIV = ivsit();
            IntVect coarIV = coarsen(fineIV,nRef);

            GraphNode& fineGraphNode = fineEBGraph.m_graph(fineIV);

            // If there is a valid cellList then there will be references to
            // a parent VoF that need to be corrected
            if (fineGraphNode.hasValidCellList())
            {
              Vector<GraphNodeImplem>* fineCellList = fineGraphNode.m_cellList;

              // Correct the parent references in all the VoF's
              for (int iVoF = 0; iVoF < fineGraphNode.size(); iVoF++)
              {
                GraphNodeImplem& fineGraphNodeImplem = (*fineCellList)[iVoF];

                VolIndex coarVoF(coarIV,fineGraphNodeImplem.m_coarserNode);

                fineGraphNodeImplem.m_coarserNode = newNumberingFAB(coarVoF,comp);
              }
            }
          }
        }
      }
    }
  }

  // Fifth pass to correct all the VolIndex's giving the children of each VoF.
  // To do this it is necessary to make a refined version of the current
  // (coarse) level to place the data from the next finer level (which may
  // have an incompatible DisjointBoxLayout).
  for (int ilev = 1; ilev < m_nlevels; ilev++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop9");

    // Get the new numberings for the children of the current (coarse) level
    LevelData<EBCellFAB>& newFineNumbering = *newNumberings[ilev-1];

    // Refine the coarse DisjointBoxLayout, create a temporary data holder
    // based on that, and copy the new numberings of the children into this
    // new data holder
    const DisjointBoxLayout& coarGrids = newNumberings[ilev]->disjointBoxLayout();

    DisjointBoxLayout fineGrids;
    refine(fineGrids,coarGrids,nRef);

    EBISLayout fineEBISLayout;
    m_ebisLevel[ilev-1]->fillEBISLayout(fineEBISLayout,fineGrids,nGhosts);

    EBCellFactory fineFactory(fineEBISLayout);

    LevelData<EBCellFAB> newRefinedCoarNumbering(fineGrids,nComps,ghostCells,fineFactory);

    newFineNumbering.copyTo(newRefinedCoarNumbering);

    // Go through all the connected components and fix the references to the
    // children in their graphs
    for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
    {
      EBIndexSpace& curEBIS = *(connectedEBIS[iEBIS]);
      EBISLevel* coarEBISLevel = curEBIS.m_ebisLevel[ilev];

      // Iterate through all the boxes on this level making the corrections
      for (DataIterator dit = coarEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
      {
        const Box& coarBox = coarEBISLevel->m_grids.get(dit());
        EBCellFAB& newNumberingFAB = newRefinedCoarNumbering[dit()];
        EBGraphImplem& coarEBGraph = *(coarEBISLevel->m_graph[dit()].m_implem);

        // If there are irregular cells, there may be something to correct
        if (coarEBGraph.m_tag == EBGraphImplem::HasIrregular)
        {
          // Go through all the IntVect's in the current box
          IntVectSet coarIVS(coarBox);
          for (IVSIterator ivsit(coarIVS); ivsit.ok(); ++ivsit)
          {
            const IntVect& coarIV = ivsit();

            GraphNode& coarGraphNode = coarEBGraph.m_graph(coarIV);

            // If there is a valid cellList then there will be references to
            // children VoF's that need to be corrected
            if (coarGraphNode.hasValidCellList())
            {
              Vector<GraphNodeImplem>* coarCellList = coarGraphNode.m_cellList;

              // Correct the children references in all the VoF's
              for (int iVoF = 0; iVoF < coarGraphNode.size(); iVoF++)
              {
                GraphNodeImplem& coarGraphNodeImplem = (*coarCellList)[iVoF];


                // Go through all the children making corrections
                int numChildren = coarGraphNodeImplem.m_finerNodes.size();
                for (int iChild = 0; iChild < numChildren; iChild++)
                {
                  VolIndex& curVolIndex = coarGraphNodeImplem.m_finerNodes[iChild];
                  const IntVect& fineIV = curVolIndex.gridIndex();

                  curVolIndex.define(fineIV,newNumberingFAB(curVolIndex,comp));
                }
              }
            }
          }
        }
      }
    }
  }

  // Sixth pass to copy all the relevant EBData into the new EBIndexSpace's.
  // Go from the coarsest level to the finest level.
  for (int ilev = m_nlevels-1; ilev >= 0; ilev--)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop10");

    // For each connected component, initialize the volume data and face data
    // holders.  This has to be done using a graph with one layer of ghost
    // cells which is created there temporarily.
    for (int iEBIS = minEBIS; iEBIS <= maxEBIS; iEBIS++)
    {
      EBIndexSpace& copyEBIS   = *(connectedEBIS[iEBIS]);
      EBISLevel* copyEBISLevel = copyEBIS.m_ebisLevel[ilev];

      EBGraphFactory graphFactory(copyEBISLevel->m_domain);
      LevelData<EBGraph> ghostGraph(copyEBISLevel->m_grids,
                                    1,
                                    IntVect::Unit,
                                    graphFactory);
      Interval interval(0,0);
      copyEBISLevel->m_graph.copyTo(interval,ghostGraph,interval);

      EBDataFactory dataFactory;
      copyEBISLevel->m_data.define(copyEBISLevel->m_grids,
                                   1,
                                   IntVect::Zero,
                                   dataFactory);

      for (DataIterator dit = copyEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
      {
        copyEBISLevel->m_data[dit()].defineVoFData(ghostGraph[dit()],
                                                   copyEBISLevel->m_grids.get(dit()));
        copyEBISLevel->m_data[dit()].defineFaceData(ghostGraph[dit()],
                                                    copyEBISLevel->m_grids.get(dit()));
      }
    }

    // Get the original EBISLevel to get access the original EBData
    EBISLevel* origEBISLevel = m_ebisLevel[ilev];

    // Get the component numbers and new VoF numbering so that the original
    // VolIndex's can be translated into the new VolIndex's for the correct
    // connected component.
    const LevelData<EBCellFAB>& numberedComponents = *numberedComponentses[ilev];
    const LevelData<EBCellFAB>& newNumbering       = *newNumberings       [ilev];

    // Iterate through all the boxes in this level copying the EBData to the
    // appropriate connected component
    for (DataIterator dit = origEBISLevel->m_grids.dataIterator(); dit.ok(); ++dit)
    {
      RefCountedPtr<EBDataImplem> origEBData = origEBISLevel->m_data[dit()].m_implem;

      const EBCellFAB& numberedComponentsFAB = numberedComponents[dit()];
      const EBCellFAB& newNumberingFAB       = newNumbering      [dit()];

      // Start with the volume data
      const BaseIVFAB<VolData>& origVolData = origEBData->m_volData;

      // Go through all the volume data a VoF at a time
      for (VoFIterator vofit(origVolData.getIVS(),origVolData.getEBGraph()); vofit.ok(); ++vofit)
      {
        // This is the original VolIndex
        VolIndex origVoF = vofit();

        int iEBIS = numberedComponentsFAB(origVoF,comp);

        // Only copy the volume data is the connected component number is
        // valid (if it is invalid then this connected component was discarded
        // because it had a total volume fracion of 0.0)
        if (iEBIS >= minEBIS && iEBIS <= maxEBIS)
        {
          // Construct the new VolIndex for the correct connected component
          int newCellIndex = newNumberingFAB(origVoF,comp);
          VolIndex newVoF(origVoF.gridIndex(),newCellIndex);

          EBIndexSpace&               copyEBIS      = *(connectedEBIS[iEBIS]);
          EBISLevel*                  copyEBISLevel = copyEBIS.m_ebisLevel[ilev];
          RefCountedPtr<EBDataImplem> copyEBData    = copyEBISLevel->m_data[dit()].m_implem;
          BaseIVFAB<VolData>&         copyVolData   = copyEBData->m_volData;

          // Copy the volume data to the correct place
          copyVolData(newVoF,comp) = origVolData(origVoF,comp);
        }
      }

      // Go through all the face data a direction and face at a time
      for (int idir = 0; idir < SpaceDim; idir++)
      {
        const BaseIFFAB<FaceData>& origFaceData = (origEBData->m_faceData)[idir];
        for (FaceIterator faceit(origFaceData.getIVS(),origFaceData.getEBGraph(),idir,FaceStop::SurroundingWithBoundary); faceit.ok(); ++faceit)
        {
          // This is the original FaceIndex and the VolIndex's that define it
          FaceIndex origFace = faceit();
          const VolIndex& origLoVoF = origFace.getVoF(Side::Lo);
          const VolIndex& origHiVoF = origFace.getVoF(Side::Hi);

          // Start out with invalid new values and fill them in from the lo or
          // hi VoF depending on which is valid
          int iEBIS = -1;
          int newLoCellIndex = -1;
          int newHiCellIndex = -1;

          // The lo VoF is valid
          if (origLoVoF.cellIndex() != -1)
          {
            if (iEBIS == -1)
            {
              iEBIS = numberedComponentsFAB(origLoVoF,comp);
            }
            newLoCellIndex = newNumberingFAB(origLoVoF,comp);
          }

          // The hi VoF is valid
          if (origHiVoF.cellIndex() != -1)
          {
            if (iEBIS == -1)
            {
              iEBIS = numberedComponentsFAB(origHiVoF,comp);
            }
            newHiCellIndex = newNumberingFAB(origHiVoF,comp);
          }

          // Only copy the face data is the connected component number is
          // valid (if it is invalid then this connected component was
          // discarded because it had a total volume fracion of 0.0)
          if (iEBIS >= minEBIS && iEBIS <= maxEBIS)
          {
            // Construct the new FaceIndex for the correct connected component
            // (which includes new VolIndex's)
            VolIndex newLoVoF(origLoVoF.gridIndex(),newLoCellIndex);
            VolIndex newHiVoF(origHiVoF.gridIndex(),newHiCellIndex);

            FaceIndex newFace(newLoVoF,newHiVoF,origFace.direction());

            EBIndexSpace&               copyEBIS      = *(connectedEBIS[iEBIS]);
            EBISLevel*                  copyEBISLevel = copyEBIS.m_ebisLevel[ilev];
            RefCountedPtr<EBDataImplem> copyEBData    = copyEBISLevel->m_data[dit()].m_implem;
            BaseIFFAB<FaceData>&        copyFaceData  = (copyEBData->m_faceData)[idir];

            // Copy the face data to the correct place
            copyFaceData(newFace,comp) = origFaceData(origFace,comp);
          }
        }
      }
    }
  }

  // Clean up the temporary data holders defined using the original
  // EBIndexSpace
  for (int ilev = 0; ilev < m_nlevels; ilev++)
  {
    CH_TIME("EBIndexSpace::connectedComponents-loop11");

    delete numberedComponentses[ilev];
    delete newNumberings[ilev];
  }

  if (anyRegularWithMultivaluedParent)
  {
    // TODO: Remove storage for any regular cells which had a multivalued
    // parent in the original graph but don't have a multivalued parent in
    // the connected componenet graphs.  The graphs are all correct without
    // this step but contain extra storage to explicitly store the cellIndex
    // of the parent and result in an extra level of indirection accessing
    // the parent.  On the other hand, these don't occur frequently.
  }

  // Return only the biggest connected component
  if (a_onlyBiggest)
  {
    RefCountedPtr<EBIndexSpace> biggestComponent(NULL);

    // If the EBIS is empty, then biggestEBIS = -1 and connectedEBIS.size() = 0.
    if (biggestEBIS >= 0)
      {
        biggestComponent = connectedEBIS[biggestEBIS];
      }

    connectedEBIS.resize(1);
    connectedEBIS[0] = biggestComponent;
  }

  // Return the Vector of RefCountedPtr<EBIndexSpace> - one for every
  // connected component
  return connectedEBIS;
}

// Divide the EBIndexSpace into connected components and return a Vector of
// disjoint EBIndexSpace's corresponding to each connected component.
Vector<RefCountedPtr<EBIndexSpace> > EBIndexSpace::connectedComponents()
{
  int numComponents;
  bool onlyBiggest = false;

  return findConnectedComponents(numComponents,onlyBiggest);
}

// Divide the EBIndexSpace into connected components and return the
// EBIndexSpace corresponding to the largest connected component.
RefCountedPtr<EBIndexSpace> EBIndexSpace::biggestConnectedComponent(int & a_numComponents)
{
  CH_TIME("EBIndexSpace::biggestConnectedComponent");

  bool onlyBiggest = true;

  Vector<RefCountedPtr<EBIndexSpace> > biggest = findConnectedComponents(a_numComponents,onlyBiggest);

  if (biggest.size() != 1)
  {
    MayDay::Error("EBIndexSpace::connectedComponents didn't return one EBIndexSpace when asked for only the biggest component");
  }

  return biggest[0];
}

// Recursively find all VoFs connected to "a_curVoF" and number them
// "a_curNum".  This works on a single EBCellFAB.  "a_lastVoF" is used to
// avoid recursing back to the VoF that generated that call to this routine.
// For the first call, "a_lastVoF" should equal "a_curVoF" since this allows
// recursion using all faces.
bool EBIndexSpace::setAllConnectedVoFs(Real&               a_totalVolFrac,
                                       EBCellFAB&          a_curEBCellFAB,
                                       const EBGraph&      a_curEBGraph,
                                       const EBISBox&      a_curEBISBox,
                                       const VolIndex&     a_curVoF,
                                       const VolIndex&     a_lastVoF,
                                       const unsigned int& a_curNum)
{
  bool foundNewVoF = false;

  a_totalVolFrac = 0.0;

  int comp = 0;

  if (a_curEBCellFAB(a_curVoF,comp) == -1.0)
  {
    foundNewVoF = true;

    a_curEBCellFAB(a_curVoF,comp) = a_curNum;
    a_totalVolFrac += a_curEBISBox.volFrac(a_curVoF);

    for (int idir = 0; idir < SpaceDim; idir++)
    {
      for (SideIterator sit; sit.ok(); ++sit)
      {
        const Side::LoHiSide& curSide = sit();

        const Vector<FaceIndex> faces = a_curEBISBox.getFaces(a_curVoF,idir,curSide);

        for (int iface = 0; iface < faces.size(); iface++)
        {
          const FaceIndex& face = faces[iface];

          const VolIndex& nextVoF = face.getVoF(curSide);


          if (nextVoF.cellIndex() >= 0 && nextVoF != a_lastVoF)
          {
            Real totalVolFrac;

            setAllConnectedVoFs(totalVolFrac,
                                a_curEBCellFAB,
                                a_curEBGraph,
                                a_curEBISBox,
                                nextVoF,
                                a_curVoF,
                                a_curNum);

            a_totalVolFrac += totalVolFrac;
          }
        }
      }
    }
  }

  return foundNewVoF;
}

// Undo everything done by "setAllConnectedVoFs" - this is needed if the total
// volume fraction found by "setAllConnectedVoFs" is 0.0 because then this
// connected component will be ignored.
void EBIndexSpace::resetAllConnectedVoFs(EBCellFAB&          a_curEBCellFAB,
                                         const EBGraph&      a_curEBGraph,
                                         const EBISBox&      a_curEBISBox,
                                         const VolIndex&     a_curVoF,
                                         const VolIndex&     a_lastVoF)
{
  int comp = 0;

  a_curEBCellFAB(a_curVoF,comp) = -1.0;

  for (int idir = 0; idir < SpaceDim; idir++)
  {
    for (SideIterator sit; sit.ok(); ++sit)
    {
      const Side::LoHiSide& curSide = sit();

      const Vector<FaceIndex> faces = a_curEBISBox.getFaces(a_curVoF,idir,curSide);

      for (int iface = 0; iface < faces.size(); iface++)
      {
        const FaceIndex& face = faces[iface];

        const VolIndex& nextVoF = face.getVoF(curSide);

        if (nextVoF.cellIndex() >= 0 && nextVoF != a_lastVoF)
        {
          resetAllConnectedVoFs(a_curEBCellFAB,
                                a_curEBGraph,
                                a_curEBISBox,
                                nextVoF,
                                a_curVoF);
        }
      }
    }
  }
}

/****************/
EBIndexSpace* Chombo_EBIS::s_instance = NULL;
bool          Chombo_EBIS::s_aliased  = false;

/****************/
EBIndexSpace* Chombo_EBIS::instance()
{
  if ((!s_aliased) && (s_instance == NULL))
  {
    s_instance = new EBIndexSpace();
  }

  return  s_instance;
}
/****************/
void Chombo_EBIS::alias(const EBIndexSpace* a_input)
{
  s_instance = (EBIndexSpace*)(a_input);
  s_aliased  = true;
}

#include "NamespaceFooter.H"
