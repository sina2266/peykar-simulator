Module GlobalVariables

!Module space for public global variables

!MAX NUMBER OF REACTANTS/DEPENDENTS is set to 6! CHANGE AS NEEDED!
!Max Number of Reactions in Dependency Graph is set to 5500! Change as Needed!!
!Max number of Species affected by a reaction is set to 25! Change as needed!!

Integer, Public, Parameter :: MaxDepList = 6, MaxSpeciesList = 25, MaxDGraphList = 5500

!Additions for v1.1
Real*8 :: TimeLastCellDivision

!Additions for v0.3
Integer, Allocatable, Public :: PerturbationIDs(:,:)
Real*8, Allocatable, Public :: PerturbationData(:,:)
Integer, Public :: DataLen, NumPerturbations

!Additions for v0.2
Integer, Public :: LastTrial, LastModel, NumModels, MaxNumModels, ExpType, N_Max, M_Max
Integer, Public :: OverWriteSolution, NumBins
Real*8, Public :: CellGrowthTimeSD

Integer, Public ::  Trials, TimePoints, N, M
Real*8, Public :: TStart, TEnd, SaveTime, CellGrowthTime, Vo
Logical, Allocatable, Public :: SplitOnDivision(:), SaveSpeciesData(:)

Integer*4, Allocatable, Public :: Xo(:)
Integer, Allocatable, Public :: RandSeed(:)

Character(len=128), Public :: filename

!JMS info
Character(len=72), Public :: jmsUrl, jmsUser, jmsPassword, jmsQueue, jmsTopic, vcellUser
Integer, Public :: simKey, jobIndex, taskID

Type, Public :: RxnDataType
	Real*8 :: c(MaxDepList)
	Integer :: MType, Data, SListLen, DListLen, EListLen
	Integer, Dimension(MaxDepList) :: Dlist, EList, EStoichList
	Integer, Dimension(MaxSpeciesList) :: SList, v		!Stoichiometrix coefficients corresponding to SList
End Type

Type, Public :: DGraphType
        Integer :: ListLen
	Integer, Dimension(MaxDGraphList) :: List
End Type

Type (RxnDataType), Allocatable, Dimension(:), Public  :: RxnData

Type (DGraphType), Allocatable, Dimension(:), Public :: DGraph

! only warn about brown tree rows (see progagators-HyJCMSSS) once per run.
! vcell - gcw 08/2013
Integer  :: brownianTreeWarningCount = 0 

!****v dataio/N
!VARIABLE
!	N is the number of species in the current model.
!***

!****v dataio/M
!	M is the number of reactions in the current model.
!***

!****v dataio/MaxDepList
!VARIABLE
!	MaxDepList is a parameter that stores the maximum number of kinetic parameters of a reaction.
!	If you permanently change this parameter, you'll need to make changes to the GUI.
!	c in RxndataType uses this parameter.
!SOURCE

!***

!****v dataio/MaxSpeciesList
!VARIABLE
!	MaxSpeciesList is a parameter that stores the maximum number of species that
!	may be in the stoichiometry of a reaction.
!
!	If you permanently change this parameter, you'll need to make changes to the GUI.
!	Slist and v in RxnDataType use this parameter.
!***

!****v dataio/MaxDGraphList
!VARIABLE
!	MaxDGraphList is a parameter that stores the maximum number of reactions that
!	require updates of their propensities & times after any reaction occurrence.
!
!	Change this parameter where necessary. No changes to the NetCDF file required.
!***

!****v dataio/DataLen
!VARIABLE
!	DataLen is the length of Data in system perturbation variables.
!***

!****v dataio/NumPerturbations
!VARIABLE
!	NumPerturbations is the number of system perturbations.
!***

!****v dataio/PerturbationIDs
!VARIABLE
!	PerturbationIDs contains integer data for applying system perturbations.
!	2D Allocatable (NumPerturbations x DataLen).
!	Fields Are:
!		...
!***

!****v dataio/PerturbationData
!VARIABLE
!	PerturbationData contains Real*8 data for applying system perturbations.
!	2D Allocatable (NumPerturbations x DataLen)
!	Fields Are:
!		....
!***

!****v dataio/NumModels
!VARIABLE
!	NumModels is the number of models stored in the NetCDF file.
!***

!****v dataio/MaxNumModels
!VARIABLE
!	MaxNumModels is the maximum number of models allowable in the NetCDF file.
!***

!****v dataio/ExpType
!VARIABLE
!	ExpType is the Experiment Type of the NetCDF file.
!	See dataio comment for more information.
!***

!****v dataio/LastTrial
!VARIABLE
!	LastTrial is the last trial to be simulated before an unexpected break in the program.
!***

!****v dataio/LastModel
!VARIABLE
!	LastModel is the last model to be simulated before an unexpected break in the program.
!***

!****v dataio/N_Max
!VARIABLE
!	N_Max is the Maximum number of species allowed in any model stored in a Multiple Model NetCDF file.
!***

