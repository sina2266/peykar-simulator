#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#include <cstdio>

#include "AMRIO.H"
#include "SPMD.H"
#include "PhysIBC.H"
#include "LoHiSide.H"
#include "CH_Timer.H"

#include "LevelGodunov.H"

#include "NamespaceHeader.H"

// Constructor - set up some defaults
LevelGodunov::LevelGodunov()
{
  m_dx           = 0.0;
  m_refineCoarse = 0;
  m_isDefined    = false;
}

// Destructor - free up storage
LevelGodunov::~LevelGodunov()
{

}

// Define the object so that time stepping can begin
void LevelGodunov::define(const DisjointBoxLayout&    a_thisDisjointBoxLayout,
                          const DisjointBoxLayout&    a_coarserDisjointBoxLayout,
                          const ProblemDomain&        a_domain,
                          const int&                  a_refineCoarse,
                          const Real&                 a_dx,
                          const GodunovPhysics* const a_gdnvPhysics,
                          const int&                  a_normalPredOrder,
                          const bool&                 a_useFourthOrderSlopes,
                          const bool&                 a_usePrimLimiting,
                          const bool&                 a_useCharLimiting,
                          const bool&                 a_useFlattening,
                          const bool&                 a_useArtificialViscosity,
                          const Real&                 a_artificialViscosity,
                          const bool&                 a_hasCoarser,
                          const bool&                 a_hasFiner)
{
  CH_TIME("LevelGodunov::define");

  // Sanity checks
  CH_assert(a_refineCoarse > 0);
  CH_assert(a_dx > 0.0);

  // Make a copy of the current grids
  m_grids  = a_thisDisjointBoxLayout;

  // Order of the normal predictor (1 -> PLM, 2-> PPM)
  m_normalPredOrder = a_normalPredOrder;

  // Store the various slope computation flags
  m_useFourthOrderSlopes = a_useFourthOrderSlopes;
  m_usePrimLimiting      = a_usePrimLimiting;
  m_useCharLimiting      = a_useCharLimiting;
  m_useFlattening        = a_useFlattening;

  // Store the artificial viscosity flag and coefficient
  m_useArtificialViscosity = a_useArtificialViscosity;
  m_artificialViscosity = a_artificialViscosity;

  // Cache data
  m_dx = a_dx;
  m_domain = a_domain;
  m_refineCoarse = a_refineCoarse;
  m_hasCoarser = a_hasCoarser;
  m_hasFiner = a_hasFiner;

  m_patchGodunov.define(m_domain,m_dx,
                        a_gdnvPhysics,
                        m_normalPredOrder,
                        m_useFourthOrderSlopes,
                        m_usePrimLimiting,
                        m_useCharLimiting,
                        m_useFlattening,
                        m_useArtificialViscosity,
                        m_artificialViscosity);

  // Set the number of ghost cells appropriately
  if (m_useFourthOrderSlopes || m_normalPredOrder == 2)
    {
      m_numGhost = 4;
    }
  else
    {
      m_numGhost = 2;
    }

  // (DFM, 9/29/2005) This is really silly, but the simplest
  // work-around. The problem is that the numConserved and
  // numFluxes functions are not const, but a_gdnvPhysics is.
  // So. the simplest thing is to cast away the const-ness
  // just for this limited context.
  // We may eventually want to make the functions non-const,
  // but that will break all classes derived from GodunovPhysics.
  {
    GodunovPhysics* nonConstPhysicsPtr = (GodunovPhysics*) a_gdnvPhysics;
    m_numCons   = nonConstPhysicsPtr->numConserved();
    m_numFluxes = nonConstPhysicsPtr->numFluxes();
  }

  m_exchangeCopier.exchangeDefine(a_thisDisjointBoxLayout,
                                  m_numGhost*IntVect::Unit);

  // Setup an interval corresponding to the conserved variables
  Interval UInterval(0,m_numCons-1);

  // Create temporary storage with a layer of "m_numGhost" ghost cells
  {
    CH_TIME("setup::Udefine");
    m_U.define(m_grids,m_numCons,m_numGhost*IntVect::Unit);
  }

  // Set up the interpolator if there is a coarser level
  if (m_hasCoarser)
    {
      m_patcher.define(a_thisDisjointBoxLayout,
                       a_coarserDisjointBoxLayout,
                       m_numCons,
                       coarsen(a_domain,a_refineCoarse),
                       a_refineCoarse,
                       m_numGhost);
    }

  // Everything is defined
  m_isDefined = true;
}

