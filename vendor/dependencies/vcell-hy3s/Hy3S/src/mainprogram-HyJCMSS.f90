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

!-----------------------
!****e Hy3S_main/hyhomss
!PROGRAM
!	Main program for the hybrid homogeneous simulator.
!	Uses the hybrid jump/continuous Markov process algorithm.
!PURPOSE
!	Simulates the stochastic dynamics of a general system of
!	chemical or biochemical reactions.
!AUTHOR
!	Created by Howard Salis. Contributions by Jon Tomshine & Vassilios Sotiropoulos.
!COMMENT
!	Licensed by the GNU Public License.
!SOURCE
!***
!-----------------------

program Mainprogram

!Main program for Hybrid Stochastic Simulations

!List of utilized modules - contains global variables used by this program

USE GlobalVariables
Use Propagators_HyJCMSS
Use DataIO
USE NetCDF

!Using MPI? If USING_MPI is defined, then MPI is on.

!!!DEC$ DEFINE USING_MPI
#if defined(USING_MPI)
  USE MPI
#endif

IMPLICIT NONE

Integer i, j, k, me, XStateID, TimeID, fileunit, ierror, NumProcs, plotcounter
REAL time1, time2 !to compute time between 2 trials or time steps. usded when outputing progress messages
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

!Simulation parameters
Real*8 :: epsilon, lambda, SDEdt, SDEdt_Adapt, deltat, SDE_Tolerance, MSR_Tolerance
Real*8 :: percentile

!These variables are used to compute the wall times for the program
Real*4 :: t1, t2, t3, t4, t5, t6, t7

!Variables for computing the statistics of the program
Integer, Allocatable :: Reactioncountout(:)
Integer :: Itercounter, Multicounter
Real*8 :: AvgTime, AvgNumIter, AvgNumSlowRxns, DiskTime, AvgSlowIter
Real*8 :: AvgModelTime

Logical :: is_stop_requested

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

#ifndef SVNVERSION
#error "SVNVERSION required"
#endif
! The build supplies SVNVERSION already quoted, and this concatenates it.
! It previously stringified an unquoted macro with #x, which gfortran cannot do:
! -cpp preprocesses in traditional mode, predating ANSI stringification. The
! $URL$ was an SVN keyword that stopped expanding when the project left SVN.
    print *, "Virtual Cell version " // SVNVERSION

Call cpu_time(t1)

!Get simulation parameters from command line arguments
!If there are none, set to default values

!Parse command line arguments and read in simulation parameters
Call ParseCLA(NumCLParameters, CLParameters)

