#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

//  ANAG, LBNL, DTG

#ifndef _EBINDEXSPACE_H_
#define _EBINDEXSPACE_H_

#include "REAL.H"
#include "IntVect.H"
#include "IntVectSet.H"
#include "VolIndex.H"
#include "FaceIndex.H"
#include "IntVectSet.H"
#include "DisjointBoxLayout.H"
#include "LevelData.H"
#include "EBISBox.H"
#include "GeometryService.H"
#include "EBISLayout.H"
#include "CH_HDF5.H"
#include <map>

#include "NamespaceHeader.H"

typedef std::map<DisjointBoxLayout, EBISLayout> dmap;

class EBIndexSpace;

///singleton version of EBIndexSpace.
class Chombo_EBIS
{
private:
  ///
  /**
   This is NULL until the first time
     instance() is called.
     On the first time it is called it
     is set to  new EBIndexSpace();
     After that it points to the same
     thing.
   */
  static EBIndexSpace* s_instance;

  Chombo_EBIS()
  {;}

  ~Chombo_EBIS()
  {;}

  ///if you want to sneak an existing EBIS into the singleton.
  static bool s_aliased;
public:
  ///
  /**
     Return a pointer to the singleton
     EBIndexSpace.  Until this is called
     once, the singleton is null.  See
     Design patterns for why this is done
     this way.
   */
  static EBIndexSpace* instance();

  ///
  static void alias(const EBIndexSpace* a_input);
};

///
/**
   EBISLevel represents the geometric information
   of the domain at a particular level of refinement.
*/
class EBISLevel
{
public:
  void dumpDebug(const string& a_string);

  void coarsenVoFs(EBISLevel& a_fineEBIS);

  void fixFineToCoarse(EBISLevel& a_fineEBIS);

  void coarsenFaces(EBISLevel& a_fineEBIS);

  int numVoFsOnProc() const;

  void levelStitch(EBISLevel&       a_otherPhase,
                   const EBISLevel* a_fineThisPtr,
                   const EBISLevel* a_fineOtherPtr); // MF Addition  bvs

  void reconcileIrreg(EBISLevel & a_otherPhase);

  void cellStitch(EBData&        a_ebdataCoarA,
                  EBData&        a_ebdataCoarB,
                  const EBGraph& a_ebgrapCoarA,
                  const EBGraph& a_ebgrapCoarB,
                  const EBISBox& a_ebisbxFineA,
                  const EBISBox& a_ebisbxFineB,
                  const IntVect& a_iv,
                  const int&     a_aphase,
                  const int&     a_bphase);
  EBISLevel();

  ///
  /**
     Defines the level.  Calls the geoserver
     to fill the layout with geometric information.
   */
  EBISLevel(const ProblemDomain   & a_domain,
            const RealVect        & a_origin,
            const Real            & a_dx,
            const GeometryService & a_geoserver,
            const EBIndexSpace    * const a_ebisPtr               = Chombo_EBIS::instance(),
            const bool            & a_distributedData             = false,
            const bool            & a_fixRegularNextToMultiValued = true);

  ///
  /**
     Defines the level from a finer level
     (factor of two refinement ratio).
     Fills in all the coarse-fine information
     in both levels.
   */
  EBISLevel(EBISLevel             & a_finerLevel,
            const GeometryService & a_geoserver,
            const EBIndexSpace    * const a_ebisPtr               = Chombo_EBIS::instance(),
            const bool            & a_distributedData             = false,
            const bool            & a_fixRegularNextToMultiValued = true);

#ifdef CH_USE_HDF5
  /// read from file
  EBISLevel(HDF5Handle& handle);

  ///
  void write(HDF5Handle& a_handle) const;
#endif

  ///
  /**
     Checks to see the vofs are in the correct cells.
     Checks to see that the faces are over the correct cells.
     Checks that volume fractions, area fractions are positive.
     Bail out with MayDay::Error if anything fails.
  */
  void sanityCheck(const EBIndexSpace* const a_ebisPtr = Chombo_EBIS::instance());

  ///
  void fillEBISLayout(EBISLayout&              a_ebis,
                      const DisjointBoxLayout& a_grids,
                      const int&               a_nghost) const;

  ~EBISLevel();

  const ProblemDomain& getDomain() const;

  const DisjointBoxLayout& getGrids() const;

  DisjointBoxLayout getIrregGrids(const ProblemDomain& a_domain) const;

  DisjointBoxLayout getFlowGrids(const ProblemDomain& a_domain) const;

  DisjointBoxLayout getCoveredGrids(const ProblemDomain& a_domain) const;

  IntVectSet irregCells() const;

  const Real& getDX() const;

  const RealVect& getOrigin() const;

  ///
  void fixRegularNextToMultiValued();

  void clearMultiBoundaries();
  void setBoundaryPhase(int phase);

  int m_phase;


private:

  ///
  DisjointBoxLayout m_grids;