// Advance the solution by "a_dt" by using an unsplit method.
// "a_finerFluxRegister" is the flux register with the next finer level.
// "a_coarseFluxRegister" is flux register with the next coarser level.
// If source terms do not exist, "a_S" should be null constructed and not
// defined (i.e. its define() should not be called).
Real LevelGodunov::step(LevelData<FArrayBox>&       a_U,
                        LevelData<FArrayBox>        a_flux[CH_SPACEDIM],
                        LevelFluxRegister&          a_finerFluxRegister,
                        LevelFluxRegister&          a_coarserFluxRegister,
                        const LevelData<FArrayBox>& a_S,
                        const LevelData<FArrayBox>& a_UCoarseOld,
                        const Real&                 a_TCoarseOld,
                        const LevelData<FArrayBox>& a_UCoarseNew,
                        const Real&                 a_TCoarseNew,
                        const Real&                 a_time,
                        const Real&                 a_dt)
{
  CH_TIMERS("LevelGodunov::step");

  CH_TIMER("LevelGodunov::step::setup"   ,timeSetup);
  CH_TIMER("LevelGodunov::step::update"  ,timeUpdate);
  CH_TIMER("LevelGodunov::step::reflux"  ,timeReflux);
  CH_TIMER("LevelGodunov::step::conclude",timeConclude);

  // Make sure everything is defined
  CH_assert(m_isDefined);

  CH_START(timeSetup);

  // Clear flux registers with next finer level
  if (m_hasFiner)
    {
      a_finerFluxRegister.setToZero();
    }

  // Setup an interval corresponding to the conserved variables
  Interval UInterval(0,m_numCons-1);

  {
    CH_TIME("setup::localU");
    for (DataIterator dit = m_U.dataIterator(); dit.ok(); ++dit)
      {
        m_U[dit].setVal(0.0); // Gets rid of denormalized crap.
        m_U[dit].copy(a_U[dit]);
      }

    m_U.exchange(m_exchangeCopier);
  }

  // Fill m_U's ghost cells using fillInterp
  if (m_hasCoarser)
    {
      // Fraction "a_time" falls between the old and the new coarse times
      Real alpha = (a_time - a_TCoarseOld) / (a_TCoarseNew - a_TCoarseOld);

      // Truncate the fraction to the range [0,1] to remove floating-point
      // subtraction roundoff effects
      Real eps = 0.04 * a_dt / m_refineCoarse;

      if (Abs(alpha) < eps)
        {
          alpha = 0.0;
        }

      if (Abs(1.0-alpha) < eps)
        {
          alpha = 1.0;
        }

      // Current time before old coarse time
      if (alpha < 0.0)
        {
          MayDay::Error( "LevelGodunov::step: alpha < 0.0");
        }

      // Current time after new coarse time
      if (alpha > 1.0)
        {
          MayDay::Error( "LevelGodunov::step: alpha > 1.0");
        }

      // Interpolate ghost cells from next coarser level using both space
      // and time interpolation
      m_patcher.fillInterp(m_U,
                           a_UCoarseOld,
                           a_UCoarseNew,
                           alpha,
                           0,0,m_numCons);
    }

  // Potentially used in boundary conditions
  m_patchGodunov.setCurrentTime(a_time);

  // Dummy source used if source term passed in is empty
  FArrayBox zeroSource;

  // Use to restrict maximum wave speed away from zero
  Real maxWaveSpeed = 1.0e-12;

  CH_STOP(timeSetup);

  // Beginning of loop through patches/grids.
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      CH_START(timeUpdate);

      // The current box
      Box curBox = m_grids.get(dit());

      // The current grid of conserved variables
      FArrayBox& curU = m_U[dit];

      // The current source terms if they exist
      const FArrayBox* source = &zeroSource;
      if (a_S.isDefined())
        {
          source = &a_S[dit];
        }

      // The fluxes computed for this grid - used for refluxing and returning
      // other face centered quantities
      FluxBox flux;

      // Set the current box for the patch integrator
      m_patchGodunov.setCurrentBox(curBox);

      Real maxWaveSpeedGrid;

      // Update the current grid's conserved variables, return the final
      // fluxes used for this, and the maximum wave speed for this grid
      m_patchGodunov.updateState(curU,
                                 flux,
                                 maxWaveSpeedGrid,
                                 *source,
                                 a_dt,
                                 curBox);

      // Clamp away from zero
      maxWaveSpeed = Max(maxWaveSpeed,maxWaveSpeedGrid);

      CH_STOP(timeUpdate);

      CH_START(timeReflux);

      // Do flux register updates
      for (int idir = 0; idir < SpaceDim; idir++)
        {
          // Increment coarse flux register between this level and the next
          // finer level - this level is the next coarser level with respect
          // to the next finer level
          if (m_hasFiner)
            {
              a_finerFluxRegister.incrementCoarse(flux[idir],a_dt,dit(),
                                                  UInterval,
                                                  UInterval,idir);
            }

          // Increment fine flux registers between this level and the next
          // coarser level - this level is the next finer level with respect
          // to the next coarser level
          if (m_hasCoarser)
            {
              a_coarserFluxRegister.incrementFine(flux[idir],a_dt,dit(),
                                                  UInterval,
                                                  UInterval,idir);
            }
        }

      CH_STOP(timeReflux);
    }

  CH_START(timeConclude);

  {
    CH_TIME("conclude::copyU");
    // Now that we have completed the updates of all the patches, we copy the
    // contents of temporary storage, m_U, into the permanent storage, a_U.
    for (DataIterator dit = m_U.dataIterator(); dit.ok(); ++dit)
      {
        a_U[dit].copy(m_U[dit]);
      }
  }

  // Find the minimum of dt's over this level
  Real local_dtNew = m_dx / maxWaveSpeed;
  Real dtNew;

  {
    CH_TIME("conclude::getDt");
#ifdef CH_MPI
    int result = MPI_Allreduce(&local_dtNew, &dtNew, 1, MPI_CH_REAL,
                               MPI_MIN, Chombo_MPI::comm);
    if (result != MPI_SUCCESS)
      {
        MayDay::Error("LevelGodunov::step: MPI communcation error");
      }
#else
    dtNew = local_dtNew;
#endif
  }

  CH_STOP(timeConclude);

  // Return the maximum stable time step
  return dtNew;
}