#if defined(ADAPTIVE)
!Default values
epsilon = 1.000d2
lambda = 1.000d1
SDE_Tolerance = 1.000d-4
MSR_Tolerance = 1 / epsilon
SDEdt = 1.000d9 !Let SaveTime be the SDEdt

   Select Case (NumCLParameters)

      Case (-1)

      If (me == 0) THEN
	print*, "You are using Hy3S: Hybrid Stochastic Simulation for Supercomputers"
	print*, "Visit our website at http://hysss.sourceforge.net"
	print*, ""
	print*, "Algorithm: Hybrid Jump/Continuous Markov process simulator"
	print*, "System: Homogeneous bio/chemical systems"
	print*, "The SDE Numerical integrator is in Adaptive Step Mode."
	print*, ""
	print*, "Syntax: "

	print*, "ProgramName <NetCDF Filename> <epsilon> <lambda> <MSR_Tolerance> <SDE_Tolerance> <SDE_dt> [-R <Random Seed>] [-OV]"
	print*, "Definitions:"
	print*, "NetCDF Filename: The name of the NetCDF file containing the model data. The program will place the solution data in this file."
	print*, ""
	print*, "Epsilon: The minimum number of molecules of either the reactant or product species of a reaction before it is approximated &
	& as a continuous Markov process. (Default: 100 molecules)"
	print*, ""
	print*, "Lambda: The minimum reaction rate of a reaction before it is approximated as a continuous Markov process. (Default: 10 &
	& molecules/second)"
	print*, ""
	print*, "SDE_Tolerance: The maximum allowed Drift or Diffusion error. (Default: 1e-3)"
	print*, ""
	print*, "MSR_Tolerance: The maximum relative error generated by the Multiple Slow Reaction approximation. (Default: 1/epsilon)"
	print*, ""
	print*, "SDE_dt: The maximum allowed time step for the stochastic differential equation (SDE) numerical integrator. &
	& (Default: SaveTime of Model)"
	print*, ""
	print*, "Random Seed: The initial seed for the random number generator (a very long integer). (Default: Randomly generated, dependent on &
	& compiler.)"
	print*, ""
	print*, "-OV : Include to overwrite previously written solution data."
	print*, ""
	print*, "Program is stopping."


      End if

	stop

      Case (0)

      Case (1)
	epsilon = CLParameters(1)
	MSR_Tolerance = 1 / epsilon

      Case (2)
	epsilon = CLParameters(1)
	lambda = CLParameters(2)
	MSR_Tolerance = 1 / epsilon

      Case (3)
	epsilon = CLParameters(1)
	lambda = CLParameters(2)
	MSR_Tolerance = CLParameters(3)

      Case (4)
	epsilon = CLParameters(1)
	lambda = CLParameters(2)
	MSR_Tolerance = CLParameters(3)
	SDE_Tolerance = CLParameters(4)

      Case (5)
	epsilon = CLParameters(1)
	lambda = CLParameters(2)
	MSR_Tolerance = CLParameters(3)
	SDE_Tolerance = CLParameters(4)
	SDEdt=CLParameters(5)

    End Select


!Reads into data from NetCDF file and defines many model-related global variables -- contains some MPI code
Call InputModelDataInit

If (me == 0) THEN

   print*, "Hy3S: Hybrid Stochastic Simulation for Supercomputers"
   print*, "HTTP://hysss.sourceforge.net"
   print*, ""
   print*, "Algorithm: Hybrid Jump/Continuous Markov Stochastic Simulator (HyJCMSS)"
   print*, "System: Homogeneous Bio/Chemical Reaction systems"
   print*, ""

#if defined(MILSTEIN)
   print*, "SDE Numerical Integrator: Milstein method (1st order strong accuracy)"
#else
   print*, "SDE Numerical Integrator: Euler-Maruyama method (1/2 order strong accuracy)"
#endif

   print*, "Time Stepping: Adaptive"
   print*, ""
   if (OverWriteSolution == 1) THEN
      print*, "**Warning** Program is overwriting any previously written solution data."
      print*, ""
   end if
   print*, "Simulation Parameters: epsilon = ", epsilon, ", lambda = ", lambda, ", MSR Tolerance = ", MSR_Tolerance, &
   & " SDE Tolerance = ", SDE_Tolerance, ", SDE dt = ", min(SDEdt, SaveTime)
   print*, ""

   Call PrintModelData

End if

#else

