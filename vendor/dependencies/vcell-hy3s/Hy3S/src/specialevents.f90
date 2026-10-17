Module SpecialEvents

USE Globalvariables
USE Randomgen
USE Priorityqueue

!This module contains any special events, or transitions, which are not
!easily described as a system of Markovian biochemical reactions.

!So far, the list includes cell division events and external system perturbations.

!Gamma-distributed events are considered 'special', but they have
!been incorporated directly into the propagators because it is more
!computationally efficient to do so as they may be frequently executed

!Module variable space
!Try to keep public variables to a minimum.

public :: Init_SpecialEvents, Execute_SpecialEvents
Integer, private, parameter :: PerturbNumOffset = 1000

!***Important*** To handle both Integer and Real*8 State Vectors, the
!Execute_SpecialEvents subroutines are overloaded! 

Interface Execute_SpecialEvents
   Module Procedure Execute_SpecialEvents_Int
   Module Procedure Execute_SpecialEvents_Real
End Interface

!If you want to add special events that will be used by both the SSA (State Vector = Integers)
!and the HyJCMSS method (State Vector = Real*8), then copy the additional events to both subroutines!

CONTAINS
!--------------------------------------------------------------------------------------
Subroutine Init_SpecialEvents(t)

Real*8, intent(in) :: t

Real*8 :: tworands(2)
Logical :: TwoTrue(2)

!Add Cell Division event (Event time ~= Infinity if none exists)

If (CellGrowthTime <= 0) THEN
   CellGrowthTime = 1e20
end if

TimeLastCellDivision = t

!Add initial Cell replication time to dynamic sorted queue
TwoTrue = .TRUE.
Call Normal_Rand(tworands,TwoTrue)
Call Queue_Add(-1, t + CellGrowthTime + tworands(1)*CellGrowthTimeSD)

!Add external system perturbations to the Dynamic Sorted Queue.
!Offset mu values so external perturbations start at -1001 and go downwards

do i=1, NumPerturbations
   Call Queue_Add(-(PerturbNumOffset + i), PerturbationData(i,1))
end do

!For any other non-exponential events, generate the initial waiting times and add to the Dynamic Sorted Queue (SQ).
!As they arrive in the queue, the effect of each special event can be detailed
!in the Execute_SpecialEvents subroutine

End Subroutine
!------------------------------------------------------------------------------------
Subroutine Execute_SpecialEvents_Int(X, t, SpecialEventNumber)
IMPLICIT NONE

Real*8, intent(in) :: t
Integer, intent(in) :: SpecialEventNumber
Integer, intent(inout) :: X(:)
!More Optional arguments may be placed here, using Fortran's argument keywords

Logical :: TwoTrue(2)
Real*8 :: tworands(2)
Integer :: ID

TwoTrue = .TRUE.

Select Case (SpecialEventNumber)

Case (-1) !Cell Division event

   !Half the population of molecules
   Where (SplitOnDivision)
      X = int(X / 2) !Just half for now. Later: Select a binomially distributed random #.
   End Where

   !Reset the time until last cell division and add a cell replication event to the sorted queue

   !Remove cell replication event from sorted queue
   Call Queue_Delete(1)

   TimeLastCellDivision = t
   Call Normal_Rand(tworands,TwoTrue)
   Call Queue_Add(-1, t + CellGrowthTime + tworands(1)*CellGrowthTimeSD)

   !Blah. More special events here.
   !...

End Select

!System Perturbations execute here.

if (SpecialEventNumber < -PerturbNumOffset) THEN
	
   ID = -(SpecialEventNumber + PerturbNumOffset)

   if (PerturbationIDs(ID,1) == 0) THEN !This is a kinetic parameter perturbation
 	Rxndata(PerturbationIDs(ID,2))%c(PerturbationIDs(ID,3)) = PerturbationData(ID,2)

   elseif (PerturbationIDs(ID,1) == 1) THEN !This is a state vector perturbation
	X(PerturbationIDs(ID,2)) = PerturbationData(ID,2)

   end if

   !Can add other types of external system perturbations here. 

   !Remove external perturbation event from queue
   Call Queue_Delete(1)

  !Add reoccurring events here, if needed

end if

End Subroutine
!-------------------------------------------------------------------------------------
Subroutine Execute_SpecialEvents_Real(X, t, SpecialEventNumber)
IMPLICIT NONE

Real*8, intent(in) :: t
Integer, intent(in) :: SpecialEventNumber
Real*8, intent(inout) :: X(:)
!More Optional arguments may be placed here, using Fortran's argument keywords

Logical :: TwoTrue(2)
Real*8 :: tworands(2)
Integer :: ID

TwoTrue = .TRUE.

Select Case (SpecialEventNumber)

Case (-1) !Cell Division event

   !Half the population of molecules
   Where (SplitOnDivision)
      X = int(X / 2) !Just half for now. Later: Select a binomially distributed random #.
   End Where

   !Reset the time until last cell division and add a cell replication event to the sorted queue

   !Remove cell replication event from sorted queue
   Call Queue_Delete(1)

   TimeLastCellDivision = t
   Call Normal_Rand(tworands,TwoTrue)
   Call Queue_Add(-1, t + CellGrowthTime + tworands(1)*CellGrowthTimeSD)

   !Blah. More special events here.
   !...

End Select

!System Perturbations execute here.

if (SpecialEventNumber < -PerturbNumOffset) THEN
	
   ID = -(SpecialEventNumber + PerturbNumOffset)

   if (PerturbationIDs(ID,1) == 0) THEN !This is a kinetic parameter perturbation
 	Rxndata(PerturbationIDs(ID,2))%c(PerturbationIDs(ID,3)) = PerturbationData(ID,2)

   elseif (PerturbationIDs(ID,1) == 1) THEN !This is a state vector perturbation
	X(PerturbationIDs(ID,2)) = PerturbationData(ID,2)

   end if

   !Can add other types of external system perturbations here. 

   !Remove external perturbation event from queue
   Call Queue_Delete(1)

   !Add reoccurring events here, if needed

end if

End Subroutine
!-------------------------------------------------------------------------------------

End Module
