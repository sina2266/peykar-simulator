!Hy3S - Hybrid Stochastic Simulation for Supercomputers
!Copyright (C) 2004-2005  Howard Salis, Yiannis Kaznessis
!
!This program is free software; you can redistribute it and/or
!modify it under the terms of the GNU General Public License
!as published by the Free Software Foundation; either version 2
!of the License, or (at your option) any later version.
!
!This program is distributed in the hope that it will be useful,
!but WITHOUT ANY WARRANTY; without even the implied warranty of
!MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
!GNU General Public License for more details.
!
!You should have received a copy of the GNU General Public License
!along with this program; if not, write to the Free Software
!Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
!USA.

!---------------------------



!****m shared/propagators
!MODULE
!	All available propagators
!PURPOSE
!	A propagator is any algorithm that receives the state
!	of the system at an initial time and returns the
!	state of the system at some specified future time.
!	Each propagator utilizes a different algorithm to advance
!	the state of the system forward (or potentially backward).
!AUTHOR
!	Howard Salis (2004)
!SOURCE

!!DEC$ Define Debug_State

Module Propagators_HyJCMSS

USE Randomgen
USE Priorityqueue
USE GlobalVariables
USE RateLaws
USE SDEintegration
USE SpecialEvents

private      !All global module variables are private (accessible only from within this module)  unless specified as public

public :: Hybrid_HomSS, Init_HyHomSS, Final_HyHomSS, Stats_HyHomSS, Force_Update_HyHomSS

private :: ExecuteSlowRxns, PartitionReactions, CreateIndexes

!Definitions of variables used in a propogater that needs to be passed from one subroutine call to another
!By putting it in the module global (private) variable space, it does not need to be defined in the mainprogram

!Only make variables/subroutines public if they need to be accessed/called from outside this module

Integer, private :: Itercounter, Multicounter
Integer, Allocatable, private :: reactioncount(:), RandSeed_old(:)
Integer, Allocatable, private :: FastIndex(:), FastInverseIndex(:), BeforeFastIndex(:), BeforeFastInverseIndex(:)

Real*8, Allocatable, private :: RxnResidual(:), Rxnepsilon(:), a(:), der_a(:,:), tau(:), randnums(:), logrands(:), RxnEM(:)
Logical, Allocatable, private :: AllTrue(:), FastTemp(:), Fast(:), FastSpecies(:)
Real*8, private :: Vdim
Real*8, SAVE, private :: Initial_dt

Real*8, Parameter, private :: eps = 1.0d-9
Integer, Parameter, private :: MinMolecules = 20
!***


!Vdim = V / Vo

CONTAINS
!-----------------------------------------------------------------------------------------------------------------

!****p hyhomss/Init_HyHomSS
!NAME
!	Init_HyHomSS
!PURPOSE
!	Called before using the hybrid homogeneous stochastic simulator.
!	Allocates necessary allocatable variables. Initializes the heap sort.
!	Generates initial reaction times and updates the heap sort.
!
!	Has a NewModel logical option.
!	Use NewModel = .TRUE. when new model data has been loaded. Use .FALSE. otherwise.
!	For example, when simulating multiple trials of the same model, NewModel = .FALSE.
!SOURCE

Subroutine Init_HyHomSS(M, N, lambda, epsilon, NewModel)
IMPLICIT NONE
!Initialization subroutine for HyHomSS

integer, intent(in) :: M, N
Real*8, intent(in) :: lambda, epsilon
Logical, intent(in) :: NewModel

!***

integer :: i, j, RandSize
Real*8 :: GamRand, tworands(2), rand
Logical ::  TwoTrue(2)

if (NewModel) THEN

  Allocate(reactioncount(M))
  Allocate(AllTrue(M))
  Allocate(Fast(M))
  Allocate(FastTemp(M))
  Allocate(RxnResidual(M))
  Allocate(a(M))
  Allocate(der_a(M,N))
  Allocate(tau(M))
  Allocate(randnums(M))
  Allocate(logrands(M))
  Allocate(RxnEM(M))
  Allocate(FastSpecies(N))
  Allocate(FastIndex(M))
  Allocate(BeforeFastIndex(M))
  Allocate(Rxnepsilon(M))

  AllTrue = .TRUE.

  der_a = Real(0.00, 8)
  FastIndex = 0

  Call Random_Seed(Size = RandSize)
  Allocate(RandSeed_old(RandSize))

end if

Itercounter=0
reactioncount=0
Multicounter=0
Vdim = 1.000000

Fast = .FALSE.
FastTemp = .FALSE.

!Reset Initial_dt to maximum value
Initial_dt = 1e9

Call Random_Number(randnums)

logrands = log(randnums)
RxnResidual = logrands

Call Fun_CalcA(AllTrue, real(Xo,8), a, Vdim)

!Initialize reaction times (excluding gamma-distributed events)
Where (a > 0.AND.Rxndata(:)%MType /= 5)
      tau = -RxnResidual / a + TStart
elsewhere
      tau = 1e20
end where

Call PartitionReactions(AllTrue, real(Xo,8), a, Rxnepsilon, lambda, epsilon)

!Initialize SDE Numerical Integrator
Call SDE_Init(M, N, NewModel)

!Reset IPQ only if new model
Call Queue_Reset(NewModel)

!Initialize Heap space
Call Queue_Init(tau)

