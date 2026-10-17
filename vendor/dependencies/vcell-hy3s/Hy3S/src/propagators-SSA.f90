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

!!!DEC$ Define Debug_Time

Module Propagators_SSA

USE Randomgen
USE Priorityqueue
USE GlobalVariables
USE RateLaws
USE SpecialEvents

private      !All global module variables are private (accessible only from within this module)  unless specified as public

public :: Init_SSA, Final_SSA, SSA, Stats_SSA, Force_Update_SSA

!Definitions of variables used in a propogater that needs to be passed from one subroutine call to another
!By putting it in the module global (private) variable space, it does not need to be defined in the mainprogram

!Only make variables/subroutines public if they need to be accessed/called from outside this module

Integer, Allocatable, private :: reactioncount(:)


Real*8, Allocatable, private :: RxnResidual(:), a(:), der_a(:,:), tau(:), randnums(:), logrands(:)
Logical, Allocatable, private :: AllTrue(:), Fast(:)
Real*8, private :: Vdim, Initial_dt

Real*8, Parameter, private :: eps = 1.0d-9
!***


!Vdim = V / Vo

CONTAINS
!-------------------------------------------------------------------------------------------------------------------------
!****p SSA/Init_SSA
!NAME
!	Init_SSA
!PURPOSE
!	Initializes the stochastic simulation algorithm propagator.
!	Call before using the propagator.
!
!	Allocates all variables. Calculate reaction times. Initialize the heap sort and update it.
!
!	Has a logical flag for NewModel. NewModel = .TRUE. when a new model has been loaded.
!SOURCE

Subroutine Init_SSA(M, NewModel)
!Initialization subroutine for SSA
IMPLICIT NONE

integer, intent(in) :: M
Logical, intent(in) :: NewModel

!***

integer :: i, j
Real*8 :: GamRand, tworands(2), rand
Logical ::  TwoTrue(2)

if (NewModel) THEN

  Allocate(reactioncount(M))
  Allocate(AllTrue(M))
  Allocate(a(M))
  Allocate(tau(M))
  Allocate(RxnResidual(M))
  Allocate(randnums(M))
  Allocate(logrands(M))
  
  AllTrue = .TRUE.

end if

reactioncount=0
Vdim = 1.000000

Call Random_Number(randnums)

logrands = log(randnums)
RxnResidual = logrands

Call Fun_CalcA(AllTrue,real(Xo,8), a, Vdim)

!Initialize reaction times (excluding gamma-distributed events)
Where (a > 0.AND.Rxndata(:)%MType /= 5)
      tau = -RxnResidual / a + TStart
elsewhere
      tau = 1e20
end where

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

!****p SSA/Final_SSA
!NAME
!	Final_SSA
!PURPOSE
!	Called after the stochastic simulation propagator has finished simulating a trial.
!
!	Deallocates variables. Resets the heap sort.
!
!	NewModel = .TRUE. if a new model is loaded afterwards or .FALSE. if another trial is
!	simulated.
!SOURCE
 
Subroutine Final_SSA(NewModel)
IMPLICIT NONE

Logical, intent(in) :: NewModel

!***

if (NewModel) THEN

  Deallocate(reactioncount)
  Deallocate(AllTrue)
  Deallocate(a)
  Deallocate(tau)
  Deallocate(RxnResidual)
  Deallocate(logrands)
  Deallocate(randnums)

  !New Model
  !Reset both the priority queue and the dynamic sorted queue
  Call Queue_Reset(NewModel)
else
  !Not a new model - reset only dynamic sorted queue
  Call Queue_Reset(NewModel)
end if

End Subroutine
!--------------------------------------------------------------------------------------------------------------

!****p SSA/Stats_SSA
!NAME
!	Stats_SSA
!PURPOSE
!	Reports interesting statistics about the SSA propagator's results.
!SOURCE
Subroutine Stats_SSA(ReactioncountOut)
IMPLICIT NONE

!This subroutine simply passes variables from the private module space to the mainprogram (or any other scope which calls this subroutine)
!ReactioncountOut must be allocated to a length of M (M read from file and defined in dataio.f90)

Integer, intent(out) :: ReactioncountOut(:)

!***

ReactioncountOut = reactioncount

End Subroutine
!--------------------------------------------------------------------------------------------------------------

!****p propagators/SSA
!NAME
!	Stochastic Simulation Algorithm propagator
!PURPOSE
!	Runs the Next Reaction variant of the stochastic simulation algorithm on a model.
!SOURCE

Subroutine SSA(Xout, Xold, t, SaveTime, RandSeed, ierror)
IMPLICIT NONE

Real*8, intent(in) :: Xold(:), SaveTime
Real*8, intent(out) :: Xout(:)
Real*8, intent(inout) :: t
Integer, intent(out) :: ierror
Integer, intent(inout) :: RandSeed(:)

!***

Real*8 ::  mintau, mintauSQ, GamRand, tworands(2), TStop, X(N), rand

Integer :: mu, muSQ, i, j, GamRxn

Logical :: DGraphMask(M), TwoTrue(2)

TwoTrue = .TRUE.

TStop = t + SaveTime
X = Xold

do while (t <  TStop)

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

          if (any(X < 0)) THEN
		print*, "Warning! X = ", X
		print*, "Time = ", t
		!print*, "a = ", a
		print*, "Last Reaction executed =", mu
		ierror = 1
		return
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
             RxnResidual(mu) = log(rand)
           
          end if
          

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
            
          !Update reaction propensities, where X has changed.

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
		Call Queue_Update(i,tau(i))
	    end do

       end if

       !Update time
       t = mintau

       !Volume exponential growth
       Vdim = exp((t - TimeLastCellDivision) / CellGrowthTime)

    end do

End Subroutine
!--------------------------------------------------------------------------------------------------------------
!****p SSA/Force_Update_SSA
!NAME
!	Force_Update_SSA
!PURPOSE
!	Called to force an update of the reaction times when using the SSA
!	Used when changing the state of the system outside of the propagator,
!	such as when executing system perturbations or special events.
!SOURCE

Subroutine Force_Update_SSA(Mask, X, t)
IMPLICIT NONE

!Force an update of the reaction propensities and slow reaction times
!Only update slow reactions corresponding to true values in Mask
!Used when system is perturbed or when special events occur

Logical, intent(in) :: Mask(:)
Real*8, intent(in) :: X(:), t

!***

Integer :: i

Call Fun_CalcA(Mask, X, a, Vdim)

  Where (Mask.AND.a > 0)
     tau = -RxnResidual / a + t
  Elsewhere (Mask)
     tau = 1e20
  End where

do i=1,M
    if (Mask(i).AND.Rxndata(i)%MType /= 5) THEN
	Call Queue_Update(i,tau(i))
    end if
end do

End Subroutine
!--------------------------------------------------------------------------------------------------------------

End Module

