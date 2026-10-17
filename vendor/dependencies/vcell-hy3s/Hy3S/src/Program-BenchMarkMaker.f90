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


Program BenchMarkMaker
USE DataIO
USE GlobalVariables
USE NETCDF

IMPLICIT NONE

Integer :: datapointer(25), TrialsID, NdimID, MdimID, ModelsID, MaxDepListID, TimePointsID, StringLenID, MaxStoichListID
Integer :: Rs, Rf, i, NumCLParameters, fileunit, counter, fastcounter, StringLen
Real*8 :: CLParameters(6), kfastconstant(10), kslowconstant(10), status

Real*8, Allocatable :: k(:)
Integer, Allocatable :: SaveSpeciesDataInt(:), SpeciesSplitOnDivisionInt(:)
Character(len = 36) :: datestr

!Makes a large-scale benchmark model --> defines NetCDF file
!Syntax: BenchMarkMaker <filename> <# Fast Reactions> <# Slow Reactions> [<# Trials>]

!Parse command line
Call ParseCLA(NumCLParameters, CLParameters)

Select Case (NumCLParameters)

Case (0:1)
   print*, "Syntax is:"
   print*, "BenchMarkMaker <filename> <# Fast Reactions> <# Slow Reactions> [<# Trials>]"
   print*, "Program is stopping."
   stop

Case (2)
   Rf = floor(CLParameters(1))
   Rs = floor(CLParameters(2))
   Trials = 1
Case (3)
   Rf = floor(CLParameters(1))
   Rs = floor(CLParameters(2))
   Trials = floor(CLParameters(3))
   
End Select

print*, "Creating a large-scale benchmark model with ", Rf, " fast reactions, ", Rs," slow reactions, and ", Trials, " Trials."

 !Set total number of reactions and species
M = Rf + Rs
N = 3 * Rf + 2 * Rs

 !Set time-related variables
TStart = 0.000
TEnd = 200.000
SaveTime = 10.000
TimePoints = floor( (TEnd - TStart) / SaveTime) + 1
 
 !Set simulation-related variables
ExpType = 1
NumModels = 1
LastModel = 0
LastTrial = 0
StringLen = 72
 
 
 !Set volume/cell replication related variables
Vo = 1e-15
CellGrowthTime = 0.000000
CellGrowthTimeSD = 0.000000

!Determine kinetic constants of fast and slow reactions
Allocate(k(M))

!Round robin of selected kinetic constants for both fast and slow reactions
!Remember: the classification of fast vs. slow depends on the reaction propensity/rate. Not the kinetic constants.

kfastconstant(1:10) = (/10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0/)
kslowconstant(1:10) = (/10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0/)

kfastconstant(1:3) = (/15.06, 30.11, 60.22/)
kslowconstant(1:2) = (/60.22, 60.22/)

do i=1,Rf
   k(i) = kfastconstant(mod(i,size(kfastconstant))+1)
end do

do i=Rf+1,Rf + Rs
   k(i) = kslowconstant(mod(i,size(kslowconstant))+1)
end do


!Allocate all data structures

Allocate(Rxndata(M))
Allocate(Xo(N))
Allocate(SpeciesSplitOnDivisionInt(N))
Allocate(SaveSpeciesDataInt(N))


!Create fast reactions

counter = 0
do i=1,Rf
   Rxndata(i)%SListLen = 3
   Rxndata(i)%DListLen = 2
   Rxndata(i)%SList(1:3) = (/counter+1, counter+2, counter+3/)
   Rxndata(i)%SList(4:MaxSpeciesList) = 0
   Rxndata(i)%v(1:3) = (/-1, -1, 1/)
   Rxndata(i)%v(4:MaxSpeciesList) = 0
   Rxndata(i)%DList(1:2) = (/counter+1, counter+2/)
   Rxndata(i)%DList(3:MaxDepList) = 0
   Rxndata(i)%c(1) = k(i)
   Rxndata(i)%c(2:MaxDepList) = 0.00
   Rxndata(i)%MType = 3

   Xo(counter+1) = 1e6
   Xo(counter+2) = 1e6
   Xo(counter+3) = 100

   SpeciesSplitOnDivisionInt(counter+1) = 0
   SpeciesSplitOnDivisionInt(counter+2) = 0
   SpeciesSplitOnDivisionInt(counter+3) = 0

   SaveSpeciesDataInt(counter+1) = 1
   SaveSpeciesDataInt(counter+2) = 1
   SaveSpeciesDataInt(counter+3) = 1   
   
   counter = counter + 3
