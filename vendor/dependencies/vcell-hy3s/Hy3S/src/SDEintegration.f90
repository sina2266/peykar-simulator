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

!****m shared/SDEintegration
!MODULE
!	Solves the Chemical Langevin equation, Adaptive time step,
!	Brownian bridge
!PURPOSE
!	This modules is used to solve a system of Chemical Langevin eqns,
!	Ito stochastic differential equations with multiple multiplicative
!	noises. Euler-Maruyama and Milstein schemes are used to implement 
!	the adaptive sizestep and brownian bridges "rewind" the system if 
!	the sizestep is to big. Implicit forms of the above mentioned 
!	methods are invoked when the system is stiff. Solving the implicit 
!	schemes requires the use of Newton-Raphson method ().
!AUTHOR
!	Vassilios Sotiropoulos (2005) & Howard Salis
!SOURCE


!A List of Intel Pre-processing Declarations made in this source code.

!The following declared variables are set in the Makefile:

!If Adaptive is defined, the adaptive time step code will be used.
!If Adaptive is not defined, the fixed time step code will be used.
!If Milstein is defined, the Milstein method will be used to integrate the CLE.
!If Milstein is not defined, the Euler-Maruyama method will be used to integrate the CLE.

!The following declared variables are used in debugging and are set by the user.

!When Debug_Mem is defined, the memory requirements will be printed.
!When Debug_Time is defined, the CPU time requirements will be printed.

!!!DEC$ Define Adaptive
!!!DEC$ Define Milstein

!!!DEC$ Define Debug_Mem
!!!DEC$ Define Debug_Time

Module SDEintegration

USE Randomgen
USE GlobalVariables
USE RateLaws

private      !All global module variables are private (accessible only from within this module)  unless specified as public

public :: SDE_Init, SDE_Final
public :: SDE_CallOnClassifyFast