!Default values
epsilon = 1.000d2
lambda = 1.000d1
MSR_Tolerance = 1.000d-2
SDEdt = 1.000e-1

   Select Case (NumCLParameters)

     Case (-1)

      If (me == 0) THEN
	print*, "You are using Hy3S: Hybrid Stochastic Simulation for Supercomputers"
	print*, "Visit our website at http://hysss.sourceforge.net"
	print*, ""
	print*, "Algorithm: Hybrid Jump/Continuous Markov process simulator"
	print*, "System: Homogeneous bio/chemical systems"
	print*, "The SDE Numerical integrator is in Fixed Step Mode."
	print*, ""
	print*, "Syntax: "
	print*, "ProgramName <NetCDF Filename> <epsilon> <lambda> <MSR_Tolerance> <SDE_dt> [-R <Random Seed>] [-OV]"
	print*, ""
	print*, "Definitions:"
	print*, "NetCDF Filename: The name of the NetCDF file containing the model data. The program will place the solution data in this file."
	print*, ""
	print*, "Epsilon: The minimum number of molecules of either the reactant or product species of a reaction before it is approximated &
	& as a continuous Markov process. (Default: 100 molecules)"
	print*, ""
	print*, "Lambda: The minimum reaction rate of a reaction before it is approximated as a continuous Markov process. (Default: 10 &
	& molecules/second)"
	print*, ""
	print*, "MSR_Tolerance: The maximum relative error generated by the Multiple Slow Reaction approximation. (Default: 0.01)"
	print*, ""
	print*, "SDE_dt: The maximum allowed time step for the stochastic differential equation (SDE) numerical integrator. &
	& (Default: 0.1 seconds)"
	print*, ""
	print*, "Random Seed: The initial seed for the random number generator (a very long integer). (Default: Randomly generated, dependent on &
	& compiler.)"
	print*, ""
	print*, "-OV : Include to overwrite previously written solution data."
	print*, "Program is stopping."
     End if

	stop

     Case (0)

     Case (1)
	epsilon = CLParameters(1)
	MSR_Tolerance = 1 / epsilon

     Case (2)
	epsilon = CLParameters(1)
	lambda = CLParameters(2)
	MSR_Tolerance = 1 / epsilon

     Case (3)
	epsilon = CLParameters(1)
	lambda = CLParameters(2)
	MSR_Tolerance = CLParameters(3)

     Case (4)
	epsilon = CLParameters(1)
	lambda = CLParameters(2)
	MSR_Tolerance = CLParameters(3)
	SDEdt = CLParameters(4)

   End Select

   !Reads into data from NetCDF file and defines many model-related global variables -- contains some MPI code
   Call InputModelDataInit
!!!have to comment all the standard outputs, this cosumes a lot time in Vcell UI.
   If (me == 0) THEN
!       print*, "Hy3S: Hybrid Stochastic Simulation for Supercomputers"
!       print*, "HTTP://hysss.sourceforge.net"
!	print*, ""
!	print*, "Algorithm: Hybrid Jump/Continuous Markov Stochastic Simulator (HyJCMSS)"
!	print*, "System: Homogeneous Bio/Chemical Reaction systems"
!	print*, ""

#if defined(MILSTEIN)
!	print*, "SDE Numerical Integrator: Milstein method (1st order strong accuracy)"
#else
!	print*, "SDE Numerical Integrator: Euler-Maruyama method (1/2 order strong accuracy)"
#endif

!	print*, "Time Stepping: Fixed"
!	print*, ""

!	print*, "Simulation Parameters: epsilon = ", epsilon, ", lambda = ", lambda, "MSR Tolerance = ", MSR_Tolerance, ", &
!	& SDE dt = ", min(SDEdt, SaveTime)
!	print*, ""

        Call PrintModelData

   End if

#endif

!Force all processors to wait until the output text has printed before continuing
!Makes it look neater

#if defined(USING_MPI)
   Call MPI_Barrier(MPI_Comm_World, ierror)
#endif

#if defined(USE_MESSAGING)
!initialize JMS
Call load_jms_info(jmsUrl, jmsUser, jmsPassword, jmsQueue, jmsTopic, vcellUser, simKey, jobIndex, taskID)
#endif

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

   Call Init_HyHomSS(M, N, lambda, epsilon, NewModel = .TRUE.)

   !Make sure each processor has its own Random Seed
   RandSeed = RandSeed * (me + 1)
   Call Random_Seed(Put = RandSeed)
!  Amended in July 2007 by Tracy LI, to send progress message to VCell
   CALL CPU_TIME(time1)
   percentile = 0.0
#if defined(USE_MESSAGING)
   Call send_progress(percentile, TStart)
#else
   print*,'[[[progress:0%]]]'
#endif

   !Trials loop
   do i = LastTrial + me + 1, Trials, NumProcs

