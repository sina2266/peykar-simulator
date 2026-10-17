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

!-------------------------
!****m shared/dataio
!MODULE
!	Data Input/Output
!PURPOSE
!	This module contains all of the subroutines responsible for
!	reading and writing from/to NetCDF files, converting units from
!	macroscopic to mesoscopic (and vica verca), and making sure all
!	solution and time variables are properly defined.
!AUTHOR
!	Created by Howard Salis. Contributions by Jon Tomshine. (2004)
!COMMENT
!	What are Experiment Types (also called Experimental Types or just ExpTypes)?
!
!	ExpTypes identify what sort of 'computational experiment' the simulation will perform.
!	In order to perform the experiment, different NetCDF variables may be needed.
!	By reading the ExpType, the DataIO module will read in the required NetCDF
!	variables, such as simulation parameters or model information.
!
!	Strictly speaking, each ExpType has no relation to the simulation method performed.
!	For example, if two different simulation methods require the same set of variables,
!	then they may use the same ExpType. The program may distinguish one method from another
!	using information read in from NetCDF variables.
!
!	The ExpType is best described as a mapping from an integer to a defined set of NetCDF
!	variables, including their dimensions, types, name, and information content.
!
!	If you are developing a new simulation method that requires different information from the
!	input file (either changing the dimensions of any NetCDF variables, their data types, or
!	adding new variables or dimensions), then define a new ExpType. Otherwise, use an existing one.
!
!	Here are the current ExpTypes:
!
!	1: Single Model - Contains enough information to simulate only a single, defined model.
!	2: Multiple Models, varying initial conditions and/or kinetic parameters. Each model is fully defined.
!	3: Multiple Models, varying initial conditions and/or kinetic parameters.
!		Each model is somehow changed by the method. This will include any optimization
!		method that changes the kinetic parameters or initial conditions of a model.
!	4: Multiple Models, varying the system of reactions, initial conditions, and/or kinetic parameters.
!		Each model is somehow changed by the method. This will include any evolution in silico
!		methods.
!	5: TBD -- Add as needed.
!
!	This Module Contains MPI Code.
!
!	This Module Contains Numerous Global Variables in its Module Space.
!SOURCE

Module DataIO
!Data Input/Output module for NetCDF files
!Reads command line, reads model data, writes model data, writes state data

USE netcdf
! F2KCLI is deliberately not used here any more. It is a shim that predates
! Fortran 2003, supplying COMMAND_ARGUMENT_COUNT and GET_COMMAND_ARGUMENT for
! compilers that lacked them -- the only two routines this module ever took
! from it. Both are standard intrinsics now, implemented natively by gfortran
! and by Intel Fortran, so the shim adds nothing and costs portability: the
! copy vendored here is the Unix variant, whose own header says it excludes
! "compilers which require a special USE statement to make IARGC/GETARG
! available", which is precisely Intel Fortran on Windows. Built against it,
! all three Windows binaries died with an access violation as soon as they
! touched the command line.
USE GLOBALVARIABLES
USE RateLaws

!Using MPI? Defined USING_MPI = yes, Not Defined = no

!!!DEC$ DEFINE USING_MPI

#if defined(USING_MPI)
   USE MPI
#endif

IMPLICIT NONE
private

Public :: InputModelDataInit, InputModelData, Final_DataInput,  ParseCLA, WriteStateData, check, WriteModelData
Public :: PrintModelData

!***
CONTAINS
!-------------------------------------------------------------------------------

!****p dataio/ParseCLA
!NAME
!	ParseCLA
!PURPOSE
!	Parses the command line arguments of the program and converts any
!	numeric strings into double precision values.
!	Accepts the -OV and -R <RandSeed> options.
!
!	Returns the number of command line parameters and the values of
!	those parameters.
!
!	Modified the global variables OverWriteSolution and RandSeed.
!
!SOURCE

Subroutine ParseCLA(NumCLParameters, CLParameters)
IMPLICIT NONE

Integer, intent(out) :: NumCLParameters
Real*8, intent(out) :: CLParameters(:)

!***

integer*4 :: RandSeedNum
integer :: RandSize, status, NARG, ierror, me
integer :: CLcounter, i
Real*8 :: ParameterNum

Character(len=36) :: fileoption, RandSeedNumChar, ProgramName, arg, taskIDStr

!Command Line Syntax: <Command> <NetCDF File> <P1> <P2> ... <Pn> [-R <RandSeed>] [-OV]
!P1 ... Pn are command line parameters passed to the mainprogram within 'CLParameters'
!-R <RandSeed> optionally allows the user to select the random seed integer value
!-OV allows the user to overwrite the solution contained within the NetCDF file

NARG = COMMAND_ARGUMENT_COUNT()

CLParameters = 0.000000000
NumCLParameters = 0
OverWriteSolution = 0

taskID = -1

Call Random_Seed(Size = RandSize)
Allocate(RandSeed(RandSize))

Call Get_Command_Argument(0,ProgramName)

if (NARG == 0) THEN
	print*, "Program Name: ", trim(ProgramName)
	NumCLParameters = -1
else

Call Get_Command_Argument(1,filename)
RandSeedNumChar = ''

CLcounter = 1

do while (CLcounter < NARG)
	   CLcounter = CLcounter + 1

	   Call Get_Command_Argument(CLcounter, arg)

	   if (arg(1:len_trim(arg)) == '-R'.OR.arg(1:len_trim(arg)) == '-OV'.OR.arg(1:len_trim(arg)) == '-tid') THEN

		  if (arg(1:len_trim(arg)) == '-R') THEN
		!If parameter is '-R', the next parameter is the random seed number

		 CLcounter = CLcounter + 1
		 Call Get_Command_Argument(CLcounter,RandSeedNumChar,ierror)

		  end if

		  if (arg(1:len_trim(arg)) == '-OV') THEN
			 OverWriteSolution = 1
		  end if

		  if (arg(1:len_trim(arg)) == '-tid') THEN
			 CLcounter = CLcounter + 1
			 Call Get_Command_Argument(CLcounter,taskIDStr,ierror)
		     taskID=0
		     if (taskIDStr(1:1) == '-') THEN
		     	do i=2,len(trim(taskIDStr))
		        	taskID = taskID + (ichar(taskIDStr(i:i)) - 48)*10**(len(trim(taskIDStr))-i)
		     	end do
		     	taskID = -taskID;
		     else
		     	do i=1,len(trim(taskIDStr))
		        	taskID = taskID + (ichar(taskIDStr(i:i)) - 48)*10**(len(trim(taskIDStr))-i)
		     	end do
		     endif
		endif
   else

	      !Argument is a parameter.
	      !
	      ! This used to be converted by hand, digit by digit, and the
	      ! conversion was wrong in three ways: without a decimal point an
	      ! exponent misplaced the mantissa ("1e9" parsed as 0, "1e-10" as
	      ! 6.3e-9); a multi-digit exponent was read with its digits reversed
	      ! ("1.0e-10" parsed as 0.1, "1.0e12" as 1e21); and a leading minus
	      ! sign cost the integer part a power of ten ("-12.5" parsed as -1.5).
	      ! VCell sends String.valueOf(double), which always has a decimal
	      ! point and almost never a two-digit exponent, so it rarely showed.
	      ! A list-directed read is the standard conversion and has none of it.
	      read(arg, *, iostat=ierror) ParameterNum
	      if (ierror /= 0) THEN
		 print*, "Invalid numeric parameter: ", trim(arg)
		 error stop 2
	      end if

	      NumCLParameters = NumCLParameters + 1
	      CLParameters(NumCLParameters) = ParameterNum

	   end if

	end do

	if (len(trim(RandSeedNumChar)) > 0) THEN

  	  if (scan(RandSeedNumChar,'.') > 0.OR.scan(RandSeedNumChar,'-') > 0) THEN
	     print*, "Invalid random seed number. Number must be a positive integer."
	     print*, "Program is stopping."
             stop
	  else
	     RandSeedNum=0
	     do i=1,len(trim(RandSeedNumChar))
	        RandSeedNum = RandSeedNum + (ichar(RandSeedNumChar(i:i)) - 48)*10**(len(trim(RandSeedNumChar))-i)
	     end do

	     RandSeed = RandSeedNum
	  end if
	else
	   Call Random_Seed
	   Call Random_Seed(Get = RandSeed)
	end if
end if

End Subroutine ParseCLA
!-------------------------------------------------------------------------------

!****p dataio/InputModelDataInit
!NAME
!	InputModelDataInit
!PURPOSE
!	This subroutine opens the NetCDF file and reads in any simulation or model data
!	that does not change from one model to another. The set of NetCDF variables read into
!	memory is dependent on the ExpType.
!
!	This subroutine calls the CheckAndDefineVariables subroutine.
!
!	This subroutine modifies many global variables.
!
!	This subroutine should only be called once prior to the simulation of single or
!	multiple models. Any NetCDF variables that need to be read in only once should be
!	placed here.
!SOURCE

Subroutine InputModelDataInit

!***

integer :: fileunit, datapointer(10), i, RandSize, status, jmsid(8)
integer :: NdimID, MdimID, TrialsID, TimepointsID, XstateID, TimeID, ProbDistrID, ProbDistrRangeID, DataLenID
integer :: CDFMaxDepList, CDFMaxDepListID, NumBinsID, ModelsID, NumSavedSpeciesID, NumPerturbationsID

integer :: DataWritten, ierror, me, OldFillMode, NumSavedSpecies, error
Integer, Allocatable :: SaveSpeciesDataTemp(:)

Character(len = 256) :: errormsg

!MPI code
#if defined(USING_MPI)
  Call MPI_Comm_Rank(MPI_Comm_World,me,ierror)
#else
  me = 0
#endif