!For each initial gamma-distributed event, generate a waiting time and put it into the Dynamic Sorted Queue (SQ)
do i=1,M
      if (Rxndata(i)%Mtype == 5.AND.a(i) > 0) THEN
         do j=1,Xo(Rxndata(i)%SList(1))
            GamRand =  GammaRand(Rxndata(i)%Data,real(1.0,8)/Rxndata(i)%c(1))
            Call Queue_Add(i, GamRand + TStart)
         end do
      end if
end do

Call Init_SpecialEvents(TStart)

End Subroutine
!------------------------------------------------------------------------------------------------------------------

!****p hyhomss/Final_HyHomSS
!NAME
!	Final_HyHomSS
!PURPOSE
!	Called after using the hybrid homogeneous stochastic simulator.
!	Deallocates variables.
!	Resets the heap sort.
!
!	Has a NewModel logical flag. Use NewModel = .TRUE. when a new model will be read into memory.
!	Use .FALSE. if another trial will be simulated.
!SOURCE

Subroutine Final_HyHomSS(NewModel)
IMPLICIT NONE

Logical, intent(in) :: NewModel

!***

if (NewModel) THEN


   Deallocate(reactioncount)
   Deallocate(AllTrue)
   Deallocate(Fast)
   Deallocate(FastTemp)
   Deallocate(RxnResidual)
   Deallocate(a)
   Deallocate(der_a)
   Deallocate(tau)
   Deallocate(logrands)
   Deallocate(randnums)
   Deallocate(RxnEM)
   Deallocate(FastSpecies)
   Deallocate(FastIndex)
   Deallocate(BeforeFastIndex)
   Deallocate(Rxnepsilon)
   Deallocate(RandSeed_old)

   !New Model
   !Reset both the priority queue and the dynamic sorted queue
   Call Queue_Reset(NewModel)

   !New Model -- reset all indexes and the Brownian Tree
   Call SDE_Final(NewModel)

else
   !Not a new model - reset only dynamic sorted queue
   Call Queue_Reset(NewModel)

   !Not a new model -- reset only the Brownian Tree
   Call SDE_Final(NewModel)

end if

End Subroutine
!------------------------------------------------------------------------------------------------------------------

!****p propagators/hyhomss
!NAME
!	Hybrid_HomSS
!PURPOSE
!	The propagator for the hybrid homogeneous stochastic simulator.
!
!	Dynamically partitions reactions into fast/continuous and slow/discrete subsets.
!	Numerically integrates SDEs to describe effects of fast/continuous reactions.
!	Calculates the slow reaction times using the Multiple Slow Reaction approximation.
!
!	Also executes special events, such as cell division & gamma-distributed events.
!
!SOURCE

Subroutine Hybrid_HomSS(Xout, Xold, t, SaveTime, SDEdt, SDE_Tolerance, MSR_Tolerance, lambda, epsilon, RandSeed, ierror)
IMPLICIT NONE

Real*8, intent(in) :: Xold(:), SaveTime, lambda, epsilon, SDEdt, SDE_Tolerance, MSR_Tolerance
Real*8, intent(out) :: Xout(:)
Real*8, intent(inout) :: t
Integer, intent(out) :: ierror
Integer, intent(inout) :: RandSeed(:)

!***

Real*8 ::  mintau, mintauSQ, GamRand, tworands(2), rand, PreviousTime
Real*8 ::  TStop, TStart, deltat, Initial_dt_old, Max_dt, SpecialEvent_dt
Real*8 ::  X(N), dXfast(N), dXslow(N), NumFractionalRxns

Real*8 :: X_old(N), a_old(M), RxnResidual_old(M), time_old
Real*8 :: t0, t1, t2, t3

Real*8, Allocatable :: dW(:)

Integer :: mu, muSQ, muvector(M), i, j, stepcounter
Integer :: NumRxnOccur, GamRxn, NegPos
Integer :: MultipleSlowRxn(M), MFast, BeforeMFast
Integer :: SpecialEventNumber, Current_Tree_Row
Integer :: reactioncount_old(M), Multicounter_old

Logical :: DGraphMask(M), TwoTrue(2), ErrorOk, Once
Logical :: SpecialEvent, SpecialEventOnceThrough, SlowRxnOccurredMask(M)

Character(len = 256) :: errmsg

TwoTrue = .TRUE.
SpecialEventOnceThrough = .FALSE.
SpecialEvent = .FALSE.

TStop = t + SaveTime
TStart = t
Max_dt = min(SDEdt, SaveTime)
Initial_dt = min(Max_dt, Initial_dt)
Initial_dt_old = Initial_dt
X = Xold

!Convention: The reaction propensities LEAVE this subroutine updated.
!Therefore, this subroutine assumes that the reaction propensities ENTER as currently updated.

!Reactions are initially classified as Fast vs. Slow in the Init subroutine
!At the END of this subroutine, then, they are reclassified as Fast vs. Slow
!Reactions are only reclassified if their reactant or product species or propensities have changed.

!The flow is then:    a_t -> Call subroutine (modify X, t -> t + dt, update a_t+dt, iterate) -> a_TStop

