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

!-----------------------------
!****m shared/priorityqueue
!MODULE
!	Two different types of queues: An array heap implemented
!	priority queue and a dynamically allocated sorted queue.
!PURPOSE
!	Quickly finds the minimum of a list of values and the
!	corresponding position. The values are persistent so that
!	updates to the queues require only nlog(n) operations.
!AUTHOR
!	Howard Salis (2003)
!SOURCE
!***
!-----------------------------

module priorityqueue

!Indexed Priority Queue Implementation
!Howard Salis 3/19/03
!Uses Heap sorted (Min-top) Array, and two indexing arrays
!One index array points reaction # to tree position => reactions
!One index array points tree position to reaction # => indexer

!Minimum Reaction Time, tau(mu) => tau(1)
!Fastest Reaction Occurring     => reactions(1)
!Reaction Time of any Reaction  => tau(indexer(M))
!Reaction Time of any Position  => tau(P)
!Position of any Reaction       => indexer(M)


!Modified: 9/1/04
!Added Queue_Add, Queue_Delete subroutines to add/delete nodes to heap (deletion by position)
!Added Queue_MinTime, Queue_MinRxn: functions to return minimum event times and which event that is
!tau/reactions/indexer no longer accessible from other modules: Use above functions instead
!Added Queue_CancelEvent: Similar to Queue_Delete, but deletes a node by its reaction #. If the reaction # has more
!than one node, it will randomly select one to delete.

!Modified: 9/14/04
!Added another sorted queue: Now, there is one static-sized indexed priority queue and a dynamically sized sorted queue
!For each queue, there is a query. For the static, there is an init and update. For the dynamic, there is a add and delete node. 

!Everything defaults to private
private

!List of Public/Private subroutines/functions
public  :: Queue_Init, Queue_Update, Queue_Add, Queue_Delete, Queue_CancelEvent, Queue_Reset
public  :: Queue_MinTimeSQ, Queue_MinRxnSQ, Queue_MinTimeIPQ, Queue_MinRxnIPQ
public  :: Queue_SaveSnapshot, Queue_LoadSnapshot

private :: swap, update_aux, swapSQ, update_auxSQ !For reference