!If the file does not have its solution data variables pre-defined, the first processor defines them.
!The rest of the processors wait.

  error = 0

  if (me == 0) THEN
     !Check the file for inconsistencies, errors, missing variables, etc. Define solution variables if they don't exist.
     !Return errors and error messages

     Call CheckAndDefineVariables(error, errormsg)

  end if

  !Send error value to all processors -- Not Done Yet, But Still Somehow Works Just Fine


#if defined(USING_MPI)
   Call MPI_BARRIER(MPI_Comm_World, IERROR)
#endif

   if (error == 1) THEN
      if (me == 0) THEN
	 	print*, trim(errormsg)
      end if
      stop
   end if

!There should be no print statements below here.

status = NF90_open(path = trim(filename),mode = NF90_Share,ncid = fileunit)

!Read in simulation parameter variables - variables that do not change over a multi-model simulation
!Variables are read in according to the file's 'ExpType' or Experiment Type
!For easy maintenance, each ExpType will map to a list of its necessary variables
!ExpTypes will sometimes need the same variables so there will be repetition.

status = NF90_inq_varid(fileunit,'ExpType',datapointer(5))
if (status /= NF90_NoErr) THEN
   ExpType = 1
else
   Call check(NF90_get_var(fileunit,datapointer(5),ExpType))
end if

status = NF90_inq_dimid(fileunit,'NumPerturbations',NumPerturbationsID)
if (status == NF90_NoErr) THEN
   Call check(NF90_inquire_dimension(fileunit,NumPerturbationsID,len = NumPerturbations))

   Call check(NF90_inq_dimid(fileunit,'DataLen',DataLenID))
   Call check(NF90_inquire_dimension(fileunit,DataLenID,len = DataLen))

   if (NumPerturbations > 0) THEN

      Allocate(PerturbationIDs(NumPerturbations, DataLen))
      Allocate(PerturbationData(NumPerturbations, DataLen))

      Call check(NF90_inq_varid(fileunit,'PerturbationIDs',datapointer(8)))
      Call check(NF90_inq_varid(fileunit,'PerturbationData',datapointer(9)))

      do i=1,NumPerturbations
         Call check(NF90_get_var(fileunit,datapointer(8),PerturbationIDs(i,:), start = (/1, i/), count = (/DataLen, 1/) ))
         Call check(NF90_get_var(fileunit,datapointer(9),PerturbationData(i,:), start = (/1, i/),  count = (/DataLen, 1/) ))
      end do

   end if
else
   NumPerturbations = 0
end if

Select Case (ExpType)

Case (1)

   ! jms info
   !Call check(NF90_def_dim(fileunit,'StringLen',StringLen,StringLenID))
   !Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

#if defined(USE_MESSAGING)
   Call check(NF90_inq_varid(fileunit,'JMS_BROKER',jmsid(1)))
   Call check(NF90_get_var(fileunit,jmsid(1),jmsUrl))

   Call check(NF90_inq_varid(fileunit,'JMS_USER',jmsid(2)))
   Call check(NF90_get_var(fileunit,jmsid(2),jmsUser))

   Call check(NF90_inq_varid(fileunit,'JMS_PASSWORD',jmsid(3)))
   Call check(NF90_get_var(fileunit,jmsid(3),jmsPassword))

   Call check(NF90_inq_varid(fileunit,'JMS_QUEUE',jmsid(4)))
   Call check(NF90_get_var(fileunit,jmsid(4),jmsQueue))

   Call check(NF90_inq_varid(fileunit,'JMS_TOPIC',jmsid(5)))
   Call check(NF90_get_var(fileunit,jmsid(5),jmsTopic))

   Call check(NF90_inq_varid(fileunit,'VCELL_USER',jmsid(6)))
   Call check(NF90_get_var(fileunit,jmsid(6),vcellUser))

   Call check(NF90_inq_varid(fileunit,'SIMULATION_KEY',jmsid(7)))
   Call check(NF90_get_var(fileunit,jmsid(7),simKey))

   Call check(NF90_inq_varid(fileunit,'JOB_INDEX',jmsid(8)))
   Call check(NF90_get_var(fileunit,jmsid(8),jobIndex))
#endif

   !Time-related variables
   Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),TStart))

   Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

   Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

   TimePoints = int((TEnd - TStart) / SaveTime) + 1

   !Read in last trial simulated
   status = NF90_inq_varid(fileunit,'LastTrial',datapointer(4))
   if (status == NF90_NoErr) THEN
      Call check(NF90_get_var(fileunit,datapointer(4),LastTrial))
   else
      LastTrial = 0
   end if

   !Set multi-model related variables to default values
   LastModel = 0
   NumModels = 1
   MaxNumModels = 1


   if (OverWriteSolution == 1) THEN
      !Over write the solution data
      LastModel = 0
      LastTrial = 0
   end if

   !Read in number of trials
   Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
   Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

   !Read in maximum number of reaction Depend List
   Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
   Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

   !Read in number of reactions and species
   Call check(NF90_inq_dimid(fileunit,'NumReactions',MdimID))
   Call check(NF90_inquire_dimension(fileunit,MdimID,len = M))

   Call check(NF90_inq_dimid(fileunit,'NumSpecies',NdimID))
   Call check(NF90_inquire_dimension(fileunit,NdimID,len = N))

   N_Max = N
   M_Max = M


Case (2)

   !Time-related variables
   Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),TStart))

   Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

   Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

   TimePoints = int((TEnd - TStart) / SaveTime) + 1

   !Read in last trial simulated
   status = NF90_inq_varid(fileunit,'LastTrial',datapointer(4))
   if (status == NF90_NoErr) THEN
      Call check(NF90_get_var(fileunit,datapointer(4),LastTrial))
   else
      LastTrial = 0
   end if

   !Read in multi-model related variables
   Call check(NF90_inq_varid(fileunit,'LastModel',datapointer(6)))
   Call check(NF90_get_var(fileunit,datapointer(6),LastModel))

   Call check(NF90_inq_varid(fileunit,'MaxNumModels',datapointer(7)))
   Call check(NF90_get_var(fileunit,datapointer(7),MaxNumModels))

   Call check(NF90_inq_dimid(fileunit,'NumModels',ModelsID))
   Call check(NF90_inquire_dimension(fileunit,ModelsID,len = NumModels))

   if (OverWriteSolution == 1) THEN
      !Over write the solution data
      LastModel = 0
      LastTrial = 0
   end if

   !Read in number of trials
   Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
   Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

   !Read in maximum number of reaction Depend List
   Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
   Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

   !Read in number of reactions and species
   Call check(NF90_inq_dimid(fileunit,'NumReactions',MdimID))
   Call check(NF90_inquire_dimension(fileunit,MdimID,len = M))

   Call check(NF90_inq_dimid(fileunit,'NumSpecies',NdimID))
   Call check(NF90_inquire_dimension(fileunit,NdimID,len = N))

   N_Max = N
   M_Max = M


Case (3)

   !Time-related variables
   Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),TStart))

   Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

   Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

   TimePoints = int((TEnd - TStart) / SaveTime) + 1

   !Read in last trial simulated
   status = NF90_inq_varid(fileunit,'LastTrial',datapointer(4))
   if (status == NF90_NoErr) THEN
      Call check(NF90_get_var(fileunit,datapointer(4),LastTrial))
   else
      LastTrial = 0
   end if

   !Read in multi-model related variables
   Call check(NF90_inq_varid(fileunit,'LastModel',datapointer(6)))
   Call check(NF90_get_var(fileunit,datapointer(6),LastModel))

   Call check(NF90_inq_varid(fileunit,'MaxNumModels',datapointer(7)))
   Call check(NF90_get_var(fileunit,datapointer(7),MaxNumModels))

   Call check(NF90_inq_dimid(fileunit,'NumModels',ModelsID))
   Call check(NF90_inquire_dimension(fileunit,ModelsID,len = NumModels))


   if (OverWriteSolution == 1) THEN
      !Over write the solution data
      LastModel = 0
      LastTrial = 0
   end if

   !Read in number of trials
   Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
   Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

   !Read in maximum number of reaction Depend List
   Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
   Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

   !Read in number of reactions and species
   Call check(NF90_inq_dimid(fileunit,'NumReactions',MdimID))
   Call check(NF90_inquire_dimension(fileunit,MdimID,len = M))

   Call check(NF90_inq_dimid(fileunit,'NumSpecies',NdimID))
   Call check(NF90_inquire_dimension(fileunit,NdimID,len = N))

   N_Max = N
   M_Max = M

Case (4)

   !Time-related variables
   Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),TStart))

   Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

   Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

   TimePoints = int((TEnd - TStart) / SaveTime) + 1

   !Read in last trial simulated
   status = NF90_inq_varid(fileunit,'LastTrial',datapointer(4))
  if (status == NF90_NoErr) THEN
     Call check(NF90_get_var(fileunit,datapointer(4),LastTrial))
  else
     LastTrial = 0
  end if


   !Read in multi-model related variables
   Call check(NF90_inq_varid(fileunit,'LastModel',datapointer(6)))
   Call check(NF90_get_var(fileunit,datapointer(6),LastModel))

   Call check(NF90_inq_varid(fileunit,'MaxNumModels',datapointer(7)))
   Call check(NF90_get_var(fileunit,datapointer(7),MaxNumModels))

   Call check(NF90_inq_dimid(fileunit,'NumModels',ModelsID))
   Call check(NF90_inquire_dimension(fileunit,ModelsID,len = NumModels))


   if (OverWriteSolution == 1) THEN
      !Over write the solution data
      LastModel = 0
      LastTrial = 0
   end if

   !Read in number of trials
   Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
   Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

   !Read in maximum number of reaction Depend List
   Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
   Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

   !Read in maximum number of reactions and species
   Call check(NF90_inq_dimid(fileunit,'MaxNumReactions',MdimID))
   Call check(NF90_inquire_dimension(fileunit,MdimID,len = M_Max))

   Call check(NF90_inq_dimid(fileunit,'MaxNumSpecies',NdimID))
   Call check(NF90_inquire_dimension(fileunit,NdimID,len = N_Max))