do while ((TStop - t)/(TStop - TStart) > eps)          !Main time iterative loop


    !For reactions that changed their classification, reset their Residuals and/or recalculate their reaction times

    !FAST --> SLOW : Reset Residual to initial condition and recompute reaction time.

    where (.NOT.Fast.AND.(FastTemp.XOR.Fast).AND.a > 0)
       !RxnResidual = logrands
       tau = -RxnResidual / a + t
    elsewhere (.NOT.Fast.AND.(FastTemp.XOR.Fast))
       !RxnResidual = logrands
       tau = real(1e20,8)
    end where

    if (any(.NOT.Fast.AND.(FastTemp.XOR.Fast))) THEN
       do i=1,M
          if (.NOT.Fast(i).AND.(FastTemp(i).XOR.Fast(i))) THEN

	     !Update reaction times
             Call Queue_Update(i,tau(i))

          end if
       end do
    end if


    !SLOW --> FAST:  Reset these fast reaction times to infinity

    if (any(Fast.AND.(FastTemp.XOR.Fast))) THEN
       do i=1,M
          if (Fast(i).AND.(FastTemp(i).XOR.Fast(i))) THEN

             Call Queue_Update(i,real(1e20,8))
          end if
       end do
    end if

    !If there are fast reactions, perform the hybrid CLE/SSA code
    !If there are only slow reactions, perform the Next Reaction variant of the SSA.

    if (any(Fast)) THEN

#if defined(MILSTEIN) || defined(ADAPTIVE)

      Call Derivatives_Propensities(Fast, X, der_a, Vdim)

#endif

      Call CreateIndexes(FastTemp, Fast) !Creates FastSpecies, FastIndex, FastInverseIndex, BeforeFastIndex, & BeforeFastInverseIndex

      Call SDE_CallOnClassifyFast(M, FastIndex, FastInverseIndex, BeforeFastIndex, BeforeFastInverseIndex, FastTemp, Fast)

#if defined(DEBUG_STATE)
      print*, "DEBUGGING HYBRID CODE STATE"
      print*, "X_t = ", X
      print*, "a_t = ", a
      print*, "Reaction Residuals = ", RxnResidual
      print*, "t = ", t
      print*, "FastIndex = ", FastIndex
      print*, "FastInverse = ", FastInverseIndex
      print*, "Fast Reactions = ", Fast
      print*, "FastSpecies = ", FastSpecies
      print*, "Next Special Event (mu) = ", Queue_MinRxnSQ()
      print*, "Next Special Event Time = ", Queue_MinTimeSQ()
#endif

       Mfast = count(Fast)

       !Increment hybrid iteration counter
       Itercounter = Itercounter + 1

#if defined(DEBUG_STATE)
	print*, "SDE Iteration # = ", Itercounter
#endif


       Allocate(dW(Mfast))

#if defined(ADAPTIVE)

       !Parameters that define where in the Brownian path the system is :: Tree_Row, Branch

       !Determine the next time step (deltat)
       !Check for an approaching: SaveTime, SpecialEventTime, ...
       !If not a special case, then deltat = Unit_dt / 2**(Tree_Row - 1)

       !print*, "BTree Reset because SaveTime approaches? ", (Initial_dt*(1 - real(Branch,8) / real(2**(Tree_Row-1),8)) - SaveTime > eps)

       if (Initial_dt*(1 - real(Branch,8) / real(2**(Tree_Row - 1),8)) - SaveTime > eps) THEN
          Branch = 1
          Tree_Row = 1

	  !print*, "Approaching SaveTime! Generating new Brownian Path with smaller Initial_dt"

          Initial_dt_old = Initial_dt
          Initial_dt = SaveTime

          !Generate Brownian Path, starting from the top row and going down to Tree_Row
          Call SDE_BrownianPath_Generate(Fast, 0, Tree_Row, Reset = .TRUE.)

       end if

       !Save the state of the system, the reaction residuals, and the queue states for
       !possible rewind of the execution of the slow reactions

       !There is no loss of accuracy if the state of the system (including the values of all of the random numbers)
       !are exactly the same when moving forward again from a rewind

       X_old = X
       RxnResidual_old = RxnResidual
       time_old = t
       a_old = a
       reactioncount_old = reactioncount
       Multicounter_old = Multicounter
       Call Random_Seed(Get = RandSeed_old)

       Call Queue_SaveSnapshot

       !******** Adaptive Time Stepping Loop **********

       !Parameter that checks if the adaptive time step loop is done, based on the calculation of the error
       ErrorOk = .FALSE.

       !Once is true when ExecuteSlowRxns has been called at least once
       Once = .FALSE.

       Do while(.NOT.ErrorOk)   !Main Adaptive time stepping loop

          ErrorOk = .TRUE.

          deltat = Initial_dt / real(2**(Tree_Row - 1),8)

	  !Set the Brownian increment for this time
          dW = Brownian_Paths(Tree_Row)%Row(1 + (Branch-1)*Mfast : Branch*Mfast) * sqrt(Initial_dt)

#if defined(DEBUG_STATE)
	  print*, "*** Adaptive Loop ***"
	  print*, "Looped Tree Row = ", Tree_Row
	  print*, "Looped Branch = ", Branch
	  print*, "Looped deltat = ", deltat
	  print*, "Looped dW = ", dW
#endif

	  !Check criteria for halving or doubling time step
          Call SDE_Check_Criteria(Fast, FastSpecies, X, deltat, dW, a, der_a, Vdim, SDE_Tolerance, ErrorOk)

          if (ErrorOk) THEN

#if defined(MILSTEIN)

             !Compute Milstein terms and sum them all together to get dXfast

             Call SDE_Milstein_2DTerms(Fast, FastSpecies, FastIndex, a, der_a, deltat, dW)
             Call SDE_DriftDiffusion(Fast, FastIndex, a, deltat, dW, RxnEM)
             Call SDE_Explicit_Milstein(Fast, FastSpecies, N, RxnEM, dXfast)
#else

             !Compute Euler-Maruyama terms and sum them all together to get dXfast

             Call SDE_DriftDiffusion(Fast, FastIndex, a, deltat, dW, RxnEM)
             Call SDE_Explicit_EulerMaruyama(Fast, M, RxnEM, dXfast)

