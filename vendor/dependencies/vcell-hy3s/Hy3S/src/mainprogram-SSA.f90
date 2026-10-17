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

!-----------------------------------
!****e Hy3S_main/SSA
!PROGRAM
!	The main program for the stochastic simulation algorithm.
!PURPOSE
!	Simulates a general system of chemical or biochemical reactions
!	using the Next Reaction variant of the stochastic simulation algorithm (Gillespie, 1976).
!AUTHOR
!	Howard Salis (2004)
!***
!-----------------------------------

program Mainprogram

!Main program for Stochastic Simulation Algorithm

!List of utilized modules - contains global variables used by this program 

Use Propagators_SSA
Use DataIO
Use GlobalVariables
Use NetCDF

!Using MPI? yes = 1, no otherwise
!!DEC$ DEFINE USING_MPI

#if defined(USING_MPI)
   USE MPI
#endif

IMPLICIT NONE

Integer i, j, k, me, XStateID, TimeID, fileunit, ierror, NumProcs, plotcounter

!State variables

Real*8 :: t
Real*8, Allocatable :: Xnew(:), X(:)

!Data Saving vectors
Real*8, Allocatable :: Xstate(:,:), Time(:)

!Temp. Kinetic Constant Array
Real*8, Allocatable :: TempC(:,:)

!Command line argument arrays
Integer :: NumCLParameters
Real*8 :: CLParameters(6)

!These variables are used to compute the wall times for the program
Real*4 :: t1, t2, t3, t4, t5, t6, t7

!Variables for computing the statistics of the program
Integer, Allocatable :: Reactioncountout(:)
Integer :: Itercounter, Multicounter
Real*8 :: AvgTime, AvgNumIter, AvgNumSlowRxns, DiskTime, AvgSlowIter
Real*8 :: AvgModelTime

#if defined(USING_MPI)

Real*8 :: SumAvgTime, SumAvgNumIter, SumAvgNumSlowRxns, SumDiskTime, SumAvgSlowIter
Real*8 :: SumAvgModelTime

  Call MPI_Init(Ierror)
  Call MPI_Comm_Size(MPI_Comm_World, NumProcs, ierror)
  Call MPI_Comm_Rank(MPI_Comm_World, me, ierror)
  
#else
  NumProcs = 1
  me = 0
#endif

Call cpu_time(t1)

!Parse command line arguments and read in simulation parameters
Call ParseCLA(NumCLParameters, CLParameters)

!Get simulation parameters from command line arguments
!If there are none, set to default values

Select case (NumCLParameters)

   Case (-1)

   if (me == 0) THEN
      print*, "You are using Hy3S: Hybrid Stochastic Simulation for Supercomputers"
      print*, "Visit our website at http://hysss.sourceforge.net"
      print*, ""
      print*, "Algorithm: Next Reaction variant of the stochastic simulation algorithm"
      print*, "System: Homogeneous bio/chemical systems"
      print*, ""
      print*, "Syntax: "
      print*, "ProgramName <NetCDF Filename> [-R <Random Seed>] [-OV]"
      print*, "Definitions:"
      print*, "NetCDF Filename: The name of the NetCDF file containing the model data. The program will place the solution data in this file."
      print*, ""
      print*, "Random Seed: The initial seed for the random number generator (a very long integer). (Default: Randomly generated, dependent on &
           & compiler.)"
      print*, ""
      print*, "-OV : Include to overwrite previously written solution data."
      print*, ""
   end if
 
   stop

End Select

!Reads into data from NetCDF file and defines many model-related global variables -- contains some MPI code
Call InputModelDataInit

If (me == 0) THEN

   print*, "Hy3S: Hybrid Stochastic Simulation for Supercomputers"
   print*, "HTTP://hysss.sourceforge.net"
   print*, ""
   print*, "Algorithm: Next Reaction variant of the Stochastic Simulation Algorithm (SSA)"
   print*, "System: Homogeneous Bio/Chemical Reaction systems"
   print*, ""

   if (OverWriteSolution == 1) THEN
	print*, "**Warning** Program is overwriting any previously written solution data."
	print*, ""
   end if

   Call PrintModelData

End if

!Initialize statistics
AvgNumIter = 0
AvgTime = 0
AvgNumSlowRxns = 0
AvgSlowIter = 0
DiskTime = 0
AvgModelTime = 0

Allocate(Time(Timepoints))