Case (5)
   !TBD

End Select


Call check(NF90_close(fileunit))

#if defined(USING_MPI)
   Call MPI_BARRIER(MPI_Comm_World, IERROR)
#endif

 End subroutine InputModelDataInit
!-------------------------------------------------------------------------------------------

!****p dataio/InputModelData
!NAME
!	InputModelData
!PURPOSE
!	This subroutine opens the NetCDF file and reads in the NetCDF variables required to
!	simulate the Model^th model (except those that do not change from model to model).
!	The set of NetCDF variables read from file is dependent on the ExpType.
!
!	This subroutine changes many global variables in module dataio.
!
!	This subroutine converts all kinetic parameters from macroscopic
!	to mesoscopic units, creates the dependency graph, and creates the list
!	of special events (Elist).
!
!	This subroutine should be called once prior to the simulation of the Model^th model.
!SOURCE

Subroutine InputModelData(Model)
Integer, intent(in) :: Model
!***

integer :: fileunit, NVarID, MVarID, i, datapointer(14)
Integer, Allocatable :: SplitOnDivisionTemp(:), SaveSpeciesDataTemp(:)


!Open NetCDF file
Call check(NF90_open(path = trim(filename),mode = NF90_Share,ncid = fileunit))

!Read into model-dependent variables of the RecNum_th model, dependent on ExpType

Select Case (ExpType)
!Read in number of species or reactions from Model (only necessary when it may vary between models)


Case (3)
! read in previous 2 states


Case (4)
   Call check(NF90_inq_varid(fileunit,'NumReactions', MVarID))
   Call check(NF90_get_var(fileunit, MVarID, M, start = (/ Model /) ))

   Call check(NF90_inq_varid(fileunit,'NumSpecies', NVarID))
   Call check(NF90_get_var(fileunit, NVarID, N, start = (/ Model /) ))

Case (5)

End Select


!Freshly Allocate memory if necessary


Call AllocateLocalModel(N, M)

Allocate(SplitOnDivisionTemp(N))
Allocate(SaveSpeciesDataTemp(N))

Select Case (ExpType)

Case (1)

   !No NetCDF variables have record dimensions

   if (Model > 1) THEN
      print*, "Warning: This NetCDF file contains only a single model. That model's data is being read."
   end if

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),Vo))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),CellGrowthTime))


   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),CellGrowthTimeSD))

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(4)))
   Call check(NF90_get_var(fileunit,datapointer(4),SplitOnDivisionTemp, count = (/N/) ))


   where (SplitOnDivisionTemp >= 1)
      SplitOnDivision = .TRUE.
   elsewhere
      SplitOnDivision = .FALSE.
   end where

   Deallocate(SplitOnDivisionTemp)

   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5)))
   Call check(NF90_get_var(fileunit,datapointer(5),SaveSpeciesDataTemp, count = (/N/) ))

   where (SaveSpeciesDataTemp >= 1)
      SaveSpeciesData = .TRUE.
   elsewhere
      SaveSpeciesData = .FALSE.
   end where

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(6)))
   Call check(NF90_get_var(fileunit,datapointer(6),Xo, count = (/N/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(7)))
   Call check(NF90_get_var(fileunit,datapointer(7),Rxndata(:)%Data, count = (/M/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(8)))
   Call check(NF90_get_var(fileunit,datapointer(8),Rxndata(:)%Mtype, count = (/ M /) ))


   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(9)))
   Call check(NF90_get_var(fileunit,datapointer(9),Rxndata(:)%DListLen, count = (/M/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(10)))
   Call check(NF90_get_var(fileunit,datapointer(10),Rxndata(:)%SListLen,count = (/ M /)))

   forall (i=1:M)
      Rxndata(i)%SList = 0
      Rxndata(i)%DList = 0
      Rxndata(i)%c = 0.0
      Rxndata(i)%v = 0
      Rxndata(i)%EList = 0
   end forall

   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(13)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(14)))

   do i=1, M

      Call check(NF90_get_var(fileunit,datapointer(11),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(12),Rxndata(i)%c(1:MaxDepList),start = (/1, i/), count = (/MaxDepList, 1/)))

      Call check(NF90_get_var(fileunit,datapointer(13),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(14),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i/), &
           & count = (/Rxndata(i)%DListLen,1 /) ))

   end do

Case (2)

   !NetCDF Variables 'Reaction_Rate_Constants' and 'SpeciesIC' have record dimensions

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),Xo, start = (/1, Model /), count = (/N, 1/) ))

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),Vo))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),CellGrowthTime))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(4)))
   Call check(NF90_get_var(fileunit,datapointer(4),CellGrowthTimeSD))

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(5)))
   Call check(NF90_get_var(fileunit,datapointer(5),SplitOnDivisionTemp, count = (/N/) ))

   where (SplitOnDivisionTemp == 1)
      SplitOnDivision = .TRUE.
   elsewhere
      SplitOnDivision = .FALSE.
   end where
   Deallocate(SplitOnDivisionTemp)


   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(14)))
   Call check(NF90_get_var(fileunit,datapointer(14),SaveSpeciesDataTemp, count = (/N/) ))

   where (SaveSpeciesDataTemp >= 1)
      SaveSpeciesData = .TRUE.
   elsewhere
      SaveSpeciesData = .FALSE.
   end where

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(6)))
   Call check(NF90_get_var(fileunit,datapointer(6),Rxndata(:)%Data, count = (/M/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(7)))
   Call check(NF90_get_var(fileunit,datapointer(7),Rxndata(:)%Mtype, count = (/ M /) ))

   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(8)))
   Call check(NF90_get_var(fileunit,datapointer(8),Rxndata(:)%DListLen, count = (/M/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(9)))
   Call check(NF90_get_var(fileunit,datapointer(9),Rxndata(:)%SListLen,count = (/ M /)))

   forall (i=1:M)
      Rxndata(i)%SList = 0
      Rxndata(i)%DList = 0
      Rxndata(i)%c = 0.0
      Rxndata(i)%v = 0
      Rxndata(i)%EList = 0
   end forall


   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(10)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(13)))

   do i=1, M

      Call check(NF90_get_var(fileunit,datapointer(10),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(11),Rxndata(i)%c(1:MaxDepList),start = (/1, i, Model/), &
           & count = (/MaxDepList, 1, 1/)))

      Call check(NF90_get_var(fileunit,datapointer(12),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(13),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i/), &
           & count = (/Rxndata(i)%DListLen,1 /) ))

   end do

Case (3)

   !NetCDF Variables 'Reaction_Rate_Constants' and 'SpeciesIC' have record dimensions

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),Xo, start = (/1, Model /), count = (/N, 1/) ))

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),Vo))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),CellGrowthTime))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(4)))
   Call check(NF90_get_var(fileunit,datapointer(4),CellGrowthTimeSD))

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(5)))
   Call check(NF90_get_var(fileunit,datapointer(5),SplitOnDivisionTemp, count = (/N/) ))

   where (SplitOnDivisionTemp == 1)
      SplitOnDivision = .TRUE.
   elsewhere
      SplitOnDivision = .FALSE.
   end where
   Deallocate(SplitOnDivisionTemp)


   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(14)))
   Call check(NF90_get_var(fileunit,datapointer(14),SaveSpeciesDataTemp, count = (/N/) ))

   where (SaveSpeciesDataTemp >= 1)
      SaveSpeciesData = .TRUE.
   elsewhere
      SaveSpeciesData = .FALSE.
   end where

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(6)))
   Call check(NF90_get_var(fileunit,datapointer(6),Rxndata(:)%Data, count = (/M/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(7)))
   Call check(NF90_get_var(fileunit,datapointer(7),Rxndata(:)%Mtype, count = (/ M /) ))

   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(8)))
   Call check(NF90_get_var(fileunit,datapointer(8),Rxndata(:)%DListLen, count = (/M/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(9)))
   Call check(NF90_get_var(fileunit,datapointer(9),Rxndata(:)%SListLen,count = (/ M /)))

   forall (i=1:M)
      Rxndata(i)%SList = 0
      Rxndata(i)%DList = 0
      Rxndata(i)%c = 0.0
      Rxndata(i)%v = 0
      Rxndata(i)%EList = 0
   end forall


   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(10)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(13)))

   do i=1, M

      Call check(NF90_get_var(fileunit,datapointer(10),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(11),Rxndata(i)%c(1:MaxDepList),start = (/1, i, Model/), &
           & count = (/MaxDepList, 1, 1/)))

      Call check(NF90_get_var(fileunit,datapointer(12),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(13),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i/), &
	      & count = (/Rxndata(i)%DListLen,1 /) ))

   end do