end do

Fastcounter = 1
 
do i=Rf+1, Rf + Rs
   Rxndata(i)%SListLen = 3
   Rxndata(i)%DListLen = 2
   
   Rxndata(i)%SList(1:3) = (/Fastcounter, counter+1, counter+2/)
   Rxndata(i)%SList(4:MaxSpeciesList) = 0
   Rxndata(i)%v(1:3) = (/-1, -1, 1/)
   Rxndata(i)%v(4:MaxSpeciesList) = 0
   Rxndata(i)%DList(1:2) = (/Fastcounter, counter+1/)
   Rxndata(i)%DList(3:MaxDepList) = 0
   Rxndata(i)%c(1) = k(i)
   Rxndata(i)%c(2:MaxDepList) = 0.00
   Rxndata(i)%MType = 3

   Fastcounter = Fastcounter + 3
   if (Fastcounter > 3 * Rf) THEN
      Fastcounter = 1
   end if
     
   Xo(counter+1) = 100
   Xo(counter+2) = 0
  

   SpeciesSplitOnDivisionInt(counter+1) = 0
   SpeciesSplitOnDivisionInt(counter+2) = 0

   SaveSpeciesDataInt(counter+1) = 1
   SaveSpeciesDataInt(counter+2) = 1

   counter = counter + 2
end do



!Save all model data to NetCDF file
status = NF90_create(path = trim(filename),cmode = NF90_Clobber,ncid = fileunit)

!Define all needed dimensions
Call check(NF90_def_dim(fileunit,'NumTrials',Trials,TrialsID))
Call check(NF90_def_dim(fileunit,'NumSpecies',N,NdimID))
Call check(NF90_def_dim(fileunit,'NumReactions',M,MdimID))
Call check(NF90_def_dim(fileunit,'NumModels',NumModels,ModelsID))
Call check(NF90_def_dim(fileunit,'NumMaxDepList',MaxDepList,MaxDepListID))
Call check(NF90_def_dim(fileunit,'NumTimePoints',TimePoints,TimePointsID))
Call check(NF90_def_dim(fileunit,'NumMaxStoichList',MaxSpeciesList,MaxStoichListID))
Call check(NF90_def_dim(fileunit,'StringLen',StringLen,StringLenID))

!Define all needed scalar variables
Call check(NF90_def_var(fileunit,'TStart',NF90_double,datapointer(1)))
Call check(NF90_def_var(fileunit,'TEnd',NF90_Double,datapointer(2)))
Call check(NF90_def_var(fileunit,'SaveTime',NF90_double,datapointer(3)))
Call check(NF90_def_var(fileunit,'LastTrial',NF90_Int,datapointer(4)))
Call check(NF90_def_var(fileunit,'ExpType',NF90_Int,datapointer(5)))
Call check(NF90_def_var(fileunit,'LastModel',NF90_Int,datapointer(6)))
Call check(NF90_def_var(fileunit,'MaxNumModels',NF90_Int,datapointer(7)))
Call check(NF90_def_var(fileunit,'NumModels',NF90_Int,datapointer(8)))
Call check(NF90_def_var(fileunit,'Volume',NF90_double,datapointer(9)))
Call check(NF90_def_var(fileunit,'CellGrowthTime',NF90_double,datapointer(10)))
Call check(NF90_def_var(fileunit,'CellGrowthTimeSD',NF90_double,datapointer(11)))

!Define N-dim variables
Call check(NF90_def_var(fileunit,'SpeciesSplitOnDivision',NF90_Int,(/NdimID/), datapointer(12)))
Call check(NF90_def_var(fileunit,'SaveSpeciesData',NF90_Int,(/NdimID/), datapointer(13)))
Call check(NF90_def_var(fileunit,'SpeciesIC',NF90_Int,(/NdimID/), datapointer(14)))

!Define M-dim variables
Call check(NF90_def_var(fileunit,'Reaction_OptionalData',NF90_Int,(/MdimID/), datapointer(15)))
Call check(NF90_def_var(fileunit,'Reaction_Rate_Laws',NF90_Int,(/MdimID/), datapointer(16)))
Call check(NF90_def_var(fileunit,'Reaction_DListLen',NF90_Int,(/MdimID/), datapointer(17)))
Call check(NF90_def_var(fileunit,'Reaction_StoichListLen',NF90_Int,(/MdimID/), datapointer(18)))