#endif

             if (any(X + dXfast < MinMolecules.AND.FastSpecies)) THEN

#if defined(DEBUG_STATE)
	        print*, "Some number of species has gone negative. Criteria kicking in. ErrorOk = F"
#endif
                ErrorOK = .FALSE.
             else

                if (Once) THEN

		   RxnResidual = RxnResidual_old
		   a = a_old
		   X = X_old
		   t = time_old
		   reactioncount = reactioncount_old
		   Multicounter = Multicounter_old
		   Call Random_Seed(Put = RandSeed_old)

                   Call Queue_LoadSnapshot

#if defined(DEBUG_STATE)
		   print*, "***Reloading snapshot of State***"
#endif

                end if

                Call ExecuteSlowRxns(X, t, RxnResidual, a, deltat, MSR_Tolerance, dXslow, SpecialEventNumber, SpecialEvent, &
                     & SpecialEvent_dt, SlowRxnOccurredMask, reactioncount, Multicounter, ErrorOK)
                Once = .TRUE.

                !Check if a special event occurred, causing the execution of the slow reactions to stop and the unit time step to be changed.
                if (SpecialEvent) THEN

		   !Alter time step to special event dt and recycle through loop
		  if (.NOT.SpecialEventOnceThrough) THEN
                     ErrorOK = .FALSE.
		     SpecialEventOnceThrough = .TRUE.

                     Initial_dt_old = Initial_dt
                     Initial_dt = SpecialEvent_dt
                     deltat = SpecialEvent_dt

                     Tree_Row = 1
                     Branch = 1
                     !Generate Brownian Path, starting from the top row and going down to Tree_Row
                     Call SDE_BrownianPath_Generate(Fast, 0, Tree_Row, Reset = .TRUE.)

                     cycle !Goes to beginning of while do loop

		  else	!Reset ErrorOk to True (sets to False in ExecuteSlowRxns because a SpecialEvent occurs)
		     ErrorOk = .TRUE.
		  end if

                end if


             end if

          end if

          Current_Tree_Row = Tree_Row

          !Change deltat if necessary
          !Change Tree_Row and Branch
          Call SDE_Adaptive_Time_Stepping(ErrorOk, Tree_Row, Branch)

          !Generate additional rows of the Brownian tree, if necessary
          !Nothing happens if Current_Tree_Row > Tree_Row
          Call SDE_BrownianPath_Generate(Fast, Current_Tree_Row, Tree_Row, Reset = .FALSE.)

          if (Tree_Row >= 15) THEN
			  if (brownianTreeWarningCount .EQ. 0) THEN
				 print*, "***WARNING***: The Brownian Tree Is Allocated To >=15 Rows. "
				 print*, "A Segmentation Fault (Out of Memory) May Be Approaching!"
			  endif
			  brownianTreeWarningCount = brownianTreeWarningCount + 1
		  end if

#if defined(DEBUG_STATE)
	  print*, "Looped dXfast = ", dXfast
          print*, "Looped dXslow = ", dXslow
	  print*, "*** Adaptive Loop ***"
#endif

       end do

       !Did we finish transversing the Brownian Path? If so, reset, but keep the Tree_Row number as the same.
       If (Branch == 2**(Tree_Row - 1) + 1) THEN
          Branch = 1

          !Generate Brownian Path, starting from the top row and going down to Tree_Row
          Call SDE_BrownianPath_Generate(Fast, 0, Tree_Row, Reset = .TRUE.)

       End if

#if defined(DEBUG_STATE)
       print*, "Adaptive Code Being Used"
       print*, "Tree Row = ", Tree_Row
       print*, "Branch = ", Branch
       print*, "Allocated BTree Rows = ", PathSize
       print*, "Final chosen deltat = ", deltat
       print*, "Final dXfast = ", dXfast
       print*, "Final dXslow = ", dXslow
#endif

#else

       deltat = min(Initial_dt, TStop - t)
       Branch = 1
       Tree_Row = 1

       Call ExecuteSlowRxns(X, t, RxnResidual, a, deltat, MSR_Tolerance, dXslow, SpecialEventNumber, SpecialEvent, SpecialEvent_dt, &
            & SlowRxnOccurredMask, reactioncount, Multicounter, ErrorOK)

       deltat = min(deltat, SpecialEvent_dt)

       dW = Brownian_Paths(Tree_Row)%Row(1 + (Branch-1)*Mfast : Branch*Mfast) * sqrt(deltat)

#if defined(MILSTEIN)
       Call SDE_Milstein_2DTerms(Fast, FastSpecies, FastIndex, a, der_a, deltat, dW)
       Call SDE_DriftDiffusion(Fast, FastIndex, a, deltat, dW, RxnEM)
       Call SDE_Explicit_Milstein(Fast, FastSpecies, N, RxnEM, dXfast)

#else
       Call SDE_DriftDiffusion(Fast, FastIndex, a, deltat, dW, RxnEM)
       Call SDE_Explicit_EulerMaruyama(Fast, M, RxnEM, dXfast)

#endif

       if (any(X + dXslow + dXfast < 0)) THEN
       	  errmsg = 'The fixed method is unstable! The number of molecules of a species has gone negative!'

#if defined(USE_MESSAGING)
		  Call send_failed(errmsg)
#else
          open (unit = 0)
          write (0,*), errmsg
	      write (0,*), "Program is Stopping."
	      close(unit = 0)