// To be added...
void LevelGodunov::computeWHalf(LayoutData<FluxBox>&        a_WHalf,
                                LevelData<FArrayBox>&       a_U,
                                const LevelData<FArrayBox>& a_S,
                                const LevelData<FArrayBox>& a_UCoarseOld,
                                const Real&                 a_TCoarseOld,
                                const LevelData<FArrayBox>& a_UCoarseNew,
                                const Real&                 a_TCoarseNew,
                                const Real&                 a_time,
                                const Real&                 a_dt)
{
  CH_TIME("LevelGodunov::computeWHalf");

  // Make sure everything is defined
  CH_assert(m_isDefined);

  // Setup an interval corresponding to the conserved variables
  Interval UInterval(0,m_numCons-1);

  // Create temporary storage with a layer of "m_numGhost" ghost cells
  LevelData<FArrayBox> U(m_grids,m_numCons,m_numGhost*IntVect::Unit);

  for (DataIterator dit = U.dataIterator(); dit.ok(); ++dit)
    {
      U[dit].setVal(0.0);
    }

  // Copy the current conserved variables into the temporary storage
  a_U.copyTo(UInterval,U,UInterval);

  // Fill U's ghost cells using fillInterp
  if (m_hasCoarser)
    {
      // Fraction "a_time" falls between the old and the new coarse times
      Real alpha = (a_time - a_TCoarseOld) / (a_TCoarseNew - a_TCoarseOld);

      // Truncate the fraction to the range [0,1] to remove floating-point
      // subtraction roundoff effects
      Real eps = 0.04 * a_dt / m_refineCoarse;

      if (Abs(alpha) < eps)
        {
          alpha = 0.0;
        }

      if (Abs(1.0-alpha) < eps)
        {
          alpha = 1.0;
        }

      // Current time before old coarse time
      if (alpha < 0.0)
        {
          MayDay::Error( "LevelGodunov::step: alpha < 0.0");
        }

      // Current time after new coarse time
      if (alpha > 1.0)
        {
          MayDay::Error( "LevelGodunov::step: alpha > 1.0");
        }

      // Interpolate ghost cells from next coarser level using both space
      // and time interpolation
      m_patcher.fillInterp(U,
                           a_UCoarseOld,
                           a_UCoarseNew,
                           alpha,
                           0,0,m_numCons);
    }

  // Exchange all the data between grids at this level
  // I don't think this is necessary
  //U.exchange(UInterval);

  // Now we copy the contents of temporary storage, U, into the permanent
  // storage, a_U, to get ghost cells set for call "computeUpdate".
  for (DataIterator dit = U.dataIterator(); dit.ok(); ++dit)
    {
      a_U[dit].copy(U[dit]);
    }

  // Potentially used in boundary conditions
  m_patchGodunov.setCurrentTime(a_time);

  // Dummy source used if source term passed in is empty
  FArrayBox zeroSource;
  // Beginning of loop through patches/grids.
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      // The current box
      Box curBox = m_grids.get(dit());

      // The current grid of conserved variables
      const FArrayBox& curU = U[dit];

      // The current grid of primitive variables extrapolated to faces and
      // half a time step
      FluxBox& curWHalf = a_WHalf[dit];

      // The current source terms if they exist
      const FArrayBox* source = &zeroSource;
      if (a_S.isDefined())
        {
          source = &a_S[dit];
        }

      // Set the current box for the patch integrator
      m_patchGodunov.setCurrentBox(curBox);

      // Update the current grid's conserved variables, return the final
      // fluxes used for this, and the maximum wave speed for this grid
      m_patchGodunov.computeWHalf(curWHalf,
                                  curU,
                                  *source,
                                  a_dt,
                                  curBox);
    }
}