!Define 2D variables
Call check(NF90_def_var(fileunit,'Reaction_StoichCoeff',NF90_Int,(/MaxStoichListID, MdimID/), datapointer(19)))
Call check(NF90_def_var(fileunit,'Reaction_StoichSpecies',NF90_Int,(/MaxStoichListID, MdimID/), datapointer(20)))
Call check(NF90_def_var(fileunit,'Reaction_Rate_Constants',NF90_Double,(/MaxDepListID, MdimID/), datapointer(21)))
Call check(NF90_def_var(fileunit,'Reaction_DepList',NF90_Int,(/MaxDepListID, MdimID/), datapointer(22)))


Call Date_and_Time(date = datestr)

!Define global attributes
Call check(NF90_put_att(fileunit, NF90_Global,'Model_Description', 'Automatically Generated Large-scale Benchmark Model'))
Call check(NF90_put_att(fileunit, NF90_Global,'Data_Written', int(0)))
Call check(NF90_put_att(fileunit, NF90_Global,'Format_Version', '1.3'))
Call check(NF90_put_att(fileunit, NF90_Global,'Creation', trim(datestr)))
Call check(NF90_put_att(fileunit, NF90_Global,'Last_Modified', trim(datestr)))
Call check(NF90_put_att(fileunit, NF90_Global,'Author_Info', 'Benchmark Maker code by Howard Salis'))


!End definition mode
Call check(NF90_enddef(fileunit))

!Put data into variables

!Scalar variables
Call check(NF90_put_var(fileunit,datapointer(1),TStart))
Call check(NF90_put_var(fileunit,datapointer(2),TEnd))
Call check(NF90_put_var(fileunit,datapointer(3),SaveTime))
Call check(NF90_put_var(fileunit,datapointer(4),LastTrial))
Call check(NF90_put_var(fileunit,datapointer(5),ExpType))
Call check(NF90_put_var(fileunit,datapointer(6),LastModel))
Call check(NF90_put_var(fileunit,datapointer(7),MaxNumModels))
Call check(NF90_put_var(fileunit,datapointer(8),NumModels))
Call check(NF90_put_var(fileunit,datapointer(9),Vo))
Call check(NF90_put_var(fileunit,datapointer(10),CellGrowthTime))
Call check(NF90_put_var(fileunit,datapointer(11),CellGrowthTimeSD))

!N-dim variables
Call check(NF90_put_var(fileunit,datapointer(12),SpeciesSplitOnDivisionInt, count = (/N/) ))
Call check(NF90_put_var(fileunit,datapointer(13),SaveSpeciesDataInt, count = (/N/) ))
Call check(NF90_put_var(fileunit,datapointer(14),Xo, count = (/N/) ))

!M-dim variables
Call check(NF90_put_var(fileunit,datapointer(15),Rxndata(:)%Data, count = (/M/) ))
Call check(NF90_put_var(fileunit,datapointer(16),Rxndata(:)%MType, count = (/M/) ))
Call check(NF90_put_var(fileunit,datapointer(17),Rxndata(:)%DListLen, count = (/M/) ))
Call check(NF90_put_var(fileunit,datapointer(18),Rxndata(:)%SListLen, count = (/M/) ))

!2D variables

do i=1,M

   Call check(NF90_put_var(fileunit,datapointer(19),Rxndata(i)%v(:), start = (/1, i/), count = (/MaxSpeciesList, 1/) ))
   Call check(NF90_put_var(fileunit,datapointer(20),Rxndata(i)%SList(:), start = (/1, i/), count = (/MaxSpeciesList, 1/) ))
   Call check(NF90_put_var(fileunit,datapointer(21),Rxndata(i)%c, start = (/1, i/), count = (/MaxDepList, 1/) ))
   Call check(NF90_put_var(fileunit,datapointer(22),Rxndata(i)%DList, start = (/1, i/), count = (/MaxDepList, 1/) ))

end do

!Close the file
Call check(NF90_close(fileunit))

!Deallocate all variables
Deallocate(Rxndata)
Deallocate(SaveSpeciesDataInt)
Deallocate(Xo)
Deallocate(SpeciesSplitOnDivisionInt)
Deallocate(k)

print*, "Done!"

End Program