#endif

	  	  print*, "X = ", X + dXslow + dXfast
	  	  print*, "Time = ", t + deltat
	  	  ierror = 1
	  	  return
       end if

       !Generate the top row Brownian Path
       Call SDE_BrownianPath_Generate(Fast, 0, 1, Reset = .TRUE.)

#if defined(DEBUG_STATE)
	print*, "Fixed Code Being Used"
	print*, "Chosen deltat = ", deltat
        print*, "Final dXfast = ", dXfast
        print*, "Final dXslow = ", dXslow
#endif

#endif

       Deallocate(dW)

       !Update the state of the system and the time

       X = X + dXslow + dXfast

       t = t + deltat


       !Execute any special events that may have occurred
       !If one has occurred, reset the unit time step of the SDE integration to the maximum one

       if (SpecialEvent) THEN

          !Reset unit time step of the integrator to the old one
          SpecialEvent = .FALSE.
	  SpecialEventOnceThrough = .FALSE.

          Initial_dt = Initial_dt_old

          Call Execute_SpecialEvents(X, t, SpecialEventNumber)

          !Update everything
	  DGraphMask = .TRUE.

          !Call Force_Update_HyHomSS(AllTrue, X, t)

       else

          !Update reaction propensities and reaction times affected by fast reactions

          DGraphMask = .FALSE.

          do i=1,M
             if (Fast(i)) THEN
                DGraphMask( (/ DGraph(i)%List(1:DGraph(i)%ListLen) /) ) = .TRUE.
             end if
          end do

       end if

       Call Fun_CalcA(DGraphMask, X, a, Vdim)

       !Update slow reaction times affected by fast reactions
       where (.NOT.Fast.AND.DGraphMask.AND.Rxndata(:)%MType /= 5.AND.a > 0)
          tau = -RxnResidual / a + t
       elsewhere (.NOT.Fast.AND.DGraphMask.AND.Rxndata(:)%MType /= 5)
          tau = real(1e20,8)
       end where

       do i=1,M
          if (.NOT.Fast(i).AND.DGraphMask(i).AND.Rxndata(i)%Mtype /= 5) THEN
             Call Queue_Update(i,tau(i))
          end if
       end do


       !All reaction propensities and slow reaction times should be updated to X_t+dt at this point

       !Volume exponential growth
       Vdim = exp((t - TimeLastCellDivision) / CellGrowthTime)


    else !NO FAST REACTIONS, corresponding to IF any(fast)

       !Perform Next Reaction SSA code
       !print*, "Next Reaction SSA code used!"

       mintau = Queue_MinTimeIPQ()
       mu = Queue_MinRxnIPQ()

       muSQ = Queue_MinRxnSQ()
       mintauSQ = Queue_MinTimeSQ()

       if (mintau >= mintauSQ) THEN
          mu = muSQ
          mintau = mintauSQ
       end if

       if (mintau >= TStop) THEN

         !If the next reaction time is greater than TStop, then the state of the system
         !at t = TStop is X and the time to save is TStop.
         !Do not execute this reaction or remove it from the queue. It may be executed during the next
         !call to this subroutine.

          Xout = X

	  !Update Reaction Residuals with leftover time
	  where (a > 0)
	    RxnResidual = RxnResidual + a*(Tstop - t)
	  end where

          t = TStop
          ierror = 0

          return
       end if


       if (mu > 0) THEN
          !This is a normal reaction

          !Increment counter
          reactioncount(mu) = reactioncount(mu) + 1

          !Update state
          X( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen) /) ) = X( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen) /) ) + &
          & Rxndata(mu)%v(1:Rxndata(mu)%SListLen)


          !If X goes negative (due to fractional values of X), then execute a fraction of the reaction event to return
	  !the state vector, X, to near-integer numbers (physically weird, but a decent solution to a hard problem).
          if (any(X < 0)) THEN

	   	 NegPos = minloc(X, dim = 1)

		 do i=1, Rxndata(mu)%SlistLen
		    if (Rxndata(mu)%SList(i) == NegPos) THEN
			NumFractionalRxns = abs(X(NegPos)) / Rxndata(mu)%v(i)
			X( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen)/) )  = X( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen)/) ) + &
			& NumFractionalRxns * Rxndata(mu)%v(1:Rxndata(mu)%SListLen)
		    end if
		 end do

 	  else

            !If reaction execution creates or cancels the possibility of another event, add or delete that event from the queue

            if (Rxndata(mu)%EListLen > 0) THEN
               do i=1,Rxndata(mu)%EListLen

                !Assume creation /destruction of a single gamma-distributed event.
                !Creation => EStoichList = +, Destruction => EStoichList = -
                !Could easily do other distributions by using if Mtype == #, then ...

                GamRxn = Rxndata(mu)%EList(i)

                if (Rxndata(mu)%EStoichList(i) > 0) THEN

                   !Draw gamma distributed waiting time with rate 1/k and steps N

                   GamRand = GammaRand(Rxndata(GamRxn)%Data, real(1.00,8) / Rxndata(GamRxn)%c(1))
                   Call Queue_Add(GamRxn, GamRand + mintau)

                   !print*, "Gamma event created for time = ", GamRand + mintau
                   !print*, "Next Gamma event at time = ", Queue_MinTimeSQ()

                else
                   Call Queue_CancelEvent(GamRxn)

                   !print*, "Gamma event cancelled at time = ", mintau

                end if
              end do
            end if

	  end if

          !Update reaction residuals
          Where (a > 0.AND.Rxndata(:)%Mtype /= 5)
             RxnResidual = RxnResidual + a*(mintau - t)
          End where

          if (Rxndata(mu)%Mtype == 5) THEN

             !If reaction is gamma-distributed, then delete the event from the queue.
             Call Queue_Delete(int(1))
             !print*, "Gamma event occurred at time = ", mintau

          else

             !This is a normal slow reaction event
             Call Random_Number(rand)
             RxnResidual(mu)=log(rand)
          end if

          DGraphMask = .FALSE.
          DGraphMask( (/DGraph(mu)%List(1:DGraph(mu)%ListLen) /) ) = .TRUE.

	  Call Fun_CalcA(DGraphMask, X, a, Vdim)

          Where (DGraphMask.AND.a > 0.AND.Rxndata(:)%Mtype /= 5)
             tau = -RxnResidual / a + mintau
          elsewhere (DGraphMask)
             tau = real(1e20,8)
          end where

          !Update the reaction times in the queue

          do i=1,DGraph(mu)%ListLen
             if (Rxndata(DGraph(mu)%List(i))%MType /= 5) THEN
                Call Queue_Update(DGraph(mu)%List(i),tau(DGraph(mu)%List(i)))
             end if
          end do


       else
            !These are SPECIAL EVENTS

            Call Execute_SpecialEvents(X, mintau, mu)
            DGraphMask = .TRUE. !Update everything

            Call Fun_CalcA(DGraphMask, X, a, Vdim)

            Where (a > 0.AND.Rxndata(:)%Mtype /= 5)
               tau = -RxnResidual / a + mintau
            elsewhere
               tau = real(1e20,8)
            end where

            do i=1, M
		if (Rxndata(i)%MType /= 5) THEN
                   Call Queue_Update(i,tau(i))
		end if
            end do

       end if