Real LevelGodunov::computeUpdate(LevelData<FArrayBox>&       a_dU,
                                 LevelFluxRegister&          a_finerFluxRegister,
                                 LevelFluxRegister&          a_coarserFluxRegister,
                                 const LevelData<FArrayBox>& a_U,
                                 const LayoutData<FluxBox>&  a_WHalf,
                                 const Real&                 a_time,
                                 const Real&                 a_dt)
{
  CH_TIME("LevelGodunov::computeUpdate");

  // Make sure everything is defined
  CH_assert(m_isDefined);

  // Clear flux registers with next finer level
  if (m_hasFiner)
    {
      a_finerFluxRegister.setToZero();
    }

  // Setup an interval corresponding to the conserved variables
  Interval UInterval(0,m_numCons-1);

  // Potentially used in boundary conditions
  m_patchGodunov.setCurrentTime(a_time);

  // Use to restrict maximum wave speed away from zero
  Real maxWaveSpeed = 1.0e-12;

  // Beginning of loop through patches/grids.
  for (DataIterator dit = m_grids.dataIterator(); dit.ok(); ++dit)
    {
      // The current box
      Box curBox = m_grids.get(dit());

      // The current grid of conserved variables changes
      FArrayBox& curDU = a_dU[dit];

      // The fluxes computed for this grid - used for refluxing and returning
      // other face centered quantities
      FluxBox flux;
      flux.resize(curBox,m_numFluxes);
      flux.setVal(0.0);

      // The current grid of conserved variables
      const FArrayBox& curU = a_U[dit];

      // The current grid of primitive variables extrapolated to faces and a
      // half time step
      const FluxBox& curWHalf = a_WHalf[dit];

      // Set the current box for the patch integrator
      m_patchGodunov.setCurrentBox(curBox);

      Real maxWaveSpeedGrid;

      // Update the current grid's conserved variables, return the final
      // fluxes used for this, and the maximum wave speed for this grid
      m_patchGodunov.computeUpdate(curDU,
                                   flux,
                                   curU,
                                   curWHalf,
                                   a_dt,
                                   curBox);

      // Get maximum wave speed for this grid
      maxWaveSpeedGrid = m_patchGodunov.getGodunovPhysicsPtr()->getMaxWaveSpeed(curU, curBox);

      // Clamp away from zero
      maxWaveSpeed = Max(maxWaveSpeed,maxWaveSpeedGrid);

      // Do flux register updates
      for (int idir = 0; idir < SpaceDim; idir++)
        {
          // Increment coarse flux register between this level and the next
          // finer level - this level is the next coarser level with respect
          // to the next finer level
          if (m_hasFiner)
            {
              a_finerFluxRegister.incrementCoarse(flux[idir],a_dt,dit(),
                                                  UInterval,
                                                  UInterval,idir);
            }

          // Increment fine flux registers between this level and the next
          // coarser level - this level is the next finer level with respect
          // to the next coarser level
          if (m_hasCoarser)
            {
              a_coarserFluxRegister.incrementFine(flux[idir],a_dt,dit(),
                                                  UInterval,
                                                  UInterval,idir);
            }
        }
    }

  // Find the minimum of dt's over this level
  Real local_dtNew = m_dx / maxWaveSpeed;
  Real dtNew;

#ifdef CH_MPI
  int result = MPI_Allreduce(&local_dtNew, &dtNew, 1, MPI_CH_REAL,
                                 MPI_MIN, Chombo_MPI::comm);
  if (result != MPI_SUCCESS)
  {
        MayDay::Error("LevelGodunov::step: MPI communcation error");
  }
#else
  dtNew = local_dtNew;
#endif

  // Return the maximum stable time step
  return dtNew;
}