#if defined(USE_MESSAGING)
		if (is_stop_requested()) THEN
			print*, "stopped by user"
   			stop 0
   		end if
#endif

!  Amended in July 2007 by Tracy LI, to send progress every 2 seconds to VCell for multiple trials
   If (Trials > 1) THEN
 		CALL CPU_TIME(time2)
 		if ((time2-time1) >= 2) THEN
 			percentile = ((i*1.0)/(Trials*1.0));
#if defined(USE_MESSAGING)
! send_progress takes a double by reference on the C side. `i` is the
! Integer trial counter, so this passed a 4-byte integer where 8 bytes are
! dereferenced -- a real bug, latent because Hy3S was never built in CI and
! Intel's implicit interfaces would not have caught it. Converted explicitly,
! which keeps the value the author meant to send.
 			Call send_progress(percentile, dble(i))
#else
 			print*,'[[[progress:',percentile*100,'%]]]'
#endif
 			time1=time2
 		end if
   end if
!Commented in July 2007 by Tracy LI, to avoid too much messaging to VCell
!!  print*, "Trial #",i, " running on processor #", me

      Call cpu_time(t2)

      forall (k=1:M)
         Rxndata(k)%c(1:MaxDepList) = TempC(k,1:MaxDepList)
      end forall

      Call Init_HyHomSS(M, N, lambda, epsilon, NewModel = .FALSE.)

      !Generator's current Seed stored in RandSeed. Inputted into SSA Function.
      Call Random_Seed(Get = RandSeed)

!!    print*, "Processor #", me, " has seed ", RandSeed

      !Initialize state variables
      t = TStart
      X = Xo

      !Initialize data storage
      Xstate(1,:) = Xo
      Time(1) = TStart

      plotcounter = 1

      do while (plotcounter < TimePoints)

#if defined(USE_MESSAGING)
		if (is_stop_requested()) THEN
			print*, "stopped by user"
			stop 0
   		end if
#endif

      !Call ForwardPropogater(X,t,Rxndata,SaveTime,other parameters,ierror)
      !Propogater is any function, f(X,t,params), that produces X(t+dt) = f(X,t,params)
      !where the time step, dt, is now the data saving point

      !All propogaters must maintain a list of events that will occur after t + dt
      !and save the list for itself or other propogaters (using the indexed priority queue
      !and the dynamic sorted queue)

      !Every propogater must return an error status. 0 is success.

         Call Hybrid_HomSS(Xnew, X, t, SaveTime, SDEdt, SDE_Tolerance, MSR_Tolerance, lambda, epsilon, RandSeed, ierror)
		 if (ierror > 0) THEN
            print*, "Error in propogater. Program is stopping."

            !Do more things here, such as saving last Trial/Model simulated

            stop 1
         end if
! Amended in July 2007 by Tracy LI. To send progress every 2 seconds to VCell for a single trajetory.
         If (Trials == 1) THEN
         	CALL CPU_TIME(time2)
  			if ((time2-time1) >= 2) THEN
  				percentile = ((plotcounter*1.00)/(TimePoints*1.00))
#if defined(USE_MESSAGING)
  				Call send_progress(percentile, t);
#else
		 		print*,'[[[progress:',percentile*100,'%]]]'
#endif
		 		time1 = time2
		 	end if
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

      Call Stats_HyHomSS(reactioncountout, Itercounter, MultiCounter)

      AvgNumIter = AvgNumIter + Itercounter
      AvgNumSlowRxns = AvgNumSlowRxns + sum(reactioncountout)
      AvgSlowIter = AvgSlowIter + MultiCounter

      Call cpu_time(t3)

      Call WriteStateData(ExpType, i, j, XState, Time)

      Call Final_HyHomSS(NewModel = .FALSE.)

      Call cpu_time(t4)

      AvgTime = AvgTime + (t4 - t2)
      DiskTime = DiskTime + (t4 - t3)

   end do                       !End trials iterative loop

   LastTrial = 0

   Deallocate(Xstate)
   Deallocate(Xnew)
   Deallocate(X)
   Deallocate(reactioncountout)
   Deallocate(TempC)

   Call Final_HyHomSS(NewModel = .TRUE.)

   Call cpu_time(t7)

   AvgModelTime = AvgModelTime + (t7 - t6)