Case (4)

   !The following NetCDF variables have record dimensions: SpeciesIC, SpeciesSplitOnDivision, Reaction_Rate_Laws,
   !Reaction_DListLen, Reaction_StoichListLen, Reaction_DepList, Reaction_StoichCoeff, Reaction_StoichSpecies,
   !Reaction_Rate_Constants, Reaction_OptionalData

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),Vo))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),CellGrowthTime))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),CellGrowthTimeSD))

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(4)))
   Call check(NF90_get_var(fileunit,datapointer(4),Xo, start = (/1, Model /), count = (/N, 1/) ))

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(5)))
   Call check(NF90_get_var(fileunit,datapointer(5),SplitOnDivisionTemp, start = (/1, Model/), count = (/N, 1/) ))

   where (SplitOnDivisionTemp == 1)
      SplitOnDivision = .TRUE.
   elsewhere
      SplitOnDivision = .FALSE.
   end where
   Deallocate(SplitOnDivisionTemp)

   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(14)))
   Call check(NF90_get_var(fileunit,datapointer(14),SaveSpeciesDataTemp, start = (/1, Model/), count = (/N, 1/) ))

   where (SaveSpeciesDataTemp >= 1)
      SaveSpeciesData = .TRUE.
   elsewhere
      SaveSpeciesData = .FALSE.
   end where

   Deallocate(SaveSpeciesDataTemp)


   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(6)))
   Call check(NF90_get_var(fileunit,datapointer(6),Rxndata(:)%Data, start = (/1, Model /), count = (/M, 1/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(7)))
   Call check(NF90_get_var(fileunit,datapointer(7),Rxndata(:)%Mtype, start = (/1, Model/), count = (/ M, 1/) ))

   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(8)))
   Call check(NF90_get_var(fileunit,datapointer(8),Rxndata(:)%DListLen, start = (/1, Model/), count = (/M, 1/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(9)))
   Call check(NF90_get_var(fileunit,datapointer(9),Rxndata(:)%SListLen, start = (/1, Model/), count = (/ M, 1 /) ))

   forall (i=1:M)
      Rxndata(i)%SList = 0
      Rxndata(i)%DList = 0
      Rxndata(i)%c = 0.0
      Rxndata(i)%v = 0
      Rxndata(i)%EList = 0
   end forall


   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(10)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(13)))

   do i=1, M

      Call check(NF90_get_var(fileunit,datapointer(10),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i, Model/), count = (/Rxndata(i)%SListLen, 1, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(11),Rxndata(i)%c(1:MaxDepList),start = (/1, i, Model/), &
           & count = (/MaxDepList, 1, 1/)))

      Call check(NF90_get_var(fileunit,datapointer(12),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i, Model/), count = (/Rxndata(i)%SListLen, 1, 1/) ))

      Call check(NF90_get_var(fileunit,datapointer(13),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i, Model/), &
           & count = (/Rxndata(i)%DListLen, 1, 1/) ))

   end do


Case (5)

   !TBD

End Select

!Close file
Call check(NF90_close(fileunit))

!Create Reaction-dependent data structures
!Only create these data structures if 1) it's the first model being simulated or 2) the system of reactions changes (ExpType = 3,5)
!That's not being done here for simplicity. For the future ...

!Create Dependency Graph
Call MakeDGraph(DGraph,Rxndata)

!Debugging
!print*, "Max DGraph outdegree = ", maxval(DGraph(:)%ListLen)
!print*, "Min DGraph outdegree = ", minval(DGraph(:)%ListLen)

!Create EList's -> List of reactions/Events that are triggered on execution of another reaction/event
!Rxndata(mu)%EList(i) = The Reaction/event # that is affected (waiting time is produced or destroyed)
!when the mu'th reaction/event has occurred.
!Rxndata(mu)%EStoichList(i) = The # of waiting times produced (positive) or destroyed (negative) when the m'th reaction/event
!has occurred
!The waiting time may be distributed according to a variety of distributions, but only gamma-distributed is included for now.

Call CalcEList(N,M,Rxndata)

!Perform any necessary unit conversions here

!For all gamma-distributed events, take the 'N' value from Rxndata%c(2) and put it into Rxndata%Data
Where (Rxndata%Mtype == 5)
   Rxndata%Data = int(Rxndata%c(2))
End where

!Convert CellGrowthTime from minutes to seconds
CellGrowthTime = CellGrowthTime * 60
CellGrowthTimeSD = CellGrowthTimeSD * 60

Call ConvertKineticParameters(RxnData, Vo, NumPerturbations, PerturbationIDs, PerturbationData)

!Debugging
!	do i=1,M
!	   print*, "mu = ", i
!	   print*, "c = ", Rxndata(i)%c
!	   print*, "DList = ", Rxndata(i)%DList(1:Rxndata(i)%DListLen)
!	   print*, "SList = ", Rxndata(i)%SList
!	   print*, "v = ", Rxndata(i)%v
!		if (Rxndata(i)%EListLen > 0) THEN
!	   print*, "Mtype = ", Rxndata(i)%Mtype
!	   print*, "EList = ", Rxndata(i)%EList
!	   print*, "EStoichList = ", Rxndata(i)%EStoichList
!		end if
!	   print*, "DGraph = ", DGraph(i)%List
!	end do

End Subroutine InputModelData
!-------------------------------------------------------------------------------------------

!****p dataio/Final_DataInput
!NAME
!	Final_DataInput
!PURPOSE
!	This subroutine should be called once after the simulation of all models and before exiting the program.
!	It deallocates all variables that were allocated using dataio subroutines.
!SOURCE
Subroutine Final_DataInput
IMPLICIT NONE

!Deallocates all global variables defined in DataInput module

!***

Call DeAllocateLocalModel
Deallocate(RandSeed)

End Subroutine Final_DataInput
!-------------------------------------------------------------------------------------------

!****p dataio/WriteModelData
!NAME
!	WriteModelData
!PURPOSE
!	This subroutine opens the NetCDF file in write mode and writes all model data to disk.
!	The set of NetCDF variables to be written to is dependent on the ExpType.
!
!	Kinetic constants are converted from mesoscopic to macroscopic units.
!
!SOURCE

Subroutine WriteModelData(ExpTypeIn, ModelIn)
Integer, intent(in) :: ExpTypeIn, ModelIn

!***

!Write Model data to disk -- dependent on ExpType
integer :: fileunit, i, datapointer(14)
Integer, Allocatable :: SplitOnDivisionTemp(:), SaveSpeciesDataTemp(:)
real*8, allocatable :: k_temp(:,:)


allocate(k_temp(M, MaxDepList))
forall (i=1:M)
   k_temp(i,1:MaxDepList) = Rxndata(i)%c(1:MaxDepList)
end forall


!Convert units from mesoscopic back to macroscopic for writing, but only with the temporary variable k_temp
!We don't want to keep converting back and forth any more than necessary, and some exp types are iterative
!
Where (Rxndata%Mtype == 1)
   !Rxndata%c(1) = Rxndata%c(1) / 6.022e23 / Vo
   k_temp(:,1) = k_temp(:,1) / 6.022e23 / Vo
End Where
!
where (Rxndata%Mtype == 3)
   !Rxndata%c(1) = Rxndata%c(1) * Vo * 6.022e23
   k_temp(:,1) = k_temp(:,1) * Vo * 6.022e23
end where
!
where (Rxndata%Mtype == 4)
   !Rxndata%c(1) = Rxndata%c(1) / 2 * Vo * 6.022e23
   k_temp(:,1) = k_temp(:,1) / 2 * Vo * 6.022e23
end where
!
where (Rxndata%Mtype == 6)
   !Rxndata%c(2) = Rxndata%c(2) / Vo / 6.022e23
   k_temp(:,2) = k_temp(:,2) / Vo / 6.022e23
end where
!
Where (Rxndata%Mtype == 7.OR.Rxndata%Mtype == 8.OR.Rxndata%Mtype == 9)
   !Rxndata%c(2) = Rxndata%c(2) / Vo / 6.022e23
   k_temp(:,2) = k_temp(:,2) / Vo / 6.022e23

   !Rxndata%c(3) = Rxndata%c(3) / Vo / 6.022e23
   k_temp(:,3) = k_temp(:,3) / Vo / 6.022e23
End where
!
Where (Rxndata%Mtype == 10)
   !Rxndata%c(2) = Rxndata%c(2) / Vo / 6.022e23
   k_temp(:,2) = k_temp(:,2) / Vo / 6.022e23

   !Rxndata%c(3) = Rxndata%c(3) / Vo / 6.022e23
   k_temp(:,3) = k_temp(:,3) / Vo / 6.022e23

   !Rxndata%c(4) = Rxndata%c(4) / Vo / 6.022e23
   k_temp(:,4) = k_temp(:,4) / Vo / 6.022e23
End where


!Question:  Should all model data be written to disk for all ExpTypes regardless of the repeated saving of the same data?
!Queston:   Or should model data only be written to disk if the data can change between models?


!Open NetCDF file
Call check(NF90_open(path = trim(filename),mode = IOR(NF90_Share,NF90_Write),ncid = fileunit))

Select Case (ExpTypeIn)