!****v dataio/M_Max
!VARIABLE
!	M_Max is the Maximum number of reactions allowed in any model stored in a Multiple Model NetCDF file.
!***

!****v dataio/CellGrowthTime
!VARIABLE
!	CellGrowthTime is the average cell growth time when cell division is turned on.
!	It is set to zero if cell division is turned off.
!***

!****v dataio/CellGrowthTimeSD
!VARIABLE
!	CellGrowthTimeSD is the standard deviation of the cell growth time when cell division is turned on.
!***

!****v dataio/Trials
!VARIABLE
!	Trials is the number of trials, or independent realizations, to be simulated for each model.
!***

!****v dataio/TimePoints
!VARIABLE
!	TimePoints is the number of timepoints sampled from the simulation data. Applies to all models.
!	Currently equal to floor((TEnd - TStart) / SaveTime) + 1
!***

!****v dataio/TStart
!VARIABLE
!	TStart is the start time of the simulation.
!***

!****v dataio/TEnd
!VARIABLE
!	TEnd is the end time of the simulation.
!***

!****v dataio/SaveTime
!VARIABLE
!	SaveTime is the time interval between sampling the simulation data.
!***

!****v dataio/Vo
!VARIABLE
!	Vo is the initial volume of the system.
!***

!****v dataio/SplitOnDivision
!VARIABLE
!	SplitOnDivision is an Allocatable, 1D, Logical (N x 1). 1 if the corresponding species is halved in copy
!	number when cell division occurs.
!***

!****v dataio/SaveSpeciesData
!VARIABLE
!	SaveSpeciesData is an Allocatable, 1D, Logical (N x 1). 1 if the corresponding species' simulation data
!	is saved to disk
!***

!****v dataio/Xo
!VARIABLE
!	Xo is the initial number of molecules of all of the species.
!	Allocatable, 1D, (N x 1)
!***

!****v dataio/RandSeed
!VARIABLE
!	RandSeed is an allocatable integer holding the random seed
!	of the psuedorandom number generator.
!***

!****v dataio/filename
!VARIABLE
!	filename is the character string containing the name of the input file.
!***


!Change as needed. Upper limit is MaxRxnList
!Allocatable structure components NOT Allowed (on IBM compilers)!!


!****t dataio/RxnDataType
!TYPE
!	RxnDataType stores all reaction model data.
!COMMENT
!	The fields are:
!		c -- Kinetic parameters for the reaction (always mesoscopic outside of dataio)
!		MType -- An integer identifying the rate law of the reaction
!		Data -- Stores the number of steps in a gamma-distributed reaction (optional otherwise)
!		SListLen -- The length of SList
!		DListLen -- The length of DList
!		EListLen -- The length of EList
!		DList - The list of species that affect the rate of the reaction (in a specified order)
!		SList - The list of species that affect the state of the system (stoichiometry)
!		v -- The list of stoichiometric coefficients corresponding to the species in SList
!		EList - The list of events that are created or destroyed when a reaction occurs (such as transcriptional elongation)
!		EStoichList - The list of stoichiometric coefficients (-1/+1) corresponding to the events in EList.
!			      +1 is created. -1 is destroyed.
!***

!****t dataio/DGraphType
!TYPE
!	DGraphType stores the dependency graph for a single reaction.
!COMMENT
!	Fields are:
!		List -- A list of reactions that require updating when a reaction occurs.
!		ListLen -- The length of the list.
!***

!****v RxnDataType/RxnData
!VARIABLE
!	RxnData is the allocatable 1D of type RxnDataType (allocated up to M reactions)
!***

!****v DGraphType/DGraph
!VARIABLE
!	DGraph is the allocatable 1D of type DGraphType (allocated up to M reactions)
!***

CONTAINS
!---------------------------------------------------
Subroutine AllocateLocalModel(N, M)
IMPLICIT NONE

Integer, intent(in) :: N, M


if (Allocated(Xo)) THEN
   Deallocate(Xo)
end if

if (Allocated(SplitOnDivision)) THEN
   Deallocate(SplitOnDivision)
end if

if (Allocated(SaveSpeciesData)) THEN
   Deallocate(SaveSpeciesData)
end if

if (Allocated(Rxndata)) THEN
   Deallocate(Rxndata)
end if

if (Allocated(DGraph)) THEN
   Deallocate(DGraph)
end if

Allocate(Xo(N))
Allocate(SplitOnDivision(N))
Allocate(SaveSpeciesData(N))
Allocate(RxnData(M))
Allocate(DGraph(M))

End Subroutine
!----------------------------------------------------
Subroutine DeAllocateLocalModel
!Deallocates all global variables defined in local Model

Deallocate(Xo)
Deallocate(SplitOnDivision)
Deallocate(SaveSpeciesData)
Deallocate(Rxndata)
Deallocate(DGraph)

End Subroutine
!-----------------------------------------------------

End Module GlobalVariables