do j=LastModel+1,NumModels

   Call cpu_time(t6)

   Call InputModelData(j)

   Allocate(Xstate(TimePoints,N))
   Allocate(Xnew(N))
   Allocate(X(N))
   
   Allocate(reactioncountout(M))
   Allocate(TempC(M,MaxDepList))

  
   forall (k=1:M)
      TempC(k,1:MaxDepList) = Rxndata(k)%c(1:MaxDepList)
   end forall

   Call Init_SSA(M,NewModel = .TRUE.)

   !Make sure each processor has its own Random Seed
   RandSeed = RandSeed * (me + 1)
   Call Random_Seed(Put = RandSeed)

   !Trials loop
   do i = LastTrial + me + 1, Trials, NumProcs
   
   print*, "Trial #",i, " running on processor #", me

      Call cpu_time(t2)

      forall (k=1:M)
         Rxndata(k)%c(1:MaxDepList) = TempC(k,1:MaxDepList)
      end forall

      Call Init_SSA(M, NewModel = .FALSE.)

      !Generator's current Seed stored in RandSeed. Inputted into SSA Function. 
      Call Random_Seed(Get = RandSeed)

      print*, "Processor #", me, " has seed ", RandSeed

      !Initialize state variables
      t = TStart
      X = Xo

      !Initialize data storage
      Xstate(1,:) = Xo
      Time(1) = TStart

      plotcounter = 1
      
      do while (plotcounter < TimePoints)

      !Call ForwardPropogater(X,t,Rxndata,SaveTime,other parameters,ierror)
      !Propogater is any function, f(X,t,params), that produces X(t+dt) = f(X,t,params)
      !where the time step, dt, is now the data saving point

      !All propogaters must maintain a list of events that will occur after t + dt
      !and save the list for itself or other propogaters (using the indexed priority queue 
      !and the dynamic sorted queue)

      !Every propogater must return an error status. 0 is success. 
       
         Call SSA(Xnew, X, t, SaveTime, RandSeed, ierror)

         if (ierror > 0) THEN
            print*, "Error in propogater. Program is stopping."
            
            !Do more things here, such as saving last Trial/Model simulated
            
            stop
         end if

         plotcounter = plotcounter + 1

         !Save data to storage vectors
         
         Xstate(plotcounter,:) = Xnew
         X = Xnew
      
         if (i == 1.AND.j == 1) THEN
            !Only need to save time points on first trial of first model.
            !The propogater must ensure that X is saved only on the specified time points.

            Time(plotcounter) = t
         end if

         !Debugging
         !print*, "X = ", X
         !print*, "t = ", t

      end do                    !Time iterative loop
      Call Stats_SSA(reactioncountout)
      
      Call cpu_time(t3)
 
      Call WriteStateData(ExpType, i, j, XState, Time)

      Call Final_SSA(NewModel = .FALSE.)

      Call cpu_time(t4)

      AvgTime = AvgTime + (t4 - t2)
      DiskTime = DiskTime + (t4 - t3)
      AvgNumSlowRxns = AvgNumSlowRxns + sum(reactioncountout)

   end do                       !End trials iterative loop

   LastTrial = 0

   Deallocate(Xstate)
   Deallocate(Xnew)
   Deallocate(X)
   Deallocate(reactioncountout)
   Deallocate(TempC)    

   Call Final_SSA(NewModel = .TRUE.)

   Call cpu_time(t7)

   AvgModelTime = AvgModelTime + (t7 - t6)

   print*, "Model #", j, ": ", (t7 - t6), " CPU seconds."

end do                          !End Models iterative loop

Deallocate(Time)

!Deallocate allocatable arrays defined in DataInput module
Call Final_DataInput

#if defined(USING_MPI)

  print*, "#", me, " is done with computation!"
  Call MPI_Barrier(MPI_Comm_World, ierror)  
  
 

  Call MPI_Reduce(AvgNumSlowRxns, SumAvgNumSlowRxns, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

  Call MPI_Reduce(AvgTime, SumAvgTime, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

  Call MPI_Reduce(DiskTime, SumDiskTime, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

 
  Call MPI_Reduce(AvgModelTime, SumAvgModelTime, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)


  if (me == 0) THEN
    
    SumAvgNumSlowRxns = SumAvgNumSlowRxns / real(Trials,8) / real(NumModels,8)
    SumAvgTime = SumAvgTime / real(Trials,8)  / real(NumModels,8)
    SumDiskTime = SumDiskTime / real(Trials,8) / real(NumModels,8) 
    SumAvgModelTime = SumAvgModelTime / real(NumModels,8)

    print*, "Average Number of Slow Reactions per Trial = ", SumAvgNumSlowRxns  
    print*, "Average CPU Time per Model = ", SumAvgModelTime, " seconds."
    print*, "Average CPU Time per Trial = ", SumAvgTime, " seconds."
    print*, "Average Disk I/O per Trial = ", SumDiskTime, " seconds."
  end if


#else

  
  AvgNumSlowRxns = AvgNumSlowRxns / real(Trials,8) / real(NumModels,8)
  AvgTime = AvgTime / real(Trials,8) / real(NumModels,8)
  DiskTime = DiskTime / real(Trials,8) / real(NumModels,8)
  AvgModelTime = AvgModelTime / real(NumModels,8)

  print*, "Average Number of Slow Reactions per Trial = ", AvgNumSlowRxns
  print*, "Average CPU Time per Model = ", AvgModelTime, " seconds."
  print*, "Average CPU Time per Trial = ", AvgTime, " seconds."
  print*, "Average Disk I/O per Trial = ", DiskTime, " seconds."
#endif

if (me == 0) THEN

  Call check(NF90_open(trim(filename),NF90_Write,fileunit))
  Call check(NF90_redef(fileunit))
  Call check(NF90_put_att(fileunit,NF90_global,'Data_Written',int(1)))
  Call check(NF90_close(fileunit))

  Call cpu_time(t5)
 
  print*, "Total CPU Time required: ", t5 - t1, " seconds using ", NumProcs, " processors."
end if


#if defined(USING_MPI)
   Call MPI_Finalize(ierror)

   print*, "Successful MPI exit? ", ierror

#endif

print*, "Done!"

!---------------------------------------------------------------------
End Program Mainprogram