Case (1)

   !Single model only (ModelIn must == 1)

    !No NetCDF variables have record dimensions

   if (ModelIn > 1) THEN
      print*, "Warning: This NetCDF file contains only a single model. That model's data is being read."
   end if

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(1)))
   Call check(NF90_put_var(fileunit,datapointer(1),Vo))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(2)))
   Call check(NF90_put_var(fileunit,datapointer(2),CellGrowthTime))


   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(3)))
   Call check(NF90_put_var(fileunit,datapointer(3),CellGrowthTimeSD))

   Allocate(SplitOnDivisionTemp(N))

   where (SplitOnDivision)
      SplitOnDivisionTemp = 1
   elsewhere
      SplitOnDivisionTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(4)))
   Call check(NF90_put_var(fileunit,datapointer(4),SplitOnDivisionTemp, count = (/N/) ))

   Deallocate(SplitOnDivisionTemp)


   Allocate(SaveSpeciesDataTemp(N))

   where (SaveSpeciesData)
      SaveSpeciesDataTemp = 1
   elsewhere
      SaveSpeciesDataTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5)))
   Call check(NF90_put_var(fileunit,datapointer(5),SaveSpeciesDataTemp, count = (/N/) ))

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(6)))
   Call check(NF90_put_var(fileunit,datapointer(6),Xo, count = (/N/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(7)))
   Call check(NF90_put_var(fileunit,datapointer(7),Rxndata(:)%Data, count = (/M/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(8)))
   Call check(NF90_put_var(fileunit,datapointer(8),Rxndata(:)%Mtype, count = (/ M /) ))


   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(9)))
   Call check(NF90_put_var(fileunit,datapointer(9),Rxndata(:)%DListLen, count = (/M/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(10)))
   Call check(NF90_put_var(fileunit,datapointer(10),Rxndata(:)%SListLen,count = (/ M /)))

   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(13)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(14)))

   do i=1, M

      Call check(NF90_put_var(fileunit,datapointer(11),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      !Call check(NF90_put_var(fileunit,datapointer(12),Rxndata(i)%c(1:MaxDepList),start = (/1, i/), count = (/MaxDepList, 1/)))
      Call check(NF90_put_var(fileunit,datapointer(12),k_temp(i,1:MaxDepList),start = (/1, i/), count = (/MaxDepList, 1/)))

      Call check(NF90_put_var(fileunit,datapointer(13),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_put_var(fileunit,datapointer(14),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i/), &
           & count = (/Rxndata(i)%DListLen,1 /) ))

   end do

Case (2)

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(1)))
   Call check(NF90_put_var(fileunit,datapointer(1),Vo))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(2)))
   Call check(NF90_put_var(fileunit,datapointer(2),CellGrowthTime))


   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(3)))
   Call check(NF90_put_var(fileunit,datapointer(3),CellGrowthTimeSD))

   Allocate(SplitOnDivisionTemp(N))

   where (SplitOnDivision)
      SplitOnDivisionTemp = 1
   elsewhere
      SplitOnDivisionTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(4)))
   Call check(NF90_put_var(fileunit,datapointer(4),SplitOnDivisionTemp, count = (/N/) ))

   Deallocate(SplitOnDivisionTemp)


   Allocate(SaveSpeciesDataTemp(N))

   where (SaveSpeciesData)
      SaveSpeciesDataTemp = 1
   elsewhere
      SaveSpeciesDataTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5)))
   Call check(NF90_put_var(fileunit,datapointer(5),SaveSpeciesDataTemp, count = (/N/) ))

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(4)))
   Call check(NF90_get_var(fileunit,datapointer(4),Xo, start = (/1, ModelIn /), count = (/N, 1/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(7)))
   Call check(NF90_put_var(fileunit,datapointer(7),Rxndata(:)%Data, count = (/M/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(8)))
   Call check(NF90_put_var(fileunit,datapointer(8),Rxndata(:)%Mtype, count = (/ M /) ))


   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(9)))
   Call check(NF90_put_var(fileunit,datapointer(9),Rxndata(:)%DListLen, count = (/M/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(10)))
   Call check(NF90_put_var(fileunit,datapointer(10),Rxndata(:)%SListLen,count = (/ M /)))

   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(13)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(14)))

   do i=1, M

      Call check(NF90_put_var(fileunit,datapointer(10),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      !Call check(NF90_put_var(fileunit,datapointer(11),Rxndata(i)%c(1:MaxDepList),start = (/1, i, ModelIn/), &
      !     & count = (/MaxDepList, 1, 1/)))
      Call check(NF90_put_var(fileunit,datapointer(11),k_temp(i,1:MaxDepList),start = (/1, i, ModelIn/), &
           & count = (/MaxDepList, 1, 1/)))

      Call check(NF90_put_var(fileunit,datapointer(12),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_put_var(fileunit,datapointer(13),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i/), &
           & count = (/Rxndata(i)%DListLen,1 /) ))

   end do

Case (3)

!Read in multi-model related variables
   Call check(NF90_inq_varid(fileunit,'LastModel',datapointer(6)))
   Call check(NF90_put_var(fileunit,datapointer(6),LastModel))

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(1)))
   Call check(NF90_put_var(fileunit,datapointer(1),Vo))

   Call check(NF90_inq_varid(fileunit,'MaxNumModels',datapointer(7)))
   Call check(NF90_put_var(fileunit,datapointer(7),MaxNumModels))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(2)))
   Call check(NF90_put_var(fileunit,datapointer(2),CellGrowthTime))

   Call check(NF90_inq_varid(fileunit,'NumModels',datapointer(1)))
   Call check(NF90_put_var(fileunit,datapointer(1),NumModels))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(3)))
   Call check(NF90_put_var(fileunit,datapointer(3),CellGrowthTimeSD))

   Allocate(SplitOnDivisionTemp(N))

   where (SplitOnDivision)
      SplitOnDivisionTemp = 1
   elsewhere
      SplitOnDivisionTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(4)))
   Call check(NF90_put_var(fileunit,datapointer(4),SplitOnDivisionTemp, count = (/N/) ))

   Deallocate(SplitOnDivisionTemp)


   Allocate(SaveSpeciesDataTemp(N))

   where (SaveSpeciesData)
      SaveSpeciesDataTemp = 1
   elsewhere
      SaveSpeciesDataTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5)))
   Call check(NF90_put_var(fileunit,datapointer(5),SaveSpeciesDataTemp, count = (/N/) ))

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(4)))
   Call check(NF90_put_var(fileunit,datapointer(4),Xo, start = (/1, ModelIn /), count = (/N, 1/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(7)))
   Call check(NF90_put_var(fileunit,datapointer(7),Rxndata(:)%Data, count = (/M/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(8)))
   Call check(NF90_put_var(fileunit,datapointer(8),Rxndata(:)%Mtype, count = (/ M /) ))


   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(9)))
   Call check(NF90_put_var(fileunit,datapointer(9),Rxndata(:)%DListLen, count = (/M/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(10)))
   Call check(NF90_put_var(fileunit,datapointer(10),Rxndata(:)%SListLen,count = (/ M /)))

   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(13)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(14)))

   do i=1, M

      Call check(NF90_put_var(fileunit,datapointer(11),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      !Call check(NF90_put_var(fileunit,datapointer(12),Rxndata(i)%c(1:MaxDepList),start = (/1, i, ModelIn/), &
      !     & count = (/MaxDepList, 1, 1/)))
      Call check(NF90_put_var(fileunit,datapointer(12),k_temp(i,1:MaxDepList),start = (/1, i, ModelIn/), &
           & count = (/MaxDepList, 1, 1/)))

      Call check(NF90_put_var(fileunit,datapointer(13),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i/), count = (/Rxndata(i)%SListLen, 1/) ))

      Call check(NF90_put_var(fileunit,datapointer(14),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i/), &
           & count = (/Rxndata(i)%DListLen,1 /) ))

   end do