#if defined(DEBUG_STATE)
	print*, "SSA Code Being Used"
	print*, "mu = ", mu
	print*, "deltat (mintau) = ", mintau - t
	print*, "X = ", X
#endif

       !Update time
       t = mintau

       !Volume exponential growth
       Vdim = exp((t - TimeLastCellDivision) / CellGrowthTime)


    end if                      !End Hybrid vs. SSA IF THEN

    !Partition reactions into Fast vs. Slow

    Call PartitionReactions(DGraphMask.OR.SlowRxnOccurredMask, X, a, Rxnepsilon, lambda, epsilon)

 end do                         !End time iteration from t to TStop

 Initial_dt = Initial_dt_old

#if defined(DEBUG_STATE)
 print*, "Exiting X = ", X
 print*, "Exiting t = ", t
 print*, "Exiting Initial_dt = ", Initial_dt
#endif


 !Data Saving
 Xout = X

 ierror = 0

 !Debugging
 !print*, "State vector = ", X
 !print*, "Time = ", t
 !print*, "Vdim = ", Vdim
 !print*, "TimeLastCellDiv = ", TimeLastCellDiv

End Subroutine Hybrid_HomSS
!--------------------------------------------------------------------------------------------------------------
Subroutine ExecuteSlowRxns(X, t, RxnResidual, a, deltat, MSR_Tolerance, dXslow, SpecialEventNumber, SpecialEvent, &
& SpecialEvent_dt, RxnOccurredMask, reactioncount, Multicounter, ErrorOK)
IMPLICIT NONE

Real*8, intent(in) :: X(:), t, deltat, MSR_Tolerance

Integer, intent(out) :: SpecialEventNumber
Logical, intent(out) :: SpecialEvent, ErrorOK, RxnOccurredMask(:)
Real*8, intent(out) :: SpecialEvent_dt, dXSlow(:)

Integer, intent(inout) :: reactioncount(:), Multicounter
Real*8, intent(inout) :: RxnResidual(:), a(:)

!!Integer :: M, N <--- Module public variable
!!Type (RxndataType) :: Rxndata(:) <--- Module public variable

Integer :: mu, muSQ, GamRxn, i, NegPos
Real*8 :: mintau, mintauSQ, PreviousTime, rand, GamRand, tworands(2)
Real*8 :: NumFractionalRxns

Logical :: TwoTrue(2), DGraphMask(M), Once

dXslow=0.00d0
ErrorOK = .TRUE.
SpecialEvent = .FALSE.
RxnOccurredMask = .FALSE.
SpecialEvent_dt = real(1e20,8)
SpecialEventNumber = 0

Once = .FALSE.

!Find minimum reaction/event time
mu = Queue_MinRxnIPQ()
mintau = Queue_MinTimeIPQ()

muSQ = Queue_MinRxnSQ()
mintauSQ = Queue_MinTimeSQ()

if (mintau >= mintauSQ) THEN
   mu = muSQ
   mintau = mintauSQ
end if

PreviousTime = t

do while (mintau <= t + deltat.AND.ErrorOK)

   If (mu > 0) THEN    !This is a normal slow reaction event

      !Update state vector based on slow reaction
      dXslow( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen) /) ) = dXslow( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen) /) ) + &
           & Rxndata(mu)%v(1:Rxndata(mu)%SListLen)


      !Check if the Multiple Slow Reaction (MSR) approximation is invalid.
      !Must allow at least one slow reaction to occur. (Once is true when 1 has occurred.)
      !If dXslow / X for any fast species (species affected by a fast reaciton) is greater than the tolerance, then stop executing slow reactions

      If (Once.AND.any(abs(dXslow) / X > MSR_Tolerance.AND.FastSpecies)) THEN

         ErrorOK = .FALSE.
	 SpecialEvent_dt = PreviousTime - t

#if defined(DEBUG_STATE)
	print*, "MSR criteria invalid. Rewinding to snapshot and decreasing time step."
	print*, "mu = ", mu
	print*, "dXslow = ", dXslow
	print*, "X = ", X