!Static IPQ - Indexed Priority Queue
!Queue_Init(<list of reaction times> (vector M length) )
!Queue_Update(<reaction #>,<new reaction time (scalar)>)


!Dynamic SQ - Sorted Queue
!Queue_Add(<reaction #>,<new reaction time (scalar)>)
!Queue_Delete(<position in heap array> (minimum value in array = 1)>
!Queue_CancelEvent(<reaction #>)

!Query functions for both
!mintau = Queue_MinTime => returns minimum time in mintau
!mu = Queue_MinRxn => returns reaction/event # in mu

Real*8, Dimension(:), Allocatable, Save, private :: tau, tauSQ
Integer, Dimension(:), Allocatable, Save, private :: reactions, indexer, reactionsSQ
Integer, Save, private :: maxnodes, SQnodes

Integer, Allocatable, SAVE, private :: reactions_save(:), indexer_save(:), reactionsSQ_save(:)
Real*8, Allocatable, SAVE, private :: tau_save(:), tauSQ_save(:)

!Functions & Subroutines Below
CONTAINS
!------------------------------------------------------------------------------------------
Subroutine Queue_SaveSnapshot
IMPLICIT NONE

if (Allocated(tau_save)) THEN
   Deallocate(tau_save)
end if

Allocate(tau_save(maxnodes))

if (Allocated(reactions_save)) THEN
   Deallocate(reactions_save)
end if

Allocate(reactions_save(maxnodes))

if (Allocated(indexer_save)) THEN
   Deallocate(indexer_save)
end if

Allocate(indexer_save(maxnodes))

if (Allocated(tauSQ_save)) THEN
   Deallocate(tauSQ_save)
end if

Allocate(tauSQ_save(SQnodes))


if (Allocated(reactionsSQ_save)) THEN
   Deallocate(reactionsSQ_save)
end if

Allocate(reactionsSQ_save(SQnodes))

tau_save = tau
reactions_save = reactions
indexer_save = indexer
tauSQ_save = tauSQ
reactionsSQ_save = reactionsSQ

End Subroutine Queue_SaveSnapshot
!-----------------------------------------------------------------------------------------
Subroutine Queue_LoadSnapshot
IMPLICIT NONE

tau = tau_save
reactions = reactions_save
indexer = indexer_save
tauSQ = tauSQ_save
reactionsSQ = reactionsSQ_save

End Subroutine Queue_LoadSnapshot
!-----------------------------------------------------------------------------------------
!Initialize Unsorted Heap Array
Subroutine Queue_Init(rtau)
IMPLICIT NONE

Real*8, Dimension(:), intent(in) :: rtau
Integer :: i

SQnodes = 1
maxnodes=size(rtau)

if (.NOT.Allocated(tau)) THEN
  Allocate(tau(maxnodes))
end if

if (.NOT.Allocated(reactions)) THEN
  Allocate(reactions(maxnodes))
end if

if (.NOT.Allocated(indexer)) THEN
  Allocate(indexer(maxnodes))
end if

if (.NOT.Allocated(tauSQ)) THEN
  Allocate(tauSQ(SQnodes))
end if

if (.NOT.Allocated(reactionsSQ)) THEN
  Allocate(reactionsSQ(SQnodes))
end if

tauSQ = 1e20
reactionsSQ = 0

!Setting Initial Heap Array to large numbers => Easier to Build sorted Heap Array 
tau=9.999e20

!Set 'root' node #1
tau(1)=rtau(1)
reactions(1)=1
indexer(1)=1

!For each additional entry, add node, update heap array to become sorted min-top
do i=2,maxnodes
	
	tau(i)=rtau(i)
	reactions(i)=i
	indexer(i)=i

	Call Update_Aux(i)
end do


End subroutine
!--------------------------------------------
Function Queue_MinTimeIPQ()

IMPLICIT NONE

Real*8 :: Queue_MinTimeIPQ

Queue_MinTimeIPQ = tau(1)

RETURN

End Function
!---------------------------------------------
Function Queue_MinRxnIPQ()

IMPLICIT NONE

Integer :: Queue_MinRxnIPQ

Queue_MinRxnIPQ = reactions(1)

RETURN

End Function
!--------------------------------------------
Function Queue_MinTimeSQ()

IMPLICIT NONE

Real*8 :: Queue_MinTimeSQ

if (SQnodes > 0) THEN
	Queue_MinTimeSQ = tauSQ(1)
else
	Queue_MinTimeSQ = real(1e20,8)
end if

RETURN

End Function
!--------------------------------------------
Function Queue_MinRxnSQ()

IMPLICIT NONE

Integer :: Queue_MinRxnSQ

if (SQnodes > 0) THEN
	Queue_MinRxnSQ = reactionsSQ(1)
else
	Queue_MinRxnSQ = 0
end if

RETURN

End Function
!--------------------------------------------
Subroutine Queue_Update(Rnum,rtau)

!Update Rnum'th reaction time (tau) to rtau and resort min-top
IMPLICIT NONE

Real*8, intent(in) :: rtau
integer, intent(in) :: Rnum
integer :: Position

tau(indexer(Rnum))=rtau

Position=indexer(Rnum)
Call Update_Aux(Position)

end subroutine
!---------------------------------------------
Subroutine Queue_Add(Rnum, rtau)
IMPLICIT NONE

!Add a reaction/event time to the heap array with value rtau and resort min-top

Real*8, intent(in) :: rtau
Integer, intent(in) :: Rnum
Real*8, Allocatable :: TempTau(:)
Integer, Allocatable :: TempReactions(:)
!Integer, Allocatable :: TempIndexer(:)

SQnodes=SQnodes + 1
Allocate(TempTau(SQnodes))
Allocate(TempReactions(SQnodes))

TempTau(1:SQnodes-1) = TauSQ
TempReactions(1:SQnodes-1) = ReactionsSQ

TempTau(SQnodes) = rtau
TempReactions(SQnodes) = Rnum

Deallocate(TauSQ)
Deallocate(ReactionsSQ)

Allocate(TauSQ(SQnodes))
Allocate(ReactionsSQ(SQnodes))

TauSQ = TempTau
ReactionsSQ = TempReactions

Deallocate(TempTau)
Deallocate(TempReactions)

Call Update_AuxSQ(SQnodes)

End Subroutine
!---------------------------------------------
Subroutine Queue_Delete(RPos)
IMPLICIT NONE

!Delete a reaction/event time from the heap array by Position

Integer, intent(in) :: RPos

Real*8, Allocatable :: TempTau(:)
Integer, Allocatable :: TempReactions(:), TempIndexer(:)


!To delete the RPos'th node, swap it with the last node and delete the last node.
!Then resort min-top

Call SwapSQ(RPos,SQnodes)

SQnodes = SQnodes - 1

Allocate(TempTau(SQnodes))
Allocate(TempReactions(SQnodes))
	
TempTau = TauSQ(1:SQnodes-1)
TempReactions = ReactionsSQ(1:SQnodes-1)

Deallocate(TauSQ)
Deallocate(ReactionsSQ)

Allocate(TauSQ(SQnodes))
Allocate(ReactionsSQ(SQnodes))

TauSQ = TempTau
ReactionsSQ = TempReactions

Call Update_AuxSQ(RPos)

End Subroutine
!---------------------------------------------
Subroutine Queue_CancelEvent(Rnum)
IMPLICIT NONE

!Given a reaction #, it randomly selects from all events corresponding to that reaction # and deletes the node
!This only applies to the events in the dynamically sized Sorted Queue

Integer, intent(in) :: Rnum
Integer :: NumEvents, RandomPos, i
Integer, Allocatable :: EventPositions(:), PositionIndex(:)
Real*8 :: rand1
Logical, Allocatable :: PositionMask(:)

Allocate(PositionIndex(SQnodes))
Allocate(PositionMask(SQnodes))

forall (i=1:SQnodes)
	PositionIndex(i) = i
end forall

PositionMask = (ReactionsSQ == Rnum)

NumEvents = count(PositionMask)

	if (NumEvents > 0) THEN
	   Allocate(EventPositions(NumEvents))
	   
	   EventPositions = pack(PositionIndex,PositionMask)
	   
	   Call Random_Number(rand1)
	   RandomPos = floor(rand1 * NumEvents) + 1
	   
	   Call Queue_Delete(EventPositions(RandomPos))
	   Deallocate(EventPositions)
	   Deallocate(PositionIndex)
	else
	   !Should never see this unless there's a bug!
	   print*, "Error! Attempt to delete a non-existing event. You'll need to debug this!"
	end if
	
End Subroutine
!---------------------------------------------
Subroutine Swap(low,high)
IMPLICIT NONE

!Swap the Low Value into the High Value's Spot and vica verca

integer, intent(in) :: low, high
integer :: tempreaction
Real*8 :: temptau

temptau=tau(high)
tempreaction=reactions(high)

tau(high)=tau(low)
reactions(high)=reactions(low)
indexer(reactions(high))=high

tau(low)=temptau
reactions(low)=tempreaction
indexer(reactions(low))=low

End Subroutine
!---------------------------------------------
Subroutine SwapSQ(low,high)
IMPLICIT NONE

!Swap the Low Value into the High Value's Spot and vica verca
!For dynamically sized Sorted Queue

integer, intent(in) :: low, high
integer :: tempreaction
Real*8 :: temptau

temptau=tauSQ(high)
tempreaction=reactionsSQ(high)

tauSQ(high)=tauSQ(low)
reactionsSQ(high)=reactionsSQ(low)

tauSQ(low)=temptau
reactionsSQ(low)=tempreaction

End Subroutine
!---------------------------------------------
Recursive Subroutine Update_Aux(N)
IMPLICIT NONE

!Sort heap array minimum-top
	
integer :: minpos
real*8 :: minchild
integer, intent(in) :: N
minpos=0

If ((N > 1).AND.(tau(max(1,N)) < tau(max(1,N/2)) ))  THEN
     Call Swap(N,N/2)
     Call Update_Aux(N/2)
else
   
   if (2*N + 1 <= maxnodes) THEN
      if (tau(2*N) > tau(2*N+1)) THEN
         minpos=1
         minchild=tau(2*N+1)
      else
         minpos=0
         minchild=tau(2*N)
      end if
   else
      if (2*N <= maxnodes) THEN
         minchild=tau(2*N)
         minpos=0
      else
         minchild=9e30
         minpos=0
      end if
   end if
   
   if (tau(N) > minchild) THEN
      Call Swap(2*N+minpos,N)
      Call Update_Aux(2*N+minpos)
   end if
end if
	   
End Subroutine
!------------------------------------------------
Recursive Subroutine Update_AuxSQ(N)
IMPLICIT NONE

!Sort heap array minimum-top
	
integer :: minpos
real*8 :: minchild
integer, intent(in) :: N
minpos=0
	

if (N > 1.AND.tauSQ(max(1,N)) < tauSQ( max(1,N/2) ) ) THEN
   Call SwapSQ(N,(N/2))
   Call Update_AuxSQ((N/2))
else
		
   if (2*N + 1 <= SQnodes) THEN
      if (tauSQ(2*N) > tauSQ(2*N+1)) THEN
         minpos=1
         minchild=tauSQ(2*N+1)
      else
         minpos=0
         minchild=tauSQ(2*N)
      end if
   else
      if (2*N <= SQnodes) THEN
         minchild=tauSQ(2*N)
         minpos=0
      else
         minchild=9e30
         minpos=0
      end if
   end if
   
   if (tauSQ(N) > minchild) THEN
      Call SwapSQ(2*N+minpos,N)
      Call Update_AuxSQ(2*N+minpos)
   end if
end if

	   
End Subroutine
!------------------------------------------------
Subroutine Queue_Reset(NewModel)
IMPLICIT NONE

Logical, intent(in) :: NewModel

!Deallocates priorityqueue storage
!If NewModel is false, only need to deallocate dynamic sorted queue

if (NewModel) THEN

  if (Allocated(tauSQ)) THEN
    Deallocate(TauSQ)
  end if

  if (Allocated(reactionsSQ)) THEN
    Deallocate(ReactionsSQ)
  end if
  
  if (Allocated(tau)) THEN
    Deallocate(tau)
  end if

  if (Allocated(indexer)) THEN
    Deallocate(indexer)
  end if

  if (Allocated(reactions)) THEN
    Deallocate(reactions)
  end if

  if (Allocated(tauSQ_save)) THEN
    Deallocate(TauSQ_save)
  end if

  if (Allocated(reactionsSQ_save)) THEN
    Deallocate(ReactionsSQ_save)
  end if
  
  if (Allocated(tau_save)) THEN
    Deallocate(tau_save)
  end if

  if (Allocated(indexer_save)) THEN
    Deallocate(indexer_save)
  end if

  if (Allocated(reactions_save)) THEN
    Deallocate(reactions_save)
  end if

else
  
  if (Allocated(TauSQ)) THEN
    Deallocate(TauSQ)
  end if
  
  if (Allocated(ReactionsSQ)) THEN
    Deallocate(ReactionsSQ)
  end if

  if (Allocated(TauSQ_save)) THEN
    Deallocate(TauSQ_save)
  end if
  
  if (Allocated(ReactionsSQ_save)) THEN
    Deallocate(ReactionsSQ_save)
  end if

end if

End Subroutine Queue_Reset
!-------------------------------------------------

End module priorityqueue