Case (4)

    !The following NetCDF variables have record dimensions: SpeciesIC, SpeciesSplitOnDivision, Reaction_Rate_Laws,
   !Reaction_DListLen, Reaction_StoichListLen, Reaction_DepList, Reaction_StoichCoeff, Reaction_StoichSpecies,
   !Reaction_Rate_Constants, Reaction_OptionalData

   Call check(NF90_inq_varid(fileunit,'Volume',datapointer(1)))
   Call check(NF90_put_var(fileunit,datapointer(1),Vo))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTime',datapointer(2)))
   Call check(NF90_put_var(fileunit,datapointer(2),CellGrowthTime))

   Call check(NF90_inq_varid(fileunit,'CellGrowthTimeSD',datapointer(3)))
   Call check(NF90_put_var(fileunit,datapointer(3),CellGrowthTimeSD))

   Call check(NF90_inq_varid(fileunit,'SpeciesIC',datapointer(4)))
   Call check(NF90_put_var(fileunit,datapointer(4),Xo, start = (/1, ModelIn /), count = (/N, 1/) ))


  Allocate(SplitOnDivisionTemp(N))

   where (SplitOnDivision)
      SplitOnDivisionTemp = 1
   elsewhere
      SplitOnDivisionTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SpeciesSplitOnDivision',datapointer(5)))
   Call check(NF90_put_var(fileunit,datapointer(5),SplitOnDivisionTemp, start = (/1, ModelIn/), count = (/N, 1/) ))

   Deallocate(SplitOnDivisionTemp)

   Allocate(SaveSpeciesDataTemp(N))

   where (SaveSpeciesData)
      SaveSpeciesDataTemp = 1
   elsewhere
      SaveSpeciesDataTemp = 0
   end where

   Call check(NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(14)))
   Call check(NF90_put_var(fileunit,datapointer(14),SaveSpeciesDataTemp, start = (/1, ModelIn/), count = (/N, 1/) ))

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_varid(fileunit,'Reaction_OptionalData',datapointer(6)))
   Call check(NF90_put_var(fileunit,datapointer(6),Rxndata(:)%Data, start = (/1, ModelIn /), count = (/M, 1/) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Laws',datapointer(7)))
   Call check(NF90_put_var(fileunit,datapointer(7),Rxndata(:)%Mtype, start = (/1, ModelIn/), count = (/ M, 1/) ))

   !Populate Rxndata(:)%DListLen - Length of dependent species list
   Call check(NF90_inq_varid(fileunit,'Reaction_DListLen',datapointer(8)))
   Call check(NF90_put_var(fileunit,datapointer(8),Rxndata(:)%DListLen, start = (/1, ModelIn/), count = (/M, 1/) ))

   !Populate Rxndata(:)%SListLen - Length of stoichiometric species list
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichListLen',datapointer(9)))
   Call check(NF90_put_var(fileunit,datapointer(9),Rxndata(:)%SListLen, start = (/1, ModelIn/), count = (/ M, 1 /) ))

   Call check(NF90_inq_varid(fileunit,'Reaction_StoichCoeff',datapointer(10)))
   Call check(NF90_inq_varid(fileunit,'Reaction_Rate_Constants',datapointer(11)))
   Call check(NF90_inq_varid(fileunit,'Reaction_StoichSpecies',datapointer(12)))
   Call check(NF90_inq_varid(fileunit,'Reaction_DepList',datapointer(13)))

   do i=1, M

      Call check(NF90_put_var(fileunit,datapointer(10),Rxndata(i)%v(1:Rxndata(i)%SListLen), &
           & start = (/1, i, ModelIn/), count = (/Rxndata(i)%SListLen, 1, 1/) ))

      !Call check(NF90_put_var(fileunit,datapointer(11),Rxndata(i)%c(1:MaxDepList),start = (/1, i, ModelIn/), &
      !     & count = (/MaxDepList, 1, 1/)))
      Call check(NF90_put_var(fileunit,datapointer(11),k_temp(i,1:MaxDepList),start = (/1, i, ModelIn/), &
           & count = (/MaxDepList, 1, 1/)))

      Call check(NF90_put_var(fileunit,datapointer(12),Rxndata(i)%SList(1:Rxndata(i)%SListLen), &
           & start = (/1, i, ModelIn/), count = (/Rxndata(i)%SListLen, 1, 1/) ))

      Call check(NF90_put_var(fileunit,datapointer(13),Rxndata(i)%DList(1:Rxndata(i)%DListLen), start = (/1, i, ModelIn/), &
           & count = (/Rxndata(i)%DListLen, 1, 1/) ))

   end do

Case (5)
   !TBD

End Select

deallocate(k_temp)

Call check(NF90_close(fileunit))

End Subroutine WriteModelData
!-------------------------------------------------------------------------------------------

!****p dataio/WriteStateData
!NAME
!	WriteStateData
!PURPOSE
!	This subroutine opens the NetCDF file and writes the solution data to disk.
!	The set of solution variables written to disk is dependent on the ExpType.
!
!	The arguments are the ExpType, the Trial #, the Model #, and the state and time
!	solution variables.
!
!SOURCE

Subroutine WriteStateData(ExpTypeIn, TrialIn, ModelIn, State, Time)
IMPLICIT NONE

Integer, intent(in) :: ExpTypeIn, TrialIn, ModelIn
Real*8, intent(in) :: State(:,:), Time(:)

!***

Real*8, Allocatable :: StateTemp(:,:)
integer :: XStateID, TimeID, NumSavedSpecies, fileunit, i

!Write state data to disk -- dependent on ExpType

!Open NetCDF file
Call check(NF90_open(trim(filename),IOR(NF90_Share,NF90_Write),fileunit))

Call check(NF90_inq_varid(fileunit,'State',XstateID))

Select Case (ExpTypeIn)

Case (1)

   NumSavedSpecies = count(SaveSpeciesData)
   Allocate(StateTemp(TimePoints,NumSavedSpecies))

   forall (i=1:TimePoints)
      StateTemp(i,:) = pack(State(i,:),mask = SaveSpeciesData)
   end forall

   !Saving to disk
   Call check(NF90_put_var(fileunit,XstateID,transpose(StateTemp),start = (/1, 1, TrialIn/), &
        & count = (/NumSavedSpecies, TimePoints, 1/)))

   Deallocate(StateTemp)

   !Time saved the first time.
   if (TrialIn == 1.AND.ModelIn == 1) THEN
      Call check(NF90_inq_varid(fileunit,'Time',TimeID))
      Call check(NF90_put_var(fileunit,TimeID,Time))
   end if

Case (2)

   NumSavedSpecies = count(SaveSpeciesData)
   Allocate(StateTemp(TimePoints,NumSavedSpecies))

   forall (i=1:TimePoints)
      StateTemp(i,:) = pack(State(i,:),SaveSpeciesData)
   end forall

   !Saving to disk
   Call check(NF90_put_var(fileunit,XstateID,transpose(StateTemp),start = (/1, 1, TrialIn, ModelIn/), &
        & count = (/NumSavedSpecies, TimePoints, 1, 1/)))

   Deallocate(StateTemp)


   !Time saved the first time.
   if (TrialIn == 1.AND.ModelIn == 1) THEN
      Call check(NF90_inq_varid(fileunit,'Time',TimeID))
      Call check(NF90_put_var(fileunit,TimeID,Time))
   end if

Case (3)
   NumSavedSpecies = count(SaveSpeciesData)
   Allocate(StateTemp(TimePoints,NumSavedSpecies))
   forall (i=1:TimePoints)
      StateTemp(i,:) = pack(State(i,:),SaveSpeciesData)
   end forall


   !Saving to disk
   Call check(NF90_put_var(fileunit,XstateID,transpose(StateTemp),start = (/1, 1, TrialIn, ModelIn/), &
        & count = (/NumSavedSpecies, TimePoints, 1, 1/)))

   !Time saved the first time.
   if (TrialIn == 1.AND.ModelIn == 1) THEN
      Call check(NF90_inq_varid(fileunit,'Time',TimeID))
      Call check(NF90_put_var(fileunit,TimeID,Time))
   end if

   Deallocate(StateTemp)
Case (4)

   !NumSavedSpecies = count(SaveSpeciesData)
   !Allocate(StateTemp(TimePoints,NumSavedSpecies))
   !Allocate(SaveMask(TimePoints,NumSavedSpecies))

   !forall (i=1:TimePoints)
   !   SaveMask(i,:) = SaveSpeciesData
   !end forall

   !StateTemp = pack(State, SaveMask)

   Allocate(StateTemp(TimePoints, N_Max))

   StateTemp(:,1:N) = State(:,1:N)
   StateTemp(:,N+1:N_Max) = -1.0000000000

   !Saving to disk
   Call check(NF90_put_var(fileunit,XstateID,transpose(StateTemp),start = (/1, 1, TrialIn, ModelIn/), &
        & count = (/N_Max, TimePoints, 1, 1/)))

   Deallocate(StateTemp)

   !Time saved the first time.
   if (TrialIn == 1.AND.ModelIn == 1) THEN
      Call check(NF90_inq_varid(fileunit,'Time',TimeID))
      Call check(NF90_put_var(fileunit,TimeID,Time))
   end if

End Select

Call check(NF90_close(fileunit))

End Subroutine WriteStateData
!---------------------------------------------------------------------

!****p dataio/MakeDGraph
!NAME
!	MakeDGraph
!PURPOSE
!	This subroutine constructs the dependency graph for determining when it is
!	necessary to update the reaction propensities and times.
!SOURCE

pure Subroutine MakeDGraph(DGraph,Rxndata)
IMPLICIT NONE

Type (DGraphType), Dimension(:), intent(out) :: DGraph
Type (RxnDataType), Dimension(:), intent(in) :: RxnData

!***

Logical :: SomeIntersection
integer :: counter, J, I

Do i = 1, M
	counter=0

	Do j = 1, M
	   SomeIntersection = intersect(Rxndata(i)%DList(1:Rxndata(i)%DListLen), Rxndata(j)%DList(1:Rxndata(j)%DListLen)) &
	   & .OR.intersect(Rxndata(i)%DList(1:Rxndata(i)%DListLen), Rxndata(j)%SList(1:Rxndata(j)%SListLen)) &
	   & .OR.intersect(Rxndata(i)%SList(1:Rxndata(i)%SListLen), Rxndata(j)%DList(1:Rxndata(j)%DListLen)) &
	   & .OR.intersect(Rxndata(i)%SList(1:Rxndata(i)%SListLen), Rxndata(j)%SList(1:Rxndata(j)%SListLen))

	   if (SomeIntersection) THEN
	      counter=counter + 1
	      DGraph(i)%list(counter)=j
	   end if
	End Do

        DGraph(i)%ListLen = counter
End Do

End Subroutine MakeDGraph
!-----------------------------------------------------------
pure Function intersect(array1,array2)

  IMPLICIT NONE

  integer :: J, counter, size1, size2
  Integer, Dimension(:), intent(in) :: array1, array2
  Logical :: intersect

  intersect=.FALSE.
  counter=0

  size1 = size(array1)
  size2 = size(array2)

  do while ((.NOT.intersect).AND.(counter < size1 ) )

     counter = counter + 1

     do J=1 , size2
        if (array1(counter) == array2(j)) THEN
           intersect=.TRUE.
        end if
     end do

  end do

End Function intersect
!-------------------------------------------------------------------

!****p dataio/CalcEList
!NAME
!	CalcEList
!PURPOSE
!	This subroutine creates the list of special events, such as gamma-distributed reactions,
!	that are created or destroyed when certain species are created or destroyed.
!SOURCE

Subroutine CalcEList(N,M,Rxndata)
IMPLICIT NONE

Integer, intent(in) :: N, M
Type (RxnDataType), Dimension(:), intent(inout) :: RxnData

!***

Integer ::  i, j, k, counter, Eventcounter
Integer :: SpeciesEvents(M), Events(M)

!Find all reactions that produce molecules that then transition through gamma-distributed events
!Under these reactions, add the gamma-distributed event that may occur within EList
!Under EStoichList, enter the stoichiometry of the reaction affecting the species involved in the gamma-distributed event

Eventcounter = 0

do i=1,M
   if (Rxndata(i)%MType == 5) THEN
      do j=1,Rxndata(i)%SListLen
         if (Rxndata(i)%v(j) < 0) THEN
            Eventcounter = Eventcounter + 1
            SpeciesEvents(Eventcounter) = Rxndata(i)%SList(j)
            Events(Eventcounter) = i
         end if
      end do
   end if
end do

do i=1,M
   Rxndata(i)%EList = 0
   Rxndata(i)%EStoichList = 0
   counter = 0

   do j=1,Rxndata(i)%SListLen
      do k=1,Eventcounter
         if (Rxndata(i)%SList(j) == SpeciesEvents(k).AND.i /= Events(k)) THEN

            counter = counter + 1
            Rxndata(i)%EList(counter) = Events(k)
            Rxndata(i)%EStoichList(counter) = Rxndata(i)%v(j)
         end if
      end do
   end do

   Rxndata(i)%EListLen = counter

end do


End Subroutine CalcEList
!------------------------------------------------------------------

!****p dataio/CheckAndDefineVariables
!NAME
!	CheckAndDefineVariables
!PURPOSE
!	This subroutine opens up the NetCDF file, reads in enough data to determine
!	the necessary size of the NetCDF solution variables, checks whether these
!	variables exist, and defines them if they do not.
!SOURCE

Subroutine CheckAndDefineVariables(error, errormsg)
IMPLICIT NONE
!check the file for inconsistencies, errors, missing variables, etc. Define solution variables if they don't exist.

integer, intent(out) :: error
Character*(*), intent(out) :: errormsg

!***

integer :: fileunit, datapointer(10), i, status
integer :: NdimID, MdimID, TrialsID, TimepointsID, XstateID, TimeID, ProbDistrID, ProbDistrRangeID, DataLenID
integer :: CDFMaxDepList, CDFMaxDepListID, NumBinsID, ModelsID, NumSavedSpeciesID, NumPerturbationsID

integer :: DataWritten, OldFillMode, NumSavedSpecies, DataLen
Integer, Allocatable :: SaveSpeciesDataTemp(:)

! error is intent(out) and was only ever assigned on the ten failure paths
! below, never on the success path. The caller does set it to zero first, but
! intent(out) promises the callee will define it, so at -O1 and above gfortran
! deletes that store as dead code -- as the standard permits -- and the
! `if (error == 1)` test in the caller then read an undefined stack slot.
! At -O0 the store survives, which is why this looked fine there; at -O1 and
! above roughly half of all runs saw a non-zero value, printed the equally
! uninitialized 256-character errormsg as binary garbage, and stopped without
! simulating anything -- while still exiting 0.
 error = 0
 errormsg = ''

 status = NF90_open(path = trim(filename),mode = IOR(NF90_Write,NF90_Share),ncid = fileunit)

 if (status /= NF90_NoErr) THEN

    error = 1
    errormsg = 'Error: Can not open NetCDF file.'
    return

 end if

 status = NF90_get_att(fileunit,NF90_Global,"Data_Written",DataWritten)

 if (status == NF90_NoErr.AND.DataWritten == 1.AND.OverWriteSolution == 0) THEN
    error=1
    errormsg = "Error: This NetCDF file has previously written output data. Use the '-OV' argument to overwrite"
    return
 end if

 status = NF90_inq_varid(fileunit,'ExpType',datapointer(4))
 if (status /= NF90_NoErr) THEN
    print*, "Warning. NetCDF file does not contain an Experiment Type (ExpType)."
    print*, "Defaulting to single model NetCDF format."
    ExpType = 1
 else
    Call check(NF90_get_var(fileunit,datapointer(4),ExpType))
 end if


 !Read in all necessary data to define any non-defined variables - dependent on ExpType
 Select Case (ExpType)

 Case (1)

    Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
    Call check(NF90_get_var(fileunit,datapointer(1),TStart))

    Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
    Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

    Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
    Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

    Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
    Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

    NumModels = 1

    Call check(NF90_inq_dimid(fileunit,'NumSpecies',NdimID))
    Call check(NF90_inquire_dimension(fileunit,NdimID,len = N))

    N_Max = N

    Allocate(SaveSpeciesDataTemp(N_Max))

    status = NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5))
    if (status == NF90_NoErr) THEN
       Call check(NF90_get_var(fileunit,datapointer(5),SaveSpeciesDataTemp))
    else
       SaveSpeciesDataTemp = 1
    end if

    NumSavedSpecies = count(SaveSpeciesDataTemp == 1)

    Deallocate(SaveSpeciesDataTemp)

    Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
    Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

    if (CDFMaxDepList /= MaxDepList) THEN
       error=1
       errormsg = "Error: The maximum number of kinetic parameters in the NetCDF file is different from the source code's maximum"
       return
    end if

    !Start defining new variables, where necessary

    !Turn off fill for faster variable definition - Make sure you write all data that is defined
    Call check(NF90_Set_Fill(fileunit, NF90_NoFill, OldFillMode))
    Call check(NF90_redef(fileunit))

    !Define NumTimePoints dimension, if necessary
    status = NF90_inq_dimid(fileunit,'NumTimePoints',TimePointsID)
    if (status /= NF90_NoErr) THEN
       if ((SaveTime > 0.0).AND.(SaveTime <= (TEnd - TStart))) THEN
          TimePoints = int((TEnd - TStart) / SaveTime) + 1

          Call check(NF90_def_dim(fileunit,'NumTimePoints',TimePoints,TimePointsID))

       else

          error = 1
          errormsg = "Error: SaveTime value is invalid."
          return
       end if
    else

       Call check(NF90_inquire_dimension(fileunit,TimePointsID, len = TimePoints))

    end if

    !Define TimePoints variable, if necessary

    status = NF90_inq_varid(fileunit,'Time',TimeID)
    if (status /= NF90_noerr) THEN
       Call check(NF90_def_var(fileunit,'Time', NF90_double, (/TimePointsID/), TimeID))
    end if

    !If total number of saved species is not the total number of species, then define a new dimension
    !Then check if the solution variables exist or not. If they don't exist, define them.

    !If the total number of saved species is equal to the total number of species, then a new dimension is not needed.
    !Then check if the solution variables exist or not. If they don't eixst, define them.

    if (NumSavedSpecies /= N) THEN

       status = NF90_inq_dimid(fileunit,'NumSaveSpecies',NumSavedSpeciesID)
       if (status /= NF90_NoErr) THEN
          Call check(NF90_def_dim(fileunit,'NumSaveSpecies', NumSavedSpecies, NumSavedSpeciesID))
       end if

       !Single model - Solution is State variable: 3D (NumSavedSpecies, NumTimePoints, NumTrials)

       status = NF90_inq_varid(fileunit,'State',XstateID)
       if (status /= NF90_noerr) THEN
          print*, "Defining State variable in NetCDF file. This may take some time."
          print*, "Solution data size = ", Trials, "x", TimePoints, "x", NumSavedSpecies

          Call check(NF90_def_var(fileunit,'State', NF90_double, (/ NumSavedSpeciesID, TimePointsID, TrialsID/), &
               & XstateID))

       end if

    else

       !Single model - Solution is State variable: 3D (N, NumTimePoints, NumTrials)

       status = NF90_inq_varid(fileunit,'State',XstateID)
       if (status /= NF90_noerr) THEN
          print*, "Defining State variable in NetCDF file. This may take some time."
          print*, "Solution data size = ", Trials, "x", TimePoints, "x", N

          Call check(NF90_def_var(fileunit,'State', NF90_double, (/ NdimID, TimePointsID, TrialsID/), XstateID))

       end if
    end if

 Case (2)

    Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
    Call check(NF90_get_var(fileunit,datapointer(1),TStart))

    Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
    Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

    Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
    Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

    Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
    Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

    Call check(NF90_inq_dimid(fileunit,'NumModels',ModelsID))
    Call check(NF90_inquire_dimension(fileunit,ModelsID,len = NumModels))

    Call check(NF90_inq_dimid(fileunit,'NumSpecies',NdimID))
    Call check(NF90_inquire_dimension(fileunit,NdimID,len = N))

    N_Max = N

    Allocate(SaveSpeciesDataTemp(N_Max))

    status = NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5))
    if (status == NF90_NoErr) THEN
       Call check(NF90_get_var(fileunit,datapointer(5),SaveSpeciesDataTemp))
    else
       SaveSpeciesDataTemp = 1
    end if

    NumSavedSpecies = count(SaveSpeciesDataTemp == 1)

    Deallocate(SaveSpeciesDataTemp)

    Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
    Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

    if (CDFMaxDepList /= MaxDepList) THEN
       error=1
       errormsg = "Error: The maximum number of kinetic parameters in the NetCDF file is different from the source code's maximum"
       return
    end if

    !Start defining new variables, where necessary

    !Turn off fill for faster variable definition - Make sure you write all data that is defined
    Call check(NF90_Set_Fill(fileunit, NF90_NoFill, OldFillMode))
    Call check(NF90_redef(fileunit))


    !Define NumTimePoints dimension, if necessary
    status = NF90_inq_dimid(fileunit,'NumTimePoints',TimePointsID)
    if (status /= NF90_NoErr) THEN
       if ((SaveTime > 0.0).AND.(SaveTime <= (TEnd - TStart))) THEN
          TimePoints = int((TEnd - TStart) / SaveTime) + 1

          Call check(NF90_def_dim(fileunit,'NumTimePoints',TimePoints,TimePointsID))

       else

          error = 1
          errormsg = "Error: SaveTime value is invalid."
          return
       end if
    else

       Call check(NF90_inquire_dimension(fileunit,TimePointsID, len = TimePoints))

    end if

    !Define TimePoints variable, if necessary

    status = NF90_inq_varid(fileunit,'Time',TimeID)
    if (status /= NF90_noerr) THEN
       Call check(NF90_def_var(fileunit,'Time', NF90_double, (/TimePointsID/), TimeID))
    end if

    !Then check if the solution variables exist or not. If they don't exist, define them.
    !If the total number of saved species is equal to the total number of species, then a new dimension is not needed.
    !Then check if the solution variables exist or not. If they don't eixst, define them.

    if (NumSavedSpecies /= N) THEN

       status = NF90_inq_dimid(fileunit,'NumSaveSpecies',NumSavedSpeciesID)
       if (status /= NF90_NoErr) THEN
          Call check(NF90_def_dim(fileunit,'NumSaveSpecies', NumSavedSpecies, NumSavedSpeciesID))
       end if

       !Multiple models - Solution is State variable: 4D (NumSaveSpecies, NumTimePoints, NumTrials, NumModels)

       status = NF90_inq_varid(fileunit,'State',XstateID)
       if (status /= NF90_noerr) THEN
          print*, "Defining State variable in NetCDF file. This may take some time."
          print*, "Solution data size = ", NumModels, "x", Trials, "x", TimePoints, "x", NumSavedSpecies

          Call check(NF90_def_var(fileunit,'State', NF90_double, (/NumSavedSpeciesID, TimePointsID, TrialsID, ModelsID/), &
               & XstateID))

       end if

    else

       !Multiple models - Solution is State variable: 4D (N, NumTimePoints, NumTrials, NumModels)

       status = NF90_inq_varid(fileunit,'State',XstateID)
       if (status /= NF90_noerr) THEN
          print*, "Defining State variable in NetCDF file. This may take some time."
          print*, "Solution data size = ", NumModels, "x", Trials, "x", TimePoints, "x", N


          Call check(NF90_def_var(fileunit,'State', NF90_double, (/ NdimID, TimePointsID, TrialsID, ModelsID/), XstateID))

       end if
    end if

 Case (3)

    Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
    Call check(NF90_get_var(fileunit,datapointer(1),TStart))

    Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
    Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

    Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
    Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

    Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
    Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

    Call check(NF90_inq_dimid(fileunit,'NumModels',ModelsID))
    Call check(NF90_inquire_dimension(fileunit,ModelsID,len = NumModels))

    Call check(NF90_inq_dimid(fileunit,'NumSpecies',NdimID))
    Call check(NF90_inquire_dimension(fileunit,NdimID,len = N))

    N_Max = N

   Allocate(SaveSpeciesDataTemp(N_Max))

   status = NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5))
   if (status == NF90_NoErr) THEN
      Call check(NF90_get_var(fileunit,datapointer(5),SaveSpeciesDataTemp))
   else
      SaveSpeciesDataTemp = 1
   end if

   NumSavedSpecies = count(SaveSpeciesDataTemp == 1)

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
   Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

   if (CDFMaxDepList /= MaxDepList) THEN
      error=1
      errormsg = "Error: The maximum number of kinetic parameters in the NetCDF file is different from the source code's maximum"
      return
   end if

   !Start defining new variables, where necessary

   !Turn off fill for faster variable definition - Make sure you write all data that is defined
   Call check(NF90_Set_Fill(fileunit, NF90_NoFill, OldFillMode))
   Call check(NF90_redef(fileunit))


   !Define NumTimePoints dimension, if necessary
   status = NF90_inq_dimid(fileunit,'NumTimePoints',TimePointsID)
   if (status /= NF90_NoErr) THEN
      if ((SaveTime > 0.0).AND.(SaveTime <= (TEnd - TStart))) THEN
         TimePoints = int((TEnd - TStart) / SaveTime) + 1

         Call check(NF90_def_dim(fileunit,'NumTimePoints',TimePoints,TimePointsID))

      else

         error = 1
         errormsg = "Error: SaveTime value is invalid."
         return
      end if
   else

      Call check(NF90_inquire_dimension(fileunit,TimePointsID, len = TimePoints))

   end if

   !Define TimePoints variable, if necessary

   status = NF90_inq_varid(fileunit,'Time',TimeID)
   if (status /= NF90_noerr) THEN
      Call check(NF90_def_var(fileunit,'Time', NF90_double, (/TimePointsID/), TimeID))
   end if

    !If total number of saved species is not the total number of species, then define a new dimension
    !Then check if the solution variables exist or not. If they don't exist, define them.

    !If the total number of saved species is equal to the total number of species, then a new dimension is not needed.
    !Then check if the solution variables exist or not. If they don't eixst, define them.

    if (NumSavedSpecies /= N) THEN

       status = NF90_inq_dimid(fileunit,'NumSaveSpecies',NumSavedSpeciesID)
       if (status /= NF90_NoErr) THEN
          Call check(NF90_def_dim(fileunit,'NumSaveSpecies', NumSavedSpecies, NumSavedSpeciesID))
       end if

       !Multiple models - Solution is State variable: 4D (NumSaveSpecies, NumTimePoints, NumTrials, NumModels)

       status = NF90_inq_varid(fileunit,'State',XstateID)
       if (status /= NF90_noerr) THEN
          print*, "Defining State variable in NetCDF file. This may take some time."
          print*, "Solution data size = ", NumModels, "x", Trials, "x", TimePoints, "x", NumSavedSpecies

          Call check(NF90_def_var(fileunit,'State', NF90_double, (/NumSavedSpeciesID, TimePointsID, TrialsID, ModelsID/), &
               & XstateID))

       end if

    else

       !Multiple models - Solution is State variable: 4D (N, NumTimePoints, NumTrials, NumModels)

       status = NF90_inq_varid(fileunit,'State',XstateID)
       if (status /= NF90_noerr) THEN
          print*, "Defining State variable in NetCDF file. This may take some time."
          print*, "Solution data size = ", NumModels, "x", Trials, "x", TimePoints, "x", N


          Call check(NF90_def_var(fileunit,'State', NF90_double, (/NdimID, TimePointsID, TrialsID, ModelsID/), XstateID))

       end if

    end if