#endif

	 !Reverse the effects of this last slow reaction.
         dXslow( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen) /) ) = dXslow( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen) /) ) - &
              & Rxndata(mu)%v(1:Rxndata(mu)%SListLen)

         return

      end if


      !Check if the slow reaction causes X to go negative (due to fractional numbers)
      !No perfect solution to this problem, But ...

      !If X goes negative due to a slow reaction, a fractional portion of the reaction event is executed so that
      !the state vector X is returned to near-integer value. This will conserve mass, but is still physically weird.

      If (any(X + dXslow < 0)) THEN

	 NegPos = minloc(X+dXslow, dim = 1)

	 do i=1, Rxndata(mu)%SlistLen
	    if (Rxndata(mu)%SList(i) == NegPos) THEN
		NumFractionalRxns = abs( X(NegPos)+dXslow(NegPos) ) / real(Rxndata(mu)%v(i),8)
		dXSlow( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen)/) )  = dXSlow( (/Rxndata(mu)%SList(1:Rxndata(mu)%SListLen)/) ) + &
		& NumFractionalRxns * Rxndata(mu)%v(1:Rxndata(mu)%SListLen)
	    end if
	 end do

      else


         !Increment counters
         reactioncount(mu) = reactioncount(mu) + 1
         Multicounter = Multicounter + 1
	 RxnOccurredMask(mu) = .TRUE.


         !Check if mu'th event generates a gamma distributed waiting time
         !If so, draw a gamma distributed waiting time and add it to the queue
         !example: for S1 --> S2, gamma distributed, each production of a S1 molecule
         !adds a gamma distributed waiting time to the queue for the production of S2

         if (Rxndata(mu)%EListLen > 0) THEN
            do i=1,Rxndata(mu)%EListLen
               !Assume event is gamma-distributed now.
               !Could easily do if Mtype == #, then ... for other types of events

               GamRxn = Rxndata(mu)%EList(i)

               !If EStoichList is positive, event is generated
               !If EStoichList is negative, event is cancelled

               if (Rxndata(mu)%EStoichList(i) > 0) THEN

                  !Draw gamma distributed waiting time with rate 1/k and steps N

                  GamRand = GammaRand(Rxndata(GamRxn)%Data, real(1.00,8) / Rxndata(GamRxn)%c(1))
                  Call Queue_Add(GamRxn, GamRand + mintau)

                  !print*, "Gamma event created for time = ", GamRand + mintau
                  !print*, "Next Gamma event at time = ", Queue_MinTimeSQ()


               else
                  Call Queue_CancelEvent(GamRxn)

                  !print*, "Gamma event cancelled at time = ", mintau

               end if
            end do
         end if

      end if

      !Increment Reaction Residuals
      Where (.NOT.Fast.AND.a > 0.AND.Rxndata(:)%Mtype /= 5)
         RxnResidual = RxnResidual + a*(mintau - PreviousTime)
      End where

      !Check if mu is a gamma distributed event
      !If so, execute corresponding action and delete event time from queue
      !If not, reset reaction residual to initial conditions

      if (Rxndata(mu)%Mtype == 5) THEN
         !Gamma distributed event - Execute reaction, remove reaction time from Dynamic Sorted Queue
         Call Queue_Delete(1)

         !print*, "Gamma event occurred at time = ", mintau
         !print*, "Next Gamma event at time = ", Queue_MinTimeSQ()

      else
         !This is an exponentially distributed slow reaction event
         Call Random_Number(rand)
         logrands(mu) = log(rand)
         RxnResidual(mu) = logrands(mu)
      end if

      !Volume exponential growth
      Vdim = exp((mintau - TimeLastCellDivision) / CellGrowthTime)

      !Recalculate the reaction propensities. But only where X has changed.

      DGraphMask = .FALSE.
      DGraphMask( (/DGraph(mu)%List(1:DGraph(mu)%ListLen) /) ) = .TRUE.

      Call Fun_CalcA(.NOT.Fast.AND.DGraphMask, X + dXslow, a, Vdim)

      !Recompute the reaction times, where a has changed.

      Where (.NOT.Fast.AND.DGraphMask.AND.a > 0.AND.Rxndata%Mtype /= 5)
         tau = -RxnResidual / a + mintau
      Elsewhere (.NOT.Fast.AND.DGraphMask.AND.Rxndata%MType /= 5)
         tau = real(1e20,8)
      End where

      !Update the reaction times in the priority queue

      do i=1,DGraph(mu)%ListLen
         if (.NOT.Fast(DGraph(mu)%List(i)).AND.Rxndata(DGraph(mu)%List(i))%MType /= 5) THEN
            Call Queue_Update(DGraph(mu)%List(i),tau(DGraph(mu)%List(i)))
         end if
      end do


   end if

   if (mu < 0) THEN !Special Event

      !This is a SPECIAL EVENT. STOP the execution of the slow reactions, numerically integrate the SDE, and  update the entire system
      !before executing the effects of this event.

      SpecialEvent = .TRUE.
      SpecialEvent_dt = mintau - t
      SpecialEventNumber = mu
      ErrorOK = .FALSE.

      !Subroutine will exit on this cycle.
   end if

   PreviousTime = mintau

   mu = Queue_MinRxnIPQ()
   mintau = Queue_MinTimeIPQ()

   muSQ = Queue_MinRxnSQ()
   mintauSQ = Queue_MinTimeSQ()

   if (mintau >= mintauSQ) THEN
      mu = muSQ
      mintau = mintauSQ
   end if