// Find the maximum wave speed on the current level
Real LevelGodunov::getMaxWaveSpeed(const LevelData<FArrayBox>& a_U)
{
  CH_TIME("LevelGodunov::getMaxWaveSpeed");

  const DisjointBoxLayout& disjointBoxLayout = a_U.disjointBoxLayout();
  DataIterator dit = disjointBoxLayout.dataIterator();

  // Initial maximum wave speed
  Real speed = 0.0;

  // This computation doesn't need involve a time but the time being set
  // is checked by PatchGodunov::getMaxWaveSpeed so we have to set it
  // to something...
  m_patchGodunov.setCurrentTime(0.0);

  // Loop over all grids to get the maximum wave speed
  for (dit.begin(); dit.ok(); ++dit)
    {
      const Box& curBox = disjointBoxLayout.get(dit());

      // Set the current box and get the maximum wave speed on the current grid
      m_patchGodunov.setCurrentBox(curBox);

      // Get maximum wave speed for this grid
      Real speedOverBox = m_patchGodunov.getGodunovPhysicsPtr()->getMaxWaveSpeed(a_U[dit], curBox);

      // Compute a running maximum
      speed = Max(speed,speedOverBox);
    }

  // Gather maximum wave speeds and broadcast the maximum over these
  Vector<Real> allSpeeds;

  gather(allSpeeds,speed,uniqueProc(SerialTask::compute));

  if (procID() == uniqueProc(SerialTask::compute))
    {
      speed = allSpeeds[0];
      for (int i = 1; i < allSpeeds.size (); ++i)
        {
          speed = Max(speed,allSpeeds[i]);
        }
    }

  broadcast(speed,uniqueProc(SerialTask::compute));

  // Return the maximum wave speed
  return speed;
}

void LevelGodunov::highOrderLimiter(bool a_highOrderLimiter)
{
  CH_assert(m_isDefined);
  m_patchGodunov.highOrderLimiter(a_highOrderLimiter);
}

GodunovPhysics*
LevelGodunov::getGodunovPhysicsPtr()
{
  return  m_patchGodunov.getGodunovPhysicsPtr();
}

const GodunovPhysics*
LevelGodunov::getGodunovPhysicsPtrConst() const
{
  return  ((PatchGodunov&)m_patchGodunov).getGodunovPhysicsPtr();
}


#include "NamespaceFooter.H"