Case (4)

   Call check(NF90_inq_varid(fileunit,'TStart',datapointer(1)))
   Call check(NF90_get_var(fileunit,datapointer(1),TStart))

   Call check(NF90_inq_varid(fileunit,'TEnd',datapointer(2)))
   Call check(NF90_get_var(fileunit,datapointer(2),TEnd))

   Call check(NF90_inq_varid(fileunit,'SaveTime',datapointer(3)))
   Call check(NF90_get_var(fileunit,datapointer(3),SaveTime))

   Call check(NF90_inq_dimid(fileunit,'NumTrials',TrialsID))
   Call check(NF90_inquire_dimension(fileunit,TrialsID,len = Trials))

   Call check(NF90_inq_dimid(fileunit,'NumModels',ModelsID))
   Call check(NF90_inquire_dimension(fileunit,ModelsID,len = NumModels))

   Call check(NF90_inq_dimid(fileunit,'MaxNumSpecies',NdimID))
   Call check(NF90_inquire_dimension(fileunit,NdimID,len = N_Max))


   !SaveSpeciesData for ExpType = 4 should be N_Max x NumModels. Will fix later.
   Allocate(SaveSpeciesDataTemp(N_Max))

   status = NF90_inq_varid(fileunit,'SaveSpeciesData',datapointer(5))
   if (status == NF90_NoErr) THEN
      Call check(NF90_get_var(fileunit,datapointer(5),SaveSpeciesDataTemp))
   else
      SaveSpeciesDataTemp = 1
   end if

   NumSavedSpecies = count(SaveSpeciesDataTemp == 1)

   Deallocate(SaveSpeciesDataTemp)

   Call check(NF90_inq_dimid(fileunit,'NumMaxDepList',CDFMaxDepListID))
   Call check(NF90_inquire_dimension(fileunit,CDFMaxDepListID,len = CDFMaxDepList))

   if (CDFMaxDepList /= MaxDepList) THEN
      error=1
      errormsg = "Error: The maximum number of kinetic parameters in the NetCDF file is different from the source code's maximum"
      return
   end if

   !Start defining new variables, where necessary

   !Turn off fill for faster variable definition - Make sure you write all data that is defined
   Call check(NF90_Set_Fill(fileunit, NF90_NoFill, OldFillMode))
   Call check(NF90_redef(fileunit))

   !Define NumTimePoints dimension, if necessary
   status = NF90_inq_dimid(fileunit,'NumTimePoints',TimePointsID)
   if (status /= NF90_NoErr) THEN
      if ((SaveTime > 0.0).AND.(SaveTime <= (TEnd - TStart))) THEN
         TimePoints = int((TEnd - TStart) / SaveTime) + 1

         Call check(NF90_def_dim(fileunit,'NumTimePoints',TimePoints,TimePointsID))

      else

         error = 1
         errormsg = "Error: SaveTime value is invalid."
         return
      end if
   else

      Call check(NF90_inquire_dimension(fileunit,TimePointsID, len = TimePoints))

   end if

   !Define TimePoints variable, if necessary

   status = NF90_inq_varid(fileunit,'Time',TimeID)
   if (status /= NF90_noerr) THEN
      Call check(NF90_def_var(fileunit,'Time', NF90_double, (/TimePointsID/), TimeID))
   end if

   !For ExpType = 4, always save N_Max species to disk. Number of real species may change between models.

   !Multiple models - Solution is State variable: 4D (N_Max, NumTimePoints, NumTrials, Record Dimension)

   status = NF90_inq_varid(fileunit,'State',XstateID)
   if (status /= NF90_noerr) THEN
      print*, "Defining State variable in NetCDF file. This may take some time."
      print*, "Solution data size = ", NumModels, "x", Trials, "x", TimePoints, "x", N_Max


      Call check(NF90_def_var(fileunit,'State', NF90_double, (/NdimID, TimePointsID, TrialsID, ModelsID/), XstateID))

   end if