  ///
  ProblemDomain m_domain;

  ///
  RealVect m_origin;

  ///
  Real m_dx;

  ///
  LevelData<EBGraph> m_graph;

  ///
  LevelData<EBData>  m_data;

  Real m_tolerance; // used in Multifluid face matching algorithm.

  mutable std::map<int, dmap> m_cache;

  mutable int m_cacheMisses, m_cacheHits, m_cacheStale;

  static Real s_tolerance;
  static bool s_verbose;

  //make grids for this level.

  static void makeBoxes(Vector<Box>&              a_boxes,
                        Vector<long>&             a_irregCount,
                        const Box&                a_region,
                        const ProblemDomain&      a_domain,
                        const GeometryService&    a_geoserver,
                        const RealVect&           a_origin,
                        const Real&               a_dx,
                        const int                 a_ncellmax,
                        const EBIndexSpace* const a_ebisPtr);

  static void makeBoxes(std::list<Box>&        a_boxes,
                        const Box&             a_region,
                        const ProblemDomain&   a_domain,
                        const GeometryService& a_geoserver,
                        const RealVect&        a_origin,
                        const Real&            a_dx,
                        const int              a_ncellmax);

  //these are disallowed
  void operator=(const EBISLevel& ebiin)
  {
    MayDay::Error("EBISLevel::operator= not allowed");
  }

  EBISLevel(const EBISLevel& ebiin)
  {
    MayDay::Error("EBISLevel::copy constructor not allowed");
  }

  bool match(const Real& a_a,
             const Real& a_b) const;

  bool match(const RealVect& a_a,
             const RealVect& a_b) const;

  void refreshCache() const;

  friend class EBIndexSpace;
};

///
/**
   EBIndexSpace represents the geometric information
   of the domain.    It should be generated through
   the Chombo_EBIS class.  This follows the singleton
   pattern.
*/
class EBIndexSpace
{
public:

  EBIndexSpace();

  ///
  /**
      If a_ncellMax is set, that is the max width of
      an internal grid.  Otherwise use defaults
      of (16 in 3D, 64 in 2d)
   */
  void
  define(const ProblemDomain   & a_domain,
         const RealVect        & a_origin,
         const Real            & a_dx,
         const GeometryService & a_geoserver,
         int                     a_nCellMax = -1,
         int                     a_maxCoarsenings = -1);

#ifdef CH_USE_HDF5
  ///
  void
  define(HDF5Handle & a_handle,
         int          a_maxCoarsenings = -1);
#endif

  void
  define(EBISLevel * a_level0,
         int         a_nCellMax = -1,
         int         a_maxCoarsenings = -1);

  void
  define(const ProblemDomain                        & a_entireDomain,
         const RealVect                             & a_origin,
         const Real                                 & a_dx,
         const Vector<RefCountedPtr<EBIndexSpace> > & a_patches,
         const Vector<IntVect>                      & a_offsets,
         int                                          a_maxCoarsenings = -1);

  EBISLevel* buildFirstLevel(const ProblemDomain& a_domain,
                             const RealVect& a_origin,
                             const Real& a_dx,
                             const GeometryService& a_geoserver,
                             int a_nCellMax,
                             int a_maxCoarsenings,
                             bool a_fixRegularNextToMultiValued = true);

  EBISLevel* buildNextLevel(const GeometryService & a_geoserver,
                            bool                    a_fixRegularNextToMultiValued = true);

  ///
  ~EBIndexSpace();

  ///
  /**
    Set a flag that indicates the data defining the EB is distributed.
    This will signal EBISLevel to hand grid generation off to the
    GeometryService.  Function returns previous value of m_distributedData.
    Call this before define() if this functionality is desired.
  */
  bool setDistributedData()
  {
    bool was = m_distributedData;
    m_distributedData = true;
    return was;
  }

  ///
  int numLevels() const;

  ///
  /**
     Get the number of vofs over the entire domain.
     This is blocking as a broadcast and gather are required.
   */
  int numVoFs(const ProblemDomain& a_domain) const;

  ///
  /**
     return level index of domain.
     return -1 if a_domain does not
     correspond to any refinement of EBIS.
   */
  int getLevel(const ProblemDomain& a_domain) const;

  RealVect getOrigin() const
  {
    return m_ebisLevel[0]->getOrigin();
  }

  ///
  /**
     returns the problem domain box at a level of refinement
  */
  const ProblemDomain& getBox(int level) const
  {
    return m_domainLevel[level];
  }

  ///
  void fillEBISLayout(EBISLayout&              a_ebis,
                      const DisjointBoxLayout& a_grids,
                      const ProblemDomain&     a_domain,
                      const int&               a_nghost) const;

  ///
  /**
     Return true if the define function has been called.
   */
  bool isDefined() const;

  ///
  void clear();