!List of private subroutines (for reference's sake)
public :: SDE_BrownianPath_Generate, SDE_BrownianPath_DeallocateRows, SDE_BrownianPath_DeallocateTree
public :: SDE_DriftDiffusion, SDE_Milstein_2DTerms, SDE_Explicit_Milstein, SDE_Explicit_EulerMaruyama
public :: SDE_Check_Criteria, SDE_Adaptive_Time_Stepping

!Definitions of variables used in a propogater that needs to be passed from one subroutine call to another
!By putting it in the module global (private) variable space, it does not need to be defined in the mainprogram

!Only make variables/subroutines public if they need to be accessed/called from outside this module

!Public Variables in the module

Integer, public :: PathSize, Max_Tree_Row, Tree_Row, Branch

Type, public :: BrownianPathType
   Real*8, Allocatable :: Row(:)     
End Type 

Type(BrownianPathType), Allocatable, public :: Brownian_Paths(:)

Real*8, Allocatable, private :: Ito2DSumTerms(:)
Integer, Allocatable, private :: Ito2DIndex(:,:)
Logical, Allocatable, private :: Ito2DIndexMask(:)

Real*8, private :: sqrtrop
Integer, Parameter, private :: p_2DIto = 10 
Integer, Parameter, private :: MinMolecules = 20

Real*8, Parameter, private :: pi = 3.14159265
Real*8, Parameter, private :: eps = 1e-9

!***** SUBROUTINES ****
CONTAINS
!--------------------------------------------------------------------------------------------------------------------
!****p SDEintegration/BrownianPath_Generate
!NAME
!      BrownianPath_Generate
!PURPOSE
!      Creates additional rows of the unit Brownian Paths (if not allocated) and calculates the Brownian increments
!      using the Levy areas
!SOURCE

Subroutine SDE_BrownianPath_Generate(Fast, Current_Row, Final_Row, Reset)
IMPLICIT NONE

Integer, intent(in) :: Current_Row, Final_Row
Logical, intent(in) :: Fast(:), Reset

Integer :: Mfast, Row_counter, i, k
Real*8 :: sum

Real*8, Allocatable :: rands(:), yrands(:)
Logical, Allocatable :: AllTrue(:), Numyrands(:)

Type (BrownianPathType), Allocatable :: Temp_Paths(:)

!Integer :: PathSize  <---Global private variable tracking the length of Brownian_Paths
!		      <---size(Brownian_Paths) is not correct!!!!

Mfast = count(Fast)

if (MFast == 0) THEN
   return
end if

!Allocate enough memory for up to Final_Row tree rows
!Regenerate top row if Brownian Tree did not previously exist
!If TreeRow < MaxTreeRow, then do not regenerate more rows of the Brownian Tree


#if defined(DEBUG_MEM)
    print*, "BrownianPath_Generate memory usage:"
    
    sum=0
    do i=1, max(Tree_Row, PathSize)
	sum = sum + Mfast*2**(i-1)
    end do

    print*, "Temp_Tree + Brownian_Path (bytes) ~= ", 2*sum*8

#endif

!Allocate all memory necessary for entire Brownian Tree
!Only allocate more memory if the current Brownian Tree needs to be expanded

if (PathSize < Final_Row) THEN

   Allocate(Temp_Paths(PathSize))
   Do i = 1, PathSize
      Allocate(Temp_Paths(i)%Row(Mfast*2**(i - 1)))
      Temp_Paths(i)%Row = Brownian_Paths(i)%Row
      
      Deallocate(Brownian_Paths(i)%Row)
      
   End do
   
   if (Allocated(Brownian_Paths)) THEN
      Deallocate(Brownian_Paths)
   end if
   
   !Allocate all necessary space for all rows
   Allocate(Brownian_Paths(Final_Row))
   
   Do i = 1 , Final_Row
      Allocate(Brownian_Paths(i)%Row(Mfast*2**(i - 1)))
   End do
   
   !Add existing data
   Do i = 1, Pathsize
      Brownian_Paths(i)%Row = Temp_Paths(i)%Row
      Deallocate(Temp_Paths(i)%Row)
   End do
   
   !Deallocate Temp Paths
   Deallocate(Temp_Paths)

   PathSize = Final_Row
end if

Row_counter = Current_Row

!If no Brownian Path exists yet, then generate the first row

if (Row_counter == 0) THEN
   
   !Mfast Gaussian random numbers with zero mean and unit variance
   Allocate(rands(Mfast))
   Allocate(AllTrue(Mfast))
   AllTrue = .TRUE.

   Call Normal_Rand(rands, AllTrue)
   !Brownian_Paths(1)%Row = rands * sqrt(deltat)
   Brownian_Paths(1)%Row = rands

   Deallocate(rands, AllTrue)

   Row_counter = 1
   Max_Tree_Row = 1
end if

!If Reset is false, then only regenerate lower rows of the Brownian Tree if they hadn't been generated before
!If Reset is true, then regenerate all tree rows up to Final_Row

if (Row_counter < Final_Row.AND.( (Final_Row > Max_Tree_Row).OR.Reset) ) THEN

  do i=Row_counter+1, Final_Row

   !Compute Brownian increments and place in already allocated Brownian Paths
      
   Allocate(Numyrands(Mfast*2**(i - 2)), yrands(Mfast*2**(i - 2)))
      
   Numyrands = .True.
      
   Call Normal_Rand(yrands, Numyrands)
   !yrands = yrands / real(2**(i - 1),8)
   !yrands = yrands
   !yrands = yrands / sqrt(real(2**(i - 1),8))
    yrands = yrands / sqrt(real(2**(i),8))

   !Generate dW 
      
   forall (k = 1:2**(i - 2) )

   Brownian_Paths(i)%Row(1 + 2*(k - 1)*Mfast : (2*k - 1)*Mfast) = & 
        & Real(0.50, 8)*Brownian_Paths(i - 1)%Row(1 + (k - 1)*Mfast : k*Mfast) &
        & + yrands(1 + (k - 1)*Mfast : k*Mfast)
   
   Brownian_Paths(i)%Row(1 + (2*k - 1)*Mfast : 2*k*Mfast) = & 
        & Real(0.50, 8)*Brownian_Paths(i - 1)%Row(1 + (k - 1)*Mfast : k*Mfast) &
        & - yrands(1 + (k - 1)*Mfast : k*Mfast)
   
   end forall

   Deallocate (Numyrands, yrands)

  end do

  Max_Tree_Row = Final_Row

end if

#if defined(DEBUG_MEM)

   print*, "Generated PathSize = ", PathSize

#endif

End Subroutine
!-------------------------------------------------------------------------------------------
!****p SDEintegration/BrownianPath_DeallocateTree
!NAME
!       SDE_BrownianPath_DeallocateTree
!PURPOSE
!       This subroutine is used to deallocate Brownian_Paths
!SOURCE

Subroutine SDE_BrownianPath_DeallocateTree()
IMPLICIT NONE
!***

Integer :: iallo

!Integer :: PathSize <------ Module global private variable that tracks the length of Brownian_Paths

If (Allocated(Brownian_Paths)) then
   Do iallo = 1, PathSize
      Deallocate(Brownian_Paths(iallo)%Row)
   End do 
   Deallocate(Brownian_Paths)
End if 

PathSize = 0 !<--- Module private global variable tracking length of Brownian_Paths
Max_Tree_Row = 0

End Subroutine
!--------------------------------------------------------------------------------------------
Subroutine SDE_BrownianPath_DeallocateRows(StartRow, EndRow)
IMPLICIT NONE

Integer, intent(in) :: StartRow, EndRow

Integer :: i

do i=StartRow, PathSize
   Deallocate(Brownian_Paths(i)%Row)
end do

PathSize = StartRow - 1
Max_Tree_Row = StartRow - 1

End Subroutine
!---------------------------------------------------------------------------------------------
Subroutine SDE_Init(M, N, NewModel)
IMPLICIT NONE

Logical, intent(in) :: NewModel
Integer, intent(in) :: M, N
Integer :: i

!Called at beginning of program 

!!Real*8 :: sqrtrop !<---- Module private variable!

!PathSize is the allocated number of rows of Brownian_Paths
!size(Brownian_Paths) is not always correct!!!

PathSize = 0 !Initially set PathSize to Zero <--- module private global variable

if (NewModel) THEN

#if defined(MILSTEIN)

   !Calculate sqrt(rop) for future usage
   do i=1, p_2DIto
      sqrtrop = sqrtrop + Real(1.0, 8) / Real(i*i, 8)
   end do

   sqrtrop = sqrt( Real(1.0, 8) / Real(12.0, 8) - Real(1.0, 8) / ((Real(2.0, 8)*pi*pi)*sqrtrop) )

   Allocate(Ito2DSumTerms(N))

#endif

end if

End Subroutine
!--------------------------------------------------------------------------------------------
Subroutine SDE_Final(NewModel)
IMPLICIT NONE

Logical, intent(in) :: NewModel

!Called at end of program -- in Final_HyHomSS
!Deallocates Allocatable Module private variables

If (NewModel) THEN

#if defined(MILSTEIN)

if (Allocated(Ito2DIndex)) THEN
   Deallocate(Ito2DIndex)
end if

if (Allocated(Ito2DIndexMask)) THEN
   Deallocate(Ito2DIndexMask)
end if

if (Allocated(Ito2DSumTerms)) THEN
   Deallocate(Ito2DSumTerms)
end if

#endif

End If

Call SDE_BrownianPath_DeallocateTree

End Subroutine
!--------------------------------------------------------------------------------------------
Subroutine SDE_CallOnClassifyFast(M, FastIndex, FastInverseIndex, BeforeFastIndex, BeforeFastInverseIndex, BeforeFast, AfterFast)
IMPLICIT NONE

!This subroutine is called when a reaction classification change may have occurred.

!If the Milstein method is used, the Ito2DIndex and Ito2DIndexMask are re-created

!If a reaction has switched its classification, the Brownian Tree is altered.
!New fast reactions have their dW increments generated.
!Previous fast reactions inherit their old dW increments.

Integer, intent(in) :: M, BeforeFastIndex(:), BeforeFastInverseIndex(:), FastIndex(:), FastInverseIndex(:)
Logical, intent(in) :: BeforeFast(:), AfterFast(:)

Integer :: Mfast, BeforeMFast, i, j, k, j1, j2, counter
Real*8 :: sum

Type (BrownianPathType), Allocatable :: Temp_Tree(:)
Real*8, Allocatable :: yrands(:)
Logical, Allocatable :: RandMask(:)
Integer, Allocatable :: TempIto2DIndex(:,:)

!!!! if (any(BeforeFast.XOR.AfterFast)) THEN
if (any(BeforeFast.NEQV.AfterFast)) THEN


#if defined(MILSTEIN)

if (Allocated(Ito2DIndex)) THEN
   Deallocate(Ito2DIndex)
end if

if (Allocated(Ito2DIndexMask)) THEN
   Deallocate(Ito2DIndexMask)
end if

counter = 0
Allocate(Ito2DIndex(counter,6))

do j2=1, M
   if (AfterFast(j2)) THEN
	do i=1, Rxndata(j2)%SListLen
	    do j1 = 1, M
		if (AfterFast(j1)) THEN
		   
		   do k=1, Rxndata(j2)%DListLen

			if (intersect(Rxndata(j2)%DList(k), Rxndata(j1)%SList(1:Rxndata(j1)%SListLen))) THEN
	                  Allocate(TempIto2DIndex(counter,6))
	                  TempIto2DIndex=Ito2DIndex
	                  Deallocate(Ito2DIndex)
                  
	                  counter = counter + 1
        	          Allocate(Ito2DIndex(counter,6))
                	  Ito2DIndex(1:counter-1,:) = TempIto2DIndex
	                  Ito2dIndex(counter,:) = (/j1, j2, Rxndata(j2)%v(i), Rxndata(j1)%v(k), Rxndata(j2)%SList(i), Rxndata(j1)%SList(k)/)

	                  Deallocate(TempIto2DIndex)                 
		        
			end if

		   end do
		end if
	   end do
       end do
  end if
end do

Allocate(Ito2DIndexMask(counter))
Ito2DIndexMask = .TRUE.

#endif


#if defined(DEBUG_MEM)
    print*, "Memory usage of SDE_CallOnClassifyFast: "
    print*, "Ito2DIndex (bytes) = ", size(Ito2DIndex,1)*size(Ito2DIndex,2)*4
    print*, "Ito2DIndexMask (bytes) = ", size(Ito2DIndexMask)
    print*, "Ito2DSumTerms (bytes) = ", N*8
#endif


   !For both Adaptive & Fixed code, Brownian Paths are always used & must be converted on reaction reclassifications.

   MFast = count(AfterFast)
   BeforeMFast = count(BeforeFast)
  
   !Convert the old Brownian Tree with the old Fast reactions to a Tree with the new Fast reactions

   !If all of the reactions were slow, then treat like the normal Brownian Tree generation
   !Otherwise, do the special generation method (which is probably slower).

   if (all(.NOT.BeforeFast)) THEN !Regenerate entire tree from scratch

	!All Slow Reactions. Use faster method.
	Tree_Row = 1
	Branch = 1

	Call SDE_BrownianPath_DeallocateTree
	Call SDE_BrownianPath_Generate(AfterFast, 0, Tree_Row, Reset = .TRUE.)
	
   else !Only regenerate newly Fast reactions


      !Allocate all necessary space for Temp_Tree
  
      Allocate(Temp_Tree(Max_Tree_Row))
      do i=1, Max_Tree_Row
   	   Allocate(Temp_Tree(i)%Row(Mfast*2**(i - 1)))

	   !Can remove when bugs are fixed.
	   !Temp_Tree(i)%Row = 0

      end do
     
      !Move dW increments of fast reactions to the temp tree
      !If a fast reactions hasn't changed, just transfer over the previous values
      !If a fast reaction is new, generate its dW increments down to Max_Tree_Row

      do j=1, M

	!These are old fast reactions.

        if (BeforeFast(j).AND.AfterFast(j)) THEN

	    do i=1, Max_Tree_Row

	      Temp_Tree(i)%Row(FastIndex(j):MFast*2**(i - 1):MFast) = &
	           Brownian_Paths(i)%Row(BeforeFastIndex(j):BeforeMFast*2**(i - 1):BeforeMFast)

	    end do

	end if


	!These are new fast reactions	

	if (.NOT.BeforeFast(j).AND.AfterFast(j)) THEN

	    !At the top of the tree, generate dW as N(0,1).

	    !Generate 2 N(0,1), but use only one.

	    Allocate(yrands(2), RandMask(2))
	    RandMask = .TRUE.

	    Call Normal_Rand(yrands, RandMask)
	    Temp_Tree(1)%Row(FastIndex(j)) = yrands(1)
	    !Temp_Tree(1)%Row(FastIndex(j)) = yrands(1) * sqrt(real(2.0,8))
	 
	    Deallocate(yrands, RandMask)

   	    do i=2, Max_Tree_Row

	        !If below the top, then use the Levy areas to compute dW increments
		
		Allocate(yrands(max(2**(i - 2),2)), RandMask(max(2**(i - 2),2)) )
		RandMask = .TRUE.

		Call Normal_Rand(yrands, RandMask)
		!yrands = yrands / real(2**((i - 1)),8)
		!yrands = yrands
	        !yrands = yrands / sqrt(real(2**(i - 1),8))
	         yrands = yrands / sqrt(real(2**(i),8))

		do k = 1, 2**(i - 2) 

		   Temp_Tree(i)%Row(FastIndex(j) + 2*(k - 1)*MFast) = &
	           & Real(0.50, 8)*Temp_Tree(i - 1)%Row(FastIndex(j) + (k - 1)*Mfast) &
	           & + yrands(k)

		   Temp_Tree(i)%Row(FastIndex(j) + (2*k - 1)*Mfast) = &
		   & Real(0.50, 8)*Temp_Tree(i - 1)%Row(FastIndex(j) + (k - 1)*Mfast) &
		   & - yrands(k)

		end do
	
		Deallocate(yrands, RandMask)

            end do

	end if

      end do

   do i = 1, PathSize
      Deallocate(Brownian_Paths(i)%Row)
   end do 
    
   Deallocate(Brownian_Paths)

   Allocate(Brownian_Paths(Max_Tree_Row))
   do i=1, Max_Tree_Row
	Allocate(Brownian_Paths(i)%Row(Mfast*2**(i - 1)))
	Brownian_Paths(i)%Row = Temp_Tree(i)%Row
	Deallocate(Temp_Tree(i)%Row)
   end do
   
   Deallocate(Temp_Tree)

   PathSize = Max_Tree_Row !Redundant?

  end if !Endif from Regenerate entire tree or only regenerate newly fast reactions
  
end if

End Subroutine
!---------------------------------------------------------------------------------------------
Subroutine SDE_DriftDiffusion(Fast, Fast_Index, a, deltat, dW, RxnEM)

Real*8, intent(in) :: deltat, a(:), dW(:)
Integer, intent(in) :: Fast_Index(:)
Logical, intent(in) :: Fast(:)

Real*8, intent(out) :: RxnEM(:)

forall (i=1:M, Fast(i))
   !RxnEM(i) = a(i) * deltat + sqrt(a(i)*deltat) * dW(Fast_Index(i))
   RxnEM(i) = a(i) * deltat + sqrt(a(i)) * dW(Fast_Index(i))
end forall

End Subroutine
!----------------------------------------------------------------------------------------
Subroutine SDE_Milstein_2DTerms(Fast, FastSpecies, Fast_Index, a, der_a, deltat, dW)
IMPLICIT NONE

Integer :: ii

Integer, intent(in) :: Fast_Index(:)
Real*8, intent(in) :: a(:), der_a(:,:), dW(:), deltat
Logical, intent(in) :: Fast(:), FastSpecies(:)

Integer, Allocatable :: j1(:), j2(:), i(:), k(:)
Integer, Allocatable :: j1_M(:), j2_M(:), vi(:), vk(:)
Logical, Allocatable :: Ito2DIntDone(:,:)

Integer :: s, co, Mfast, NumItoTerms
Real*8 :: termsum

Real*8, Allocatable :: xi(:), miu(:), niu(:,:), zeta(:,:)
Real*8, Allocatable :: rands(:), Ito2DInt(:,:), Ito2DCoeff(:), Ito2DIntArray(:), Ito2DTerms(:)

Logical, Allocatable :: RandMask(:)

!These three variables are in the module private space!!
!Integer, Allocatable :: Ito2DIndex(:,:), Ito2DIntList(:,:)
!Logical, Allocatable :: Ito2DIndexMask(:), Ito2DTermSum(:)

!Step through Ito2DIndex terms
!Calculate 2D Ito Integrals and place in 2D array (j1,j2) with a 2D (j1,j2) Logical array set to true for each one (no double computation)

Mfast = count(Fast)
NumItoTerms = size(Ito2DIndex,1)


#if defined(DEBUG_MEM)
    print*, "Milstein_2DTerms:"

    print*, "Random numbers (including initial storage) (bytes) = ", Mfast*(3 + p_2DIto*4)*8
    print*, "Integrals & Coefficients (bytes) = ", (Mfast*Mfast + 2*NumItoTerms)*8
    print*, "Indexes (bytes) = ", 8*NumItoTerms*4
    print*, "Logicals (bytes) = ", Mfast*Mfast + Mfast*(1 + 2*p_2DIto)

#endif

Allocate(j1(NumItoTerms), j2(NumItoTerms), j1_M(NumItoTerms), j2_M(NumItoTerms))
Allocate(i(NumItoTerms), k(NumItoTerms), vi(NumItoTerms), vk(NumItoTerms))
Allocate(Ito2DIntDone(Mfast, Mfast))

Allocate(xi(Mfast), zeta(p_2DIto, Mfast), miu(Mfast), niu(p_2DIto,Mfast))
Allocate(rands(2*p_2DIto*Mfast + Mfast), RandMask(2*p_2DIto*Mfast + Mfast))

Allocate(Ito2DInt(Mfast,Mfast))
Allocate(Ito2DIntArray(NumItoTerms), Ito2DCoeff(NumItoTerms), Ito2DTerms(NumItoTerms))

!Compute are random numbers needed to approximate the Levy area   
!xi, zeta, niu, miu are all Gaussian Random Numbers N(0,1)

!**** Check this!
xi = dW / sqrt(deltat)
!xi = dW

RandMask = .True.
Call Normal_Rand(rands, RandMask)

miu = rands(1 : Mfast) 

forall (ii=1:p_2DIto) 
   niu(ii,:) = rands(1 + ii*Mfast : (ii + 1)*Mfast) 
   zeta(ii,:) = rands(1 + (ii + p_2DIto)*Mfast : (ii + p_2DIto + 1)*Mfast)
End forall

Deallocate(rands)
Deallocate(RandMask)

j1 = Fast_Index( (/Ito2DIndex(:,1)/) )
j2 = Fast_Index( (/Ito2DIndex(:,2)/) )

j1_M = Ito2DIndex(:,1)
j2_M = Ito2DIndex(:,2)

vi = Ito2DIndex(:,3)
vk = Ito2DIndex(:,4)
i = Ito2DIndex(:,5)
k = Ito2DIndex(:,6)

Ito2DIntDone = .FALSE.

do s=1,NumItoTerms
   if (Ito2DIndexMask(s)) THEN

      !j1 = j2, Ito 2D Integral simplifies, if not then cross-terms must be computed

      if (j1(s) == j2(s).AND..NOT.Ito2DIntDone(j1(s),j2(s))) THEN
         
        !Ito2DInt(j1(s),j2(s)) = real(0.50000,8) * (dW(j1(s))*dW(j2(s)) - deltat)
        Ito2DInt(j1(s),j2(s)) = real(0.50000,8) * (deltat*xi(j1(s))*xi(j2(s)) - deltat)
  
        Ito2DIntDone(j1(s),j2(s)) = .TRUE.               

      elseif (.NOT.Ito2DIntDone(j1(s),j2(s))) THEN
      
         !Calculate the sum that is dependent upon p

         termsum = real(0.000,8)
         do co = 1, p_2DIto
            termsum = termsum + (  zeta(co,j1(s))*( sqrt(Real(2.0, 8))*xi(j2(s)) - niu(co,j2(s)) ) - & 
	    & zeta(co,j2(s))*( sqrt(Real(2.0, 8))*xi(j1(s)) - niu(co,j1(s)) )  )/Real(co, 8)
         End do

         Ito2DInt(j1(s),j2(s)) = deltat*(  Real(0.50, 8)*xi(j1(s))*xi(j2(s)) + & 
	 &  sqrtrop*( miu(j1(s))*xi(j2(s)) - miu(j2(s))*xi(j1(s)) ) + termsum / Real(2.00, 8) / pi  )

         !I(j1,j2) = I(j2,j1), right? Right?!?!

         Ito2DInt(j2(s),j1(s)) = Ito2DInt(j1(s), j2(s))
         
         Ito2DIntDone(j1(s),j2(s)) = .TRUE.
         Ito2DIntDone(j2(s),j1(s)) = .TRUE.     

      end if

      if (j1(s) == j2(s)) THEN
         Ito2DCoeff(s) = real(0.5000,8) * vk(s) * vi(s) * der_a(j2_M(s),k(s))

      else          
         Ito2DCoeff(s) = real(0.5000,8) * vk(s) * vi(s) * sqrt(a(j1_M(s))) * der_a(j2_M(s),k(s)) / sqrt(a(j2_M(s)))

      end if

      Ito2DIntArray(s) = Ito2DInt(j1(s),j2(s))
   end if

end do
 
Deallocate(xi, miu, niu, zeta)

Deallocate(j1, j2, j1_M, j2_M)
Deallocate(i, k, vi, vk)
Deallocate(Ito2DIntDone)

!print*, "Ito2DInt = ", Ito2DInt
!print*, "Ito2DIntArray = ", Ito2DIntArray
!print*, "Ito2DCoeff = ", Ito2DCoeff

Ito2DTerms = Ito2DCoeff * Ito2DIntArray

Ito2DSumTerms = real(0.00,8)

forall (ii=1:N, FastSpecies(ii))
     Ito2DSumTerms(ii) = sum(Ito2DTerms, mask = Ito2DIndex(:,5) == ii.AND.Ito2DIndexMask)
end forall

Deallocate(Ito2DCoeff, Ito2DIntArray, Ito2DInt, Ito2DTerms)

End Subroutine
!---------------------------------------------------------------------------------------------------------------------
Subroutine SDE_Explicit_EulerMaruyama(Fast, M, RxnEM, dXfast)
IMPLICIT NONE

Logical, intent(in) :: Fast(:)
Integer, intent(in) :: M
Real*8, intent(in) :: RxnEM(:)
Real*8, intent(out) :: dXfast(:)

Integer :: i

dXfast = real(0.0,8)

do i=1, M
   if (Fast(i)) THEN
	dXfast( (/Rxndata(i)%SList(1:Rxndata(i)%SListLen) /) ) = dXfast( (/Rxndata(i)%SList(1:Rxndata(i)%SListLen) /) ) &
           & + Rxndata(i)%v(1:Rxndata(i)%SListLen) * RxnEM(i)
   end if
end do

End Subroutine
!---------------------------------------------------------------------------------------------------------------------
!****p SDEintegration/Explicit_Milstein
!NAME
!	Explicit_Milstein
!PURPOSE
!       The actual solver of the SDE.
!SOURCE

Subroutine SDE_Explicit_Milstein(Fast, FastSpecies, N, RxnEM, dXfast)
IMPLICIT NONE

!***

Logical, intent(in) :: Fast(:), FastSpecies(:)
Integer, intent(in) :: N

Real*8, intent(in) :: RxnEM(:)
Real*8, intent(out) :: dXfast(:)

Integer :: i

dXfast = real(0.00,8)

do i=1, M
   if (Fast(i)) THEN
      dXfast( (/Rxndata(i)%SList(1:Rxndata(i)%SListLen) /) ) = dXfast( (/Rxndata(i)%SList(1:Rxndata(i)%SListLen) /) ) & 
           & + Rxndata(i)%v(1:Rxndata(i)%SListLen) * RxnEM(i)
   end if
end do

forall (i=1:N, FastSpecies(i))
   dXfast(i) = dXfast(i) + Ito2DSumTerms(i)
end forall

End Subroutine
!-------------------------------------------------------------------------------------------------------------
Subroutine SDE_Check_Criteria(Fast, FastSpecies, X, deltat, dW, a, der_a, Vdim, Tol, ErrorOk)
IMPLICIT NONE

Real*8, intent(in) :: Tol, X(:), dW(:), deltat, a(:), der_a(:,:), Vdim
Logical, intent(in) :: Fast(:), FastSpecies(:)

Logical, intent(out) :: ErrorOk

Real*8, Allocatable :: g_prime_term(:,:), g_term(:)
Real*8, Allocatable :: dW_cubed(:)
Real*8 :: Diff_term1, Diff_term2
Real*8, Allocatable ::  g_term_fast(:), g_prime_term_fast(:,:), Transpose_g_prime_term_fast(:,:) 

Real*8, Allocatable :: f_old(:,:), f_new(:,:),temp_X(:), temp_der_a(:,:)
Real*8, Allocatable :: Drift_Error(:,:)

Real*8 :: Diff_Error
Real*8 :: maxvij, sumf
Integer :: i, j, k, Mfast 
Logical, Allocatable :: AllTrue(:)

Mfast = count(Fast)

Allocate(g_prime_term(M,N), g_term(M))
Allocate(g_term_fast(Mfast), g_prime_term_fast(Mfast,N), Transpose_g_prime_term_fast(N,Mfast))
Allocate(dW_cubed(Mfast), AllTrue(M))

Allocate(f_old(M,N), f_new(M,N), temp_X(N), temp_der_a(M,N))
Allocate(Drift_Error(M,N))

!****** Diffusion Criteria *****

!compute g terms
g_term = 0; g_prime_term = 0

Do i = 1, M
   Do j = 1, Rxndata(i)%SListLen
      g_prime_term(i, Rxndata(i)%SList(j)) = Rxndata(i)%v(j) * der_a(i, Rxndata(i)%SList(j) ) & 
      & / ( real(2.0,8) * sqrt(a(i)) )
   End do

   maxvij = maxval(abs(Rxndata(i)%v(1:Rxndata(i)%SListLen)), dim = 1)
   g_term(i) = maxvij * sqrt(a(i))
   maxvij = 0
End do

!compute transpose where needed

k = 1
Do i = 1,M
   If (Fast(i)) then
      dW_cubed(k) = dW(k)**Real(3.0, 8) 
      g_term_fast(k) = g_term(i)
      g_prime_term_fast(k,1:N) = g_prime_term(i,1:N)
      k = k + 1
   End if
End do

Transpose_g_prime_term_fast = transpose(g_prime_term_fast)

!Compute terms in the criterion

Diff_term1 = maxval(abs(Matmul(Transpose_g_prime_term_fast, dW_cubed)))

Diff_term2 = maxval(abs(Matmul(Transpose_g_prime_term_fast, g_term_fast )))

! Compute Diffusion Error

Diff_Error = Diff_term1 * Diff_term2 / Real(6.0, 8)


! ***** Drift Criteria *****

!compute g terms

f_old = 0

Do i = 1, M
   Do j = 1, Rxndata(i)%SListLen
      f_old(i, Rxndata(i)%SList(j)) = Rxndata(i)%v(j) * der_a(i, Rxndata(i)%SList(j) )
   End do
End do


Do i = 1, N
sumf = 0
   Do j = 1, M
      sumf = sumf + f_old(j,i)
   End do
   temp_X(i) = X(i) + deltat * sumf
End do

AllTrue = .True.
temp_der_a = 0

Call Derivatives_Propensities(AllTrue, temp_X, temp_der_a, Vdim)

f_new = 0

Do i = 1, M
   Do j = 1, Rxndata(i)%SListLen
      f_new(i, Rxndata(i)%SList(j)) = Rxndata(i)%v(j) * temp_der_a(i, Rxndata(i)%SList(j) )
   End do
End do

! Compute Error

Drift_Error = (f_new - f_old) * deltat / Real(2.0, 8) 


!Tolerance Check

if (Diff_Error >= Tol.OR.maxval(sum(abs(Drift_Error),dim = 2)) >= Tol) Then
   ErrorOk = .FALSE.
else
   ErrorOk = .TRUE.
end if

#if defined(DEBUG_STATE)

         print*, "****** Convergence Criteria ********"
         print*, "Max Diffusion Error = ", Diff_Error
         print*, "Max Drift Error = ", maxval(maxval(abs(Drift_Error), dim = 1), dim = 1)
         !print*, "Max_N a_der = ", maxval(der_a, dim = 2)
         !print*, "Max_N a_der_new = ", maxval(temp_der_a, dim = 2)
         !print*, "Current X = ", X
	 !print*, "New X = ", temp_X

         print*, "ErrorOK = ", ErrorOk
         print*, "****** End Criteria ********"

#endif

Deallocate(g_prime_term, g_term)
Deallocate(g_term_fast, g_prime_term_fast, Transpose_g_prime_term_fast, dW_cubed)
Deallocate(f_old, f_new, AllTrue)
Deallocate(temp_X, temp_der_a)
Deallocate(Drift_Error)

End Subroutine
!--------------------------------------------------------------------------------------------------------------
!****p SDEintegration/Adaptive_Time_Stepping
!NAME
!	Adaptive_Time_Stepping
!PURPOSE
!       This subroutines implements an adaptive sizestep scheme based on the work of Gaines and Lyons (1997).
!       The integration time step is kept the same halfed or doubled, depending on the tolerance and the 
!       criteria set.  
!SOURCE

Subroutine SDE_Adaptive_Time_Stepping(ErrorOk, Tree_Row, Branch)
IMPLICIT NONE
!***

Integer, intent(inout) :: Tree_Row, Branch
Logical, intent(in) :: ErrorOk

!If criteria are met then the step is doubled or kept the same, depending on the constraints

!If (Tree_Row < 7) THEN
!   Tree_Row = Tree_Row + 1
!   Branch = Branch*2 - 1
!else
!   Branch = Branch + 1
!end if

If (ErrorOk.AND.Mod(Branch,2).EQ.0.AND.Tree_Row > 1) then

   !Double Dt
   Tree_Row = Tree_Row - 1
   Branch = Branch / 2 + 1

Else if (ErrorOk.AND.Mod(Branch,2).EQ.1) then

   !Keep same dt Adapt_Dt = Adapt_Dt  Tree_Row = Tree_Row
   Branch = Branch + 1

Else if (.NOT.ErrorOk) then
  
   !Halved dt
   Tree_Row = Tree_Row + 1
   Branch = 2*Branch - 1

End if

End Subroutine SDE_Adaptive_Time_Stepping
!--------------------------------------------------------------------------------------------------------------
pure Function intersect(Num,array2)

  IMPLICIT NONE

  Integer, intent(in) :: Num
  Integer, intent(in) :: array2(:)

  integer :: counter, size2

  Logical :: intersect

  intersect=.FALSE.
  counter=0

  size2 = size(array2)

  do while ((.NOT.intersect).AND.(counter < size2 ) )

     counter = counter + 1
     if (Num == array2(counter)) THEN
        intersect=.TRUE.
     end if
  end do

End Function intersect
!--------------------------------------------------------------------------------------------------------------
End Module