Once = .TRUE.
end do

!Update Rxn Residuals with remainder of time between last Rxn event and SDE integration
Where (.NOT.Fast.AND.a > 0.AND.Rxndata(:)%Mtype /= 5)
   RxnResidual = RxnResidual + a*(t + deltat - PreviousTime)
End Where


End Subroutine
!--------------------------------------------------------------------------------------------------------------
!****p hyhomss/Stats_HyHomSS
!NAME
!	Stats_HyHomSS
!PURPOSE
!	Reports interesting statistics about the results of the simulation.
!SOURCE

Subroutine Stats_HyHomSS(ReactioncountOut, ItercounterOut, MulticounterOut)
IMPLICIT NONE

Integer, intent(out) :: ReactioncountOut(:), ItercounterOut, MulticounterOut

!***

!This subroutine simply passes variables from the private module space to the mainprogram (or any other scope which calls this subroutine)
!ReactioncountOut must be allocated to a length of M (M read from file and defined in DataInput.f)

ReactioncountOut = reactioncount
ItercounterOut = Itercounter
MulticounterOut = Multicounter

End Subroutine Stats_HyHomSS
!--------------------------------------------------------------------------------------------------------------
Subroutine PartitionReactions(Mask, X, a, Rxnepsilon, lambda, epsilon)
IMPLICIT NONE

Logical, intent(in) :: Mask(:)
Real*8, intent(in) :: X(:), a(:), lambda, epsilon

Real*8, intent(inout) :: Rxnepsilon(:)
!Logical, intent(inout) :: FastTemp(:), Fast(:)

Integer :: i, j

!Ignoring stoichiometric coefficient when computing rxnepsilon because
!DList species may not have one (if it is not in SList, for example)

    do i=1,M

      if (Mask(i)) THEN

            Rxnepsilon(i)=1.0d6
            do j=1,Rxndata(i)%DlistLen
               Rxnepsilon(i) = min(Rxnepsilon(i),X(Rxndata(i)%DList(j)))
            end do

            do j=1,Rxndata(i)%SlistLen
               Rxnepsilon(i) = min(Rxnepsilon(i),X(Rxndata(i)%SList(j)) / abs(Rxndata(i)%v(j)))
            end do

      end if

    end do

    FastTemp = Fast

    where (Rxnepsilon >= epsilon.AND.a >= lambda.AND.Rxndata(:)%Mtype /= 5)
       Fast = .TRUE.
    elsewhere
       Fast = .FALSE.
    end where

End Subroutine
!--------------------------------------------------------------------------------------------------------------
Subroutine CreateIndexes(BeforeFast, AfterFast)
IMPLICIT NONE

Logical, intent(in) :: BeforeFast(:), AfterFast(:)

Integer :: i, MFast, BeforeMFast, counter

!!Integer, intent(inout) :: BeforeFastIndex(:), BeforeFastInverseIndex(:), FastIndex(:), FastInverseIndex)(:) <-- Module private variables
!!Logical, intent(inout) :: FastSpecies <--- Module private variables

MFast = count(AfterFast)
BeforeMFast = count(BeforeFast)

!Create FastIndex & FastInverseIndex, but save the old ones as well
BeforeFastIndex = FastIndex

if (Allocated(BeforeFastInverseIndex)) THEN
   Deallocate(BeforeFastInverseIndex)
   Allocate(BeforeFastInverseIndex(BeforeMfast))
   BeforeFastInverseIndex = FastInverseIndex
end if

if (Allocated(FastInverseIndex)) THEN
  Deallocate(FastInverseIndex)
end if

Allocate(FastInverseIndex(Mfast))

counter=0
FastIndex=0
FastInverseIndex=0

do i=1,M
   if (AfterFast(i)) THEN
      counter = counter + 1
      FastIndex(i) = counter
      FastInverseIndex(counter) = i
   end if
end do

!Create the list of species affected by fast reactions
FastSpecies = .FALSE.
Forall (i=1:M, AfterFast(i) )
   FastSpecies( (/Rxndata(i)%SList(1:Rxndata(i)%SListLen) /) ) = .TRUE.
end forall

End Subroutine CreateIndexes
!--------------------------------------------------------------------------------------------------------------
!****p hyhomss/Force_Update_HyHomSS
!NAME
!	Force_Update_HyHomSS
!PURPOSE
!	Calling this subroutine forces a subset (or all) of the reactions to be updated.
!	This may be required when changing the state of the system outside of the propagator,
!	such as executing a system perturbation.
!SOURCE

Subroutine Force_Update_HyHomSS(Mask, X, t)

!Force an update of the reaction propensities and slow reaction times
!Only update slow reactions corresponding to true values in Mask
!Used when system is perturbed or when special events occur
IMPLICIT NONE

Logical, intent(in) :: Mask(:)
Real*8, intent(in) :: X(:), t
!***

Real*8 :: tau(M)
Integer :: i

Call Fun_CalcA(Mask, X, a, Vdim)

#if defined(MILSTEIN)

Call Derivatives_Propensities(Mask, X, der_a, Vdim)

#endif

Where (.NOT.Fast.AND.Mask.AND.a > 0)
   tau = -RxnResidual / a + t
Elsewhere (Mask)
   tau = 1e20
End where

do i=1,M
    if (Mask(i).AND.Rxndata(i)%MType /= 5) THEN
	Call Queue_Update(i,tau(i))
    end if
end do

End Subroutine Force_Update_HyHomSS
!--------------------------------------------------------------------------------------------------------------
End Module