!!  print*, "Model #", j, ": ", (t7 - t6), " CPU seconds."

end do                          !End Models iterative loop

Deallocate(Time)

!Deallocate allocatable arrays defined in DataInput module
Call Final_DataInput

#if defined(USING_MPI)

  print*, "#", me, " is done with computation!"
  Call MPI_Barrier(MPI_Comm_World, ierror)

  Call MPI_Reduce(AvgNumIter, SumAvgNumIter, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

  Call MPI_Reduce(AvgNumSlowRxns, SumAvgNumSlowRxns, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

  Call MPI_Reduce(AvgTime, SumAvgTime, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

  Call MPI_Reduce(DiskTime, SumDiskTime, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

  Call MPI_Reduce(AvgSlowIter, SumAvgSlowIter, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)

  Call MPI_Reduce(AvgModelTime, SumAvgModelTime, 1, MPI_DOUBLE_PRECISION, &
  & MPI_SUM, 0, MPI_Comm_World, ierror)


  if (me == 0) THEN
    SumAvgNumIter = SumAvgNumIter / real(Trials,8) / real(NumModels,8)
    SumAvgNumSlowRxns = SumAvgNumSlowRxns / real(Trials,8) / real(NumModels,8)
    SumAvgTime = SumAvgTime / real(Trials,8)  / real(NumModels,8)
    SumDiskTime = SumDiskTime / real(Trials,8) / real(NumModels,8)
    SumAvgSlowIter = SumAvgSlowIter / real(Trials,8) / real(NumModels,8) / min(real(1e20,8),SumAvgNumIter)
    SumAvgModelTime = SumAvgModelTime / real(NumModels,8)

    print*, "Average Number of Slow Reactions per Trial = ", SumAvgNumSlowRxns
    print*, "Average Number of Iterations per Trial = ", SumAvgNumIter
    print*, "Average Number of Slow Reactions per MSR Approx. = ", SumAvgSlowIter
    print*, "Average CPU Time per Model = ", SumAvgModelTime, " seconds."
    print*, "Average CPU Time per Trial = ", SumAvgTime, " seconds."
    print*, "Average Disk I/O per Trial = ", SumDiskTime, " seconds."
  end if


#else

  AvgNumIter = AvgNumIter / real(Trials,8) / real(NumModels,8)
  AvgNumSlowRxns = AvgNumSlowRxns / real(Trials,8) / real(NumModels,8)
  AvgTime = AvgTime / real(Trials,8) / real(NumModels,8)
  DiskTime = DiskTime / real(Trials,8) / real(NumModels,8)
  AvgModelTime = AvgModelTime / real(NumModels,8)

  if (AvgNumIter > 0) THEN
     AvgSlowIter = AvgSlowIter / real(Trials,8) / real(NumModels,8) / AvgNumIter
  else
     AvgSlowIter = 0.0
  end if

  print*, "Average Number of Slow Reactions per Trial = ", AvgNumSlowRxns
  print*, "Average Number of Iterations per Trial = ", AvgNumIter
  print*, "Average Number of Slow Reactions per MSR Approx. = ", AvgSlowIter
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
  if (brownianTreeWarningCount > 0) then
	print*, "Brownian tree warning occurred ", brownianTreeWarningCount , " times."		
  end if
end if


#if defined(USING_MPI)
   Call MPI_Finalize(ierror)

   print*, "Successful MPI exit? ", ierror

#endif

#if defined(USE_MESSAGING)
	Call send_complete(t);
#else
	print*, "Done!"
#endif

!-----------------------------------------------------------------------
End Program Mainprogram