  int getNCellMax() const;
  ///
  /**
     This makes Chombo_EBIS be the only function
     that can call the private constructor.
   */
  friend class Chombo_EBIS;

#ifdef CH_USE_HDF5
  ///
  /**
     Writes out finest level as a default.   Writes at the level you are domain
     you want if a_outputLevel is defined.
   */
  void
  write(HDF5Handle&   a_handle,
        ProblemDomain a_outputLevel = ProblemDomain()) const;

  void
  writeInfo(HDF5Handle& handle) const ;

#endif

  DisjointBoxLayout getIrregGrids(int level) const
  {
    return m_ebisLevel[level]->getIrregGrids(m_domainLevel[level]);
  }

  DisjointBoxLayout getIrregGrids(const ProblemDomain& a_domain) const
  {
    return m_ebisLevel[getLevel(a_domain)]->getIrregGrids(a_domain);
  }

  DisjointBoxLayout getFlowGrids(int level) const
  {
    return m_ebisLevel[level]->getFlowGrids(m_domainLevel[level]);
  }

  DisjointBoxLayout getFlowGrids(const ProblemDomain& a_domain) const
  {
    return m_ebisLevel[getLevel(a_domain)]->getFlowGrids(a_domain);
  }

  DisjointBoxLayout getCoveredGrids(int level) const
  {
    return m_ebisLevel[level]->getCoveredGrids(m_domainLevel[level]);
  }

  DisjointBoxLayout getCoveredGrids(const ProblemDomain& a_domain) const
  {
    return m_ebisLevel[getLevel(a_domain)]->getCoveredGrids(a_domain);
  }

  DisjointBoxLayout levelGrids(int level) const
  {
    return m_ebisLevel[level]->m_grids;
  }

  Real dx(int level) const
  {
    return m_ebisLevel[level]->m_dx;
  }

  IntVectSet irregCells(int depth=0) const
  {
    return m_ebisLevel[depth]->irregCells();
  }

  void setCellMax(int max)
  {
    m_nCellMax = max;
  }

  void resetLevels(int nLevel);

  Vector<RefCountedPtr<EBIndexSpace> > connectedComponents();

  RefCountedPtr<EBIndexSpace> biggestConnectedComponent(int & a_numComponents);

  static bool s_MFSingleBox;

private:
  Vector<RefCountedPtr<EBIndexSpace> > findConnectedComponents(int        & a_numComponents,
                                                               const bool & a_onlyBiggest);

  bool setAllConnectedVoFs(Real&               a_totalVolFrac,
                           EBCellFAB&          a_curEBCellFAB,
                           const EBGraph&      a_curEBGraph,
                           const EBISBox&      a_curEBISBox,
                           const VolIndex&     a_curVoF,
                           const VolIndex&     a_lastVoF,
                           const unsigned int& a_curNum);

  void resetAllConnectedVoFs(EBCellFAB&          a_curEBCellFAB,
                             const EBGraph&      a_curEBGraph,
                             const EBISBox&      a_curEBISBox,
                             const VolIndex&     a_curVoF,
                             const VolIndex&     a_lastVoF);

  //this is private to force the singleton thing.
  //EBIndexSpace();

  int  m_nCellMax;
  bool m_isDefined;

  bool m_distributedData;

  Vector<EBISLevel*> m_ebisLevel;
  Vector<ProblemDomain> m_domainLevel;

  int m_nlevels;

  static Real s_tolerance;
  static bool s_verbose;

  //disallowed for performance reasons
  void operator=(const EBIndexSpace& ebiin)
  {
    MayDay::Error("no assignment of EBIndexSpace allowed");
  }

  EBIndexSpace(const EBIndexSpace& ebiin)
  {
    MayDay::Error("no copy construction of EBIndexSpace allowed");
  }
};

inline bool EBISLevel::match(const Real& a_a,
                             const Real& a_b) const
{
  return (a_a < (a_b + m_tolerance)) && (a_a > (a_b - m_tolerance));
}

inline bool EBISLevel::match(const RealVect& a_a,
                             const RealVect& a_b) const
{
  return D_TERM6( (a_a[0] < (a_b[0] + m_tolerance)) &&
                  (a_a[0] > (a_b[0] - m_tolerance)),
                  &&
                  (a_a[1] < (a_b[1] + m_tolerance)) &&
                  (a_a[1] > (a_b[1] - m_tolerance)),
                  &&
                  (a_a[2] < (a_b[2] + m_tolerance)) &&
                  (a_a[2] > (a_b[2] - m_tolerance)),
                  &&
                  (a_a[3] < (a_b[3] + m_tolerance)) &&
                  (a_a[3] > (a_b[3] - m_tolerance)),
                  &&
                  (a_a[4] < (a_b[4] + m_tolerance)) &&
                  (a_a[4] > (a_b[4] - m_tolerance)),
                  &&
                  (a_a[5] < (a_b[5] + m_tolerance)) &&
                  (a_a[5] > (a_b[5] - m_tolerance)));
}

#include "NamespaceFooter.H"

#endif