Case (5)
   !TBD

End Select

Call check(NF90_enddef(fileunit))

Call check(NF90_close(fileunit))

End Subroutine CheckAndDefineVariables
!-------------------------------------------------------------------

!****p dataio/PrintModelData
!NAME
!	PrintModelData
!PURPOSE
!	This subroutine prints out interesting information about the model(s).
!
!SOURCE

Subroutine PrintModelData
IMPLICIT NONE

!***

 print*, "Filename: ", trim(filename)
 print*, "Number of Reactions: ", M_Max, ". Number of Species: ", N_Max
 print*, "Experimental Type: ", ExpType, ". Number of System Perturbations: ", NumPerturbations
 print*, "Number of Models: ", NumModels, ". Number of Trials: ", Trials
 print*, "Start Time: ", TStart, ". End Time: ", TEnd, ". Number of Timepoints: ", floor((TEnd - TStart) / SaveTime) + 1


End Subroutine PrintModelData
!-------------------------------------------------------------------

!****p dataio/check
!NAME
!	check
!PURPOSE
!	A convienant function that converts a NetCDF error message into an error string.
!SOURCE

Subroutine check(status)
IMPLICIT NONE

!***
    integer, intent (in) :: status

    If (status /= NF90_noerr) Then
      print *, trim(NF90_strerror(status))
    end if
  end subroutine check
!-------------------------------------------------------------------
End module DataIO
