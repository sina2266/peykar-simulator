/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/SundialsPdeScheduler.h>
#include <VCELL/SimTypes.h>

extern "C"
{
#if defined(FORTRAN_BARE)
#define PCILU pcilu
#elif defined(FORTRAN_UNDERSCORE)
#define PCILU pcilu_
#endif
void PCILU(int *, long *, int32 *, double *, double *, double *, double *);
}

#include <algorithm>
#include <iostream>
using std::max;
using std::endl;

#include <VCELL/Element.h>
#include <VCELL/SimTypes.h>
#include <VCELL/Solver.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/MembraneVariable.h>
#include <VCELL/VolumeRegionVariable.h>
#include <VCELL/MembraneRegionVariable.h>
#include <VCELL/Mesh.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/VolumeVarContextExpression.h>
#include <VCELL/VolumeRegionVarContextExpression.h>
#include <VCELL/MembraneVarContextExpression.h>
#include <VCELL/MembraneRegionVarContextExpression.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/FVUtils.h>
#include <VCELL/SimTool.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/VCellModel.h>
#include <SimpleSymbolTable.h>
#include <VCELL/SparseMatrixPCG.h>

#include <assert.h>
#include <string.h>
#include <limits.h>

#include <cvode/cvode.h>             /* prototypes for CVODE fcts. and consts. */
#include <nvector/nvector_serial.h>  /* serial N_Vector types, fcts., and macros */
#include <cvode/cvode_spgmr.h>       /* prototype for CVSPGMR */
#include <sundials/sundials_dense.h> /* definitions DenseMat DENSE_ELEM */
#include <sundials/sundials_types.h> /* definition of type realtype */

#define ToleranceType CV_SS
#define epsilon  1e-12

//#define SHOW_RHS
//#define SUNDIALS_USE_PCNONE
//#define COMPARE_WITH_OLD

static double interp_coeff[3] = {3.0/2, -1.0/2, 0};
static int dirichletExpIndexes[3] = {BOUNDARY_XP_EXP, BOUNDARY_YP_EXP, BOUNDARY_ZP_EXP};
static int velocityExpIndexes[3] = {VELOCITY_X_EXP, VELOCITY_Y_EXP, VELOCITY_Z_EXP};
static int gradExpIndexes[3] = {GRADIENT_X_EXP, GRADIENT_Y_EXP, GRADIENT_Z_EXP};
static int minusMasks[3] = {NEIGHBOR_XM_MASK, NEIGHBOR_YM_MASK, NEIGHBOR_ZM_MASK};
static int plusMasks[3] = {NEIGHBOR_XP_MASK, NEIGHBOR_YP_MASK, NEIGHBOR_ZP_MASK};

#define PRECOMPUTE_DIFFUSION_COEFFICIENT


SundialsPdeScheduler::SundialsPdeScheduler(SimulationExpression *sim, const SundialsSolverOptions& sso, int numDisTimes, double* disTimes, bool bDefaultOuptput) : Scheduler(sim)
{
    simulation = sim;
    mesh = simulation->getMesh();

    y = 0;
    sundialsSolverMemory = 0;

    sundialsSolverOptions = sso;
    currDiscontinuityTimeCount = 0;
    numDiscontinuityTimes = numDisTimes;
    discontinuityTimes = disTimes;

    numUnknowns = 0;

    M = 0;
    pcg_workspace = 0;

    bPcReinit = true;
    bLUcomputed = false;

    bSundialsOneStepOutput = bDefaultOuptput;
    currentTime = 0;

    statePointValues = 0;
    neighborStatePointValues = 0;

    diffCoeffs = 0;
    rhsGradients = 0;
}


SundialsPdeScheduler::~SundialsPdeScheduler()
{
    N_VDestroy_Serial(y);
    CVodeFree(&sundialsSolverMemory);

    delete[] statePointValues;

    if (bHasVariableDiffusionAdvection) {
        for (int n = 0; n < 3; n ++) {
            delete[] neighborStatePointValues[n];
        }
        delete neighborStatePointValues;
    }

    delete[] global2Local;
    delete[] local2Global;
    delete[] regionSizes;
    delete[] regionOffsets;
    delete[] volVectorOffsets;

    delete M;
    delete[] pcg_workspace;

    if (simulation->getNumVolVariables() > 0) {
        int numVolRegions = mesh->getNumVolumeRegions();
        delete[] regionDefinedVolVariableSizes;
        for (int r = 0; r < numVolRegions; r ++) {
            delete[] regionDefinedVolVariableIndexes[r];
        }
        delete[] regionDefinedVolVariableIndexes;
        delete[] bRegionHasTimeDepdentVariables;
        delete[] bRegionHasConstantCoefficients;
    }

    delete[] diffCoeffs;
    delete[] rhsGradients;
}

void SundialsPdeScheduler::iterate(SimTool* sim_tool) {
    if (bFirstTime) {
        setupOrderMaps();
        initSundialsSolver(sim_tool->getModel());
    }
    solve(sim_tool);
    bFirstTime = false;
}

void SundialsPdeScheduler::setupOrderMaps() {
    if (numUnknowns > 0) {
        return;
    }

    dimension =  mesh->getDimension();
    double Dx = mesh->getXScale_um();
    double Dy = mesh->getYScale_um();
    double Dz = mesh->getZScale_um();
    Nx = mesh->getNumVolumeX();
    int Ny = mesh->getNumVolumeY();
    int Nz = mesh->getNumVolumeZ();
    Nxy = Nx * Ny;
    int Nxyz = Nxy * Nz;

    VOLUME = mesh->getVolume_cu();

    oneOverH[0] = 1/Dx;
    oneOverH[1] = 1/Dy;
    oneOverH[2] = 1/Dz;

    pVolumeElement = mesh->getVolumeElements();
    pMembraneElement = mesh->getMembraneElements();

    int numVolRegions = mesh->getNumVolumeRegions();
    global2Local = new int[Nxyz];
    local2Global = new int[Nxyz];
    regionSizes = new int[numVolRegions];
    regionOffsets = new int[numVolRegions];
    regionDefinedVolVariableSizes = 0;
    regionDefinedVolVariableIndexes = 0;
    bRegionHasConstantCoefficients = 0;
    bHasAdvection = false;
    bHasVariableDiffusionAdvection = false;

    bRegionHasTimeDepdentVariables = 0;

    bHasGradient = false;

    int numActivePoints = 0;
    cout << endl << "numVolRegions=" << numVolRegions << endl;
    for (int r = 0; r < numVolRegions; r ++) {
        regionOffsets[r] = numActivePoints;
        for (int k = 0; k < Nz; k ++) {
            for (int j = 0; j < Ny; j ++) {
                for (int i = 0; i < Nx; i ++) {
                    int volIndex = k * Nxy + j * Nx + i;
                    int regionID = pVolumeElement[volIndex].getRegionIndex();
                    if (regionID == r) {
                        //global to local
                        global2Local[volIndex] = numActivePoints;
                        local2Global[numActivePoints] = volIndex;
                        numActivePoints ++;
                    }
                }
            }
        }
        regionSizes[r] = numActivePoints - regionOffsets[r];
        cout << "Region " << r << ": size=" << regionSizes[r] << ", offset=" << regionOffsets[r] << endl;
    }
    cout << "# of active points = " << numActivePoints << endl;

    // if local to global mapping is not always in increasing order
    // then in the rhs operator, we need a separate loop for reaction and scale volume
    for (int r = 0; r < numVolRegions; r ++) {
        int localIndex0 = regionOffsets[r];
        for (int localIndex = localIndex0 ; localIndex < localIndex0 + regionSizes[r]-1; localIndex ++) {
            int volIndex = local2Global[localIndex];
            int volIndexp = local2Global[localIndex+1];
            if (volIndexp < volIndex) {
                throw " local to global map not in increasing order";
            }
        }
    }

    volVectorOffsets = new int[numVolRegions];
    numUnknowns  = 0;
    if (simulation->getNumVolVariables() > 0) {
        bRegionHasConstantCoefficients = new bool[numVolRegions];
        regionDefinedVolVariableSizes = new int[numVolRegions];
        regionDefinedVolVariableIndexes = new int*[numVolRegions];

#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
        bRegionHasTimeDepdentVariables = new bool[numVolRegions];
#endif

        // Volume PDE/ODE first
        for (int r = 0; r < numVolRegions; r ++) {
            volVectorOffsets[r] = numUnknowns;
            regionDefinedVolVariableSizes[r] = 0;
            regionDefinedVolVariableIndexes[r] = 0;
            bRegionHasConstantCoefficients[r] = true;

#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
            bRegionHasTimeDepdentVariables[r] = false;
#endif
            int firstPointVolIndex = local2Global[regionOffsets[r]];
            Feature* feature = pVolumeElement[firstPointVolIndex].getFeature();
            for (int v = 0; v < simulation->getNumVolVariables(); v ++) {
                if (simulation->isVolumeVariableDefinedInRegion(v, r)) {
                    numUnknowns += regionSizes[r];
                    regionDefinedVolVariableSizes[r] ++;

                    VolumeVariable* var = (VolumeVariable*)simulation->getVolVariable(v);
                    VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);
                    if (var->isDiffusing() && !varContext->hasConstantCoefficients(dimension)) {
                        bRegionHasConstantCoefficients[r] = false;
                        bHasVariableDiffusionAdvection = true;
                    }
                    if (var->isAdvecting()) {
                        bHasAdvection = true;
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                        bRegionHasTimeDepdentVariables[r] = true;
#endif
                    }
                    if (var->hasGradient()) {
                        bHasGradient = true;
                    }
                }
            }
            if (regionDefinedVolVariableSizes[r] > 0) {
                cout << (bRegionHasConstantCoefficients[r] ? "Constant" : "Variable") << " diffusion/advection in region " << mesh->getVolumeRegion(r)->getName() << endl;
                regionDefinedVolVariableIndexes[r] = new int[regionDefinedVolVariableSizes[r]];
                int activeVarCount = 0;
                for (int v = 0; v < simulation->getNumVolVariables(); v ++) {
                    if (simulation->isVolumeVariableDefinedInRegion(v, r)) {
                        regionDefinedVolVariableIndexes[r][activeVarCount ++] = v;
                    }

#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                    if (bRegionHasTimeDepdentVariables[r]) {
                        continue;
                    }

                    VolumeVariable* var = (VolumeVariable*)simulation->getVolVariable(v);
                    VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);
                    if (var->isDiffusing()) {
                        if (!varContext->hasConstantDiffusion() &&  !varContext->hasXYZOnlyDiffusion() || var->hasGradient()) {
                            bRegionHasTimeDepdentVariables[r] = true;
                        }
                    }
#endif
                }
            }
        }
    }
    numVolUnknowns = numUnknowns;

    // Membrane ODE/PDE
    memVectorOffset = numUnknowns;
    numUnknowns += mesh->getNumMembraneElements() * simulation->getNumMemVariables();
    // Volume Region
    volRegionVectorOffset = numUnknowns;
    numUnknowns += numVolRegions * simulation->getNumVolRegionVariables();
    // Membrane Region
    memRegionVectorOffset = numUnknowns;
    numUnknowns += mesh->getNumMembraneRegions() * simulation->getNumMemRegionVariables();

    cout << "numUnknowns = " << numUnknowns << endl;
    if (bHasGradient) {
        rhsGradients = new double[numUnknowns];
    }
}

void SundialsPdeScheduler::checkCVodeReturnCode(int returnCode) {
    if (returnCode == CV_SUCCESS){
        return;
    }

    switch (returnCode){
        case CV_SUCCESS: {
            throw "CV_SUCCESS: CVode succeeded and no roots were found.";
        }
        case CV_ROOT_RETURN: {
            throw "CV_ROOT_RETURN: CVode succeeded, and found one or more roots. If nrtfn > 1, call CVodeGetRootInfo to see which g_i were found to have a root at (*tret).";
        }
        case CV_TSTOP_RETURN: {
            throw "CV_TSTOP_RETURN: CVode succeeded and returned at tstop.";
        }
        case CV_MEM_NULL:{
            throw "CV_MEM_NULL: mem argument was null";
        }
        case CV_ILL_INPUT:{
            throw "CV_ILL_INPUT: one of the inputs to CVode is illegal";
        }
        case CV_TOO_MUCH_WORK:{
            throw "CV_TOO_MUCH_WORK: took mxstep internal steps but could not reach tout";
        }
        case CV_TOO_MUCH_ACC:{
            throw "CV_TOO_MUCH_ACC: could not satisfy the accuracy demanded by the user for some internal step";
        }
        case CV_ERR_FAILURE:{
            throw "CV_ERR_FAILURE: error test failures occurred too many times during one internal step";
        }
        case CV_CONV_FAILURE:{
            throw "CV_CONV_FAILURE: convergence test failures occurred too many times during one internal step";
        }
        case CV_LINIT_FAIL:{
            throw "CV_LINIT_FAIL: the linear sundialsSolverMemory's initialization function failed.";
        }
        case CV_LSETUP_FAIL:{
            throw "CV_LSETUP_FAIL: the linear sundialsSolverMemory's setup routine failed in an unrecoverable manner.";
        }
        case CV_LSOLVE_FAIL:{
            throw "CV_LSOLVE_FAIL: the linear sundialsSolverMemory's solve routine failed in an unrecoverable manner";
        }
        case CV_REPTD_RHSFUNC_ERR: {
            throw "repeated recoverable right-hand side function errors : ";
        }
        case CV_UNREC_RHSFUNC_ERR:{
            throw "the right-hand side failed in a recoverable manner, but no recovery is possible";
        }
        default:
            throw CVodeGetReturnFlagName(returnCode);
    }
}

int SundialsPdeScheduler::RHS_callback(realtype t, N_Vector yinput, N_Vector rhs, void *fdata) {
    SundialsPdeScheduler* solver = (SundialsPdeScheduler*)fdata;
    double* y_data = NV_DATA_S(yinput);
    double* r_data = NV_DATA_S(rhs);
    return solver->CVodeRHS(t, y_data, r_data);
}

int SundialsPdeScheduler::CVodeRHS(double t, double* yinput, double* rhs) {
#ifdef COMPARE_WITH_OLD
    double* r1 = new double[numUnknowns];
	double* r2 = new double[numUnknowns];
	memset(r1, 0, numUnknowns * sizeof(double));
	memset(r2, 0, numUnknowns * sizeof(double));
	applyVolumeOperatorOld(t, yinput, r1);
	applyVolumeOperator(t, yinput, r2);

	cout << "--comparing with old code at " << t << endl;
	for (int i = 0; i < numUnknowns; i ++) {
		double relerror = r2[i] == 0? fabs(r1[i] - r2[i]) : fabs((r1[i] - r2[i])/r2[i]);
		if (relerror > 1e-5) {
			cout << i << ":" << "r1=" << r1[i] << ", r2=" << r2[i] << endl;
		}
	}
	cout << "--compared with old code" << endl;
	delete[] r1;
	delete[] r2;
#endif
    //cout << yinput[14008] << endl;
    //if (MathUtil::isInfinity(yinput[14008]) || MathUtil::isNaN(yinput[14008])) {
    //	exit(1);
    //}
    memset(rhs, 0, numUnknowns * sizeof(double));
    if (bHasGradient) {
        memset(rhsGradients, 0, numUnknowns * sizeof(double));
    }
    applyVolumeOperator(t, yinput, rhs);
    applyMembraneDiffusionReactionOperator(t, yinput, rhs);
    applyVolumeRegionReactionOperator(t, yinput, rhs);
    applyMembraneRegionReactionOperator(t, yinput, rhs);
    applyMembraneFluxOperator(t, yinput, rhs);

#ifdef SHOW_RHS
    cout << endl << "-----------RHS----at time " << t << "--------------" << endl;
	for (int i = 0; i < numUnknowns; i ++) {
		cout << i << " " << rhs[i] << endl;
	}
	cout << endl;
#endif

    return 0;
}

void SundialsPdeScheduler::initSundialsSolver(VCellModel* model) {
    if (!bFirstTime) {
        return;
    }

    int numVolVar = simulation->getNumVolVariables();
    int numMemVar = simulation->getNumMemVariables();
    int numVolRegionVar = simulation->getNumVolRegionVariables();
    int numMemRegionVar = simulation->getNumMemRegionVariables();
    if (sundialsSolverMemory == 0) {
        numSymbolsPerVolVar = model->getNumFeatures() + 1;

        // t, x, y, z, (U, U_Feature1_membrane, U_Feature2_membrane, ...), (M),
        // (VR, VR_Feature1_membrane, ...), (MR), (RegionSize), (FieldData), (RandomVariable), (Parameter)
        volSymbolOffset = 4;
        memSymbolOffset = volSymbolOffset + numVolVar * numSymbolsPerVolVar;
        volRegionSymbolOffset = memSymbolOffset + numMemVar;
        memRegionSymbolOffset = volRegionSymbolOffset + numVolRegionVar * numSymbolsPerVolVar;
        regionSizeVariableSymbolOffset = memRegionSymbolOffset + numMemRegionVar;
        fieldDataSymbolOffset = regionSizeVariableSymbolOffset + simulation->getNumRegionSizeVariables();
        randomVariableSymbolOffset = fieldDataSymbolOffset + simulation->getNumFields();
        parameterSymbolOffset = randomVariableSymbolOffset + simulation->getNumRandomVariables();

        int valueArraySize = parameterSymbolOffset + simulation->getNumParameters();

        statePointValues = new double[valueArraySize];
        memset(statePointValues, 0, valueArraySize * sizeof(double));

        if (bHasVariableDiffusionAdvection) {
            neighborStatePointValues = new double*[3];
            for (int n = 0; n < 3; n ++) {
                neighborStatePointValues[n] = new double[valueArraySize];
                memset(neighborStatePointValues[n], 0, valueArraySize * sizeof(double));
            }
        }

#ifndef SUNDIALS_USE_PCNONE
        preallocateM();
#endif

        y = N_VNew_Serial(numUnknowns);
        if (y == 0) {
            throw "SundialsPDESolver:: Out of Memory : y ";
        }
    }

#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
    // initialize diff coeff
    if (bHasVariableDiffusionAdvection) {
        precomputeDiffusionCoefficients();
    }
#endif

    // initialize y with initial conditions
    int count = 0;
    double *y_data = NV_DATA_S(y);
    if (numVolVar > 0) {
        for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
            for (int ri = 0; ri < regionSizes[r]; ri ++) {
                int volIndex = local2Global[ri + regionOffsets[r]];
                for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[r]; activeVarCount ++) {
                    int varIndex = regionDefinedVolVariableIndexes[r][activeVarCount];
                    y_data[count ++] = simulation->getVolVariable(varIndex)->getCurr(volIndex);
                }
            }
        }
    }
    if (simulation->getNumMemVariables() > 0) {
        for (int mi = 0; mi < mesh->getNumMembraneElements(); mi ++) {
            for (int v = 0; v < simulation->getNumMemVariables(); v ++) {
                y_data[count ++] = simulation->getMemVariable(v)->getCurr(mi);
            }
        }
    }

    if (simulation->getNumVolRegionVariables() > 0) {
        for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
            for (int v = 0; v < simulation->getNumVolRegionVariables(); v ++) {
                y_data[count ++] = simulation->getVolRegionVariable(v)->getCurr(r);
            }
        }
    }

    if (simulation->getNumMemRegionVariables() > 0) {
        for (int r = 0; r < mesh->getNumMembraneRegions(); r ++) {
            for (int v = 0; v < simulation->getNumMemRegionVariables(); v ++) {
                y_data[count ++] = simulation->getMemRegionVariable(v)->getCurr(r);
            }
        }
    }

    int returnCode = 0;
    if (sundialsSolverMemory == 0) {
        sundialsSolverMemory = CVodeCreate(CV_BDF, CV_NEWTON);
        if (sundialsSolverMemory == 0) {
            throw "SundialsPDESolver:: Out of memory : sundialsSolverMemory";
        }

        returnCode = CVodeMalloc(sundialsSolverMemory, RHS_callback, currentTime, y, ToleranceType,
                                 sundialsSolverOptions.relTol, &sundialsSolverOptions.absTol);
        checkCVodeReturnCode(returnCode);

        returnCode = CVodeSetFdata(sundialsSolverMemory, this);
        checkCVodeReturnCode(returnCode);

        // set linear solver
#ifdef SUNDIALS_USE_PCNONE
        cout << endl << "******using Sundials CVode with PREC_NONE";
		returnCode = CVSpgmr(sundialsSolverMemory, PREC_NONE, 0);
#else
        if (simulation->getNumMemPde() == 0 && simulation->getNumVolPde() == 0) {
            cout << endl << "******ODE only, using Sundials CVode with PREC_NONE";
            returnCode = CVSpgmr(sundialsSolverMemory, PREC_NONE, 0);
        } else {
            cout << endl << "****** using Sundials CVode with PREC_LEFT";
            returnCode = CVSpgmr(sundialsSolverMemory, PREC_LEFT, 0);
        }
#endif
        cout << ", relTol=" << sundialsSolverOptions.relTol << ", absTol=" << sundialsSolverOptions.absTol
             << ", maxStep=" <<  sundialsSolverOptions.maxStep;
        checkCVodeReturnCode(returnCode);

        // set max absolute step size
        returnCode = CVodeSetMaxStep(sundialsSolverMemory, sundialsSolverOptions.maxStep);
        checkCVodeReturnCode(returnCode);

        // set max of number of steps
        returnCode = CVodeSetMaxNumSteps(sundialsSolverMemory, LONG_MAX);
        checkCVodeReturnCode(returnCode);

        // set preconditioner.
        returnCode = CVSpilsSetPreconditioner (sundialsSolverMemory, pcSetup_callback, pcSolve_callback, this);
        checkCVodeReturnCode(returnCode);

        if (bHasAdvection) {
            returnCode = CVodeSetMaxOrd(sundialsSolverMemory, sundialsSolverOptions.maxOrderAdvection);
            checkCVodeReturnCode(returnCode);
            cout << ", maxOrderAdvection=" << sundialsSolverOptions.maxOrderAdvection;
        }
        cout << endl;
    } else {
        returnCode = CVodeReInit(sundialsSolverMemory, RHS_callback, currentTime, y, ToleranceType,
                                 sundialsSolverOptions.relTol, &sundialsSolverOptions.absTol);
        checkCVodeReturnCode(returnCode);
    }

    cout << endl << "----------------------------------" << endl;
    cout << "sundials pde solver is starting from time " << currentTime << endl;
    cout << "----------------------------------" << endl;

    // only populate once serial scan parameter values
    simulation->populateParameterValues(statePointValues + parameterSymbolOffset);
    if (bHasVariableDiffusionAdvection) {
        for (int n = 0; n < 3; n ++) {
            simulation->populateParameterValues(neighborStatePointValues[n] + parameterSymbolOffset);
        }
    }

#ifndef SUNDIALS_USE_PCNONE
    if (!simulation->hasTimeDependentDiffusionAdvection()) {
        oldGamma = 1.0;
        buildM_Volume(currentTime, 0, oldGamma);
        buildM_Membrane(currentTime, 0, oldGamma);
    }
#endif

    currDiscontinuityTimeCount = 0;
    while (numDiscontinuityTimes > 0 && discontinuityTimes[currDiscontinuityTimeCount] < currentTime) {
        currDiscontinuityTimeCount ++;
    }
}

void SundialsPdeScheduler::printCVodeStats()
{
    long int lenrw, leniw ;
    long int lenrwLS, leniwLS;
    long int nst, nfe, nsetups, nni, ncfn, netf;
    long int nli, npe, nps, ncfl, nfeLS;
    int flag;

    flag = CVodeGetWorkSpace(sundialsSolverMemory, &lenrw, &leniw);
    checkCVodeReturnCode(flag);
    flag = CVodeGetNumSteps(sundialsSolverMemory, &nst);
    checkCVodeReturnCode(flag);
    flag = CVodeGetNumRhsEvals(sundialsSolverMemory, &nfe);
    checkCVodeReturnCode(flag);
    flag = CVodeGetNumLinSolvSetups(sundialsSolverMemory, &nsetups);
    checkCVodeReturnCode(flag);
    flag = CVodeGetNumErrTestFails(sundialsSolverMemory, &netf);
    checkCVodeReturnCode(flag);
    flag = CVodeGetNumNonlinSolvIters(sundialsSolverMemory, &nni);
    checkCVodeReturnCode(flag);
    flag = CVodeGetNumNonlinSolvConvFails(sundialsSolverMemory, &ncfn);
    checkCVodeReturnCode(flag);

    flag = CVSpilsGetWorkSpace(sundialsSolverMemory, &lenrwLS, &leniwLS);
    checkCVodeReturnCode(flag);
    flag = CVSpilsGetNumLinIters(sundialsSolverMemory, &nli);
    checkCVodeReturnCode(flag);
    flag = CVSpilsGetNumPrecEvals(sundialsSolverMemory, &npe);
    checkCVodeReturnCode(flag);
    flag = CVSpilsGetNumPrecSolves(sundialsSolverMemory, &nps);
    checkCVodeReturnCode(flag);
    flag = CVSpilsGetNumConvFails(sundialsSolverMemory, &ncfl);
    checkCVodeReturnCode(flag);
    flag = CVSpilsGetNumRhsEvals(sundialsSolverMemory, &nfeLS);
    checkCVodeReturnCode(flag);

    double hlast;
    flag = CVodeGetLastStep(sundialsSolverMemory, &hlast);
    checkCVodeReturnCode(flag);

    printf("\nFinal Statistics.. \n\n");
    printf("lenrw   = %5ld     leniw   = %5ld\n", lenrw, leniw);
    printf("lenrwLS = %5ld     leniwLS = %5ld\n", lenrwLS, leniwLS);
    printf("nst     = %5ld\n"                  , nst);
    printf("nfe     = %5ld     nfeLS   = %5ld\n"  , nfe, nfeLS);
    printf("nni     = %5ld     nli     = %5ld\n"  , nni, nli);
    printf("nsetups = %5ld     netf    = %5ld\n"  , nsetups, netf);
    printf("npe     = %5ld     nps     = %5ld\n"  , npe, nps);
    printf("ncfn    = %5ld     ncfl    = %5ld\n", ncfn, ncfl);
    printf("last step  = %f\n\n", hlast);
}

void SundialsPdeScheduler::solve(SimTool* sim_tool) {
    double endTime = 0;
    if (bSundialsOneStepOutput) {
        endTime = sim_tool->getEndTime();
    } else {
        endTime = currentTime + simulation->getDT_sec();
    }

    bool bStop = false;
    double stopTime = endTime;
    // if next stop time between (currTime, endTime]
    if (currDiscontinuityTimeCount < numDiscontinuityTimes && currentTime < discontinuityTimes[currDiscontinuityTimeCount] && (endTime >  discontinuityTimes[currDiscontinuityTimeCount] || fabs(endTime - discontinuityTimes[currDiscontinuityTimeCount]) < epsilon)) {
        stopTime = discontinuityTimes[currDiscontinuityTimeCount];
        bStop = true;
    }

    while (true) {
        if (fabs(endTime - currentTime) < epsilon) {
            break;
        }
        CVodeSetStopTime(sundialsSolverMemory, stopTime);
        int returnCode = CVode(sundialsSolverMemory, stopTime, y, &currentTime,  bSundialsOneStepOutput ? CV_ONE_STEP_TSTOP : CV_NORMAL_TSTOP);
        if (returnCode != CV_SUCCESS && returnCode != CV_TSTOP_RETURN) {
            checkCVodeReturnCode(returnCode);
        }

        if (bStop && fabs(stopTime - currentTime) < epsilon) {
            currDiscontinuityTimeCount ++;
            if (sim_tool->getEndTime() - currentTime > epsilon  // currentTime less than endTime
                || fabs(sim_tool->getEndTime() - currentTime) > epsilon) { // if this is the last solve, we don't have to reinit
                cout << endl << "SundialsPdeScheduler::solve() : cvode reinit at time " << currentTime << endl;
                returnCode = CVodeReInit(sundialsSolverMemory, RHS_callback, currentTime, y, ToleranceType,
                                         sundialsSolverOptions.relTol, &sundialsSolverOptions.absTol);
                checkCVodeReturnCode(returnCode);
            }

            if (bSundialsOneStepOutput) {
                break;
            }
            // find out next stop time
            bStop = false;
            stopTime = endTime;
            if (currDiscontinuityTimeCount < numDiscontinuityTimes
                && currentTime < discontinuityTimes[currDiscontinuityTimeCount]
                && (endTime >  discontinuityTimes[currDiscontinuityTimeCount]
                    || fabs(endTime - discontinuityTimes[currDiscontinuityTimeCount]) < epsilon)) {
                stopTime = discontinuityTimes[currDiscontinuityTimeCount];
                bStop = true;
            }
        }
        if (bSundialsOneStepOutput) {
            break;
        }
    }

    if (currentTime - sim_tool->getEndTime() > epsilon // currentTime greater than endTime
        || (sim_tool->getEndTime() - currentTime) < epsilon) {
        printCVodeStats();
    }
    updateSolutions();
}

void SundialsPdeScheduler::updateSolutions() {
    double *y_data = NV_DATA_S(y);
    int count = 0;

    if (simulation->getNumVolVariables() > 0) {
        for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
            bool bDirichletBoundary = mesh->getVolumeRegion(r)->isBoundaryDirichlet();
            for (int ri = 0; ri < regionSizes[r]; ri ++) {
                int volIndex = local2Global[ri + regionOffsets[r]];
                int mask = pVolumeElement[volIndex].neighborMask;

                for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[r]; activeVarCount ++) {
                    int varIndex = regionDefinedVolVariableIndexes[r][activeVarCount];
                    VolumeVariable *var = simulation->getVolVariable(varIndex);

                    double sol = y_data[count ++];
                    if (var->isDiffusing() && bDirichletBoundary && (mask & BOUNDARY_TYPE_DIRICHLET)) {
                        updateVolumeStatePointValues(volIndex, currentTime, y_data, statePointValues);
                        Feature* feature = pVolumeElement[volIndex].getFeature();
                        VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);
                        if ((mask & NEIGHBOR_XM_BOUNDARY) && (feature->getXmBoundaryType() == BOUNDARY_VALUE)){
                            sol = varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues);
                        } else if ((mask & NEIGHBOR_XP_BOUNDARY) && (feature->getXpBoundaryType() == BOUNDARY_VALUE)){
                            sol = varContext->evaluateExpression(BOUNDARY_XP_EXP, statePointValues);
                        } else if ((mask & NEIGHBOR_YM_BOUNDARY) && (feature->getYmBoundaryType() == BOUNDARY_VALUE)){
                            sol = varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues);
                        } else if ((mask & NEIGHBOR_YP_BOUNDARY) && (feature->getYpBoundaryType() == BOUNDARY_VALUE)){
                            sol = varContext->evaluateExpression(BOUNDARY_YP_EXP, statePointValues);
                        } else if ((mask & NEIGHBOR_ZM_BOUNDARY) && (feature->getZmBoundaryType() == BOUNDARY_VALUE)){
                            sol = varContext->evaluateExpression(BOUNDARY_ZM_EXP, statePointValues);
                        } else if ((mask & NEIGHBOR_ZP_BOUNDARY) && (feature->getZpBoundaryType() == BOUNDARY_VALUE)){
                            sol = varContext->evaluateExpression(BOUNDARY_ZP_EXP, statePointValues);
                        }
                    }
                    var->getCurr()[volIndex] = sol;
                }
            }
        }
    }

    // update membrane
    if (simulation->getNumMemVariables() > 0) {
        for (int m = 0; m < mesh->getNumMembraneElements(); m ++) {
            for (int v = 0; v < simulation->getNumMemVariables(); v ++) {
                MembraneVariable *var = (MembraneVariable*)simulation->getMemVariable(v);
                int mask = mesh->getMembraneNeighborMask(m);
                double sol = y_data[count ++];
                if (var->isDiffusing() && (mask & BOUNDARY_TYPE_DIRICHLET)) {
                    updateMembraneStatePointValues(pMembraneElement[m], currentTime, y_data, statePointValues);

                    Membrane* membrane = pMembraneElement[m].getMembrane();
                    MembraneVarContextExpression *varContext = membrane->getMembraneVarContext(var);
                    if ((mask & NEIGHBOR_XM_BOUNDARY) && (membrane->getXmBoundaryType() == BOUNDARY_VALUE)){
                        sol = varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues);
                    } else if ((mask & NEIGHBOR_XP_BOUNDARY) && (membrane->getXpBoundaryType() == BOUNDARY_VALUE)){
                        sol = varContext->evaluateExpression(BOUNDARY_XP_EXP, statePointValues);
                    } else if ((mask & NEIGHBOR_YM_BOUNDARY) && (membrane->getYmBoundaryType() == BOUNDARY_VALUE)){
                        sol = varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues);
                    } else if ((mask & NEIGHBOR_YP_BOUNDARY) && (membrane->getYpBoundaryType() == BOUNDARY_VALUE)){
                        sol = varContext->evaluateExpression(BOUNDARY_YP_EXP, statePointValues);
                    } else if ((mask & NEIGHBOR_ZM_BOUNDARY) && (membrane->getZmBoundaryType() == BOUNDARY_VALUE)){
                        sol = varContext->evaluateExpression(BOUNDARY_ZM_EXP, statePointValues);
                    } else if ((mask & NEIGHBOR_ZP_BOUNDARY) && (membrane->getZpBoundaryType() == BOUNDARY_VALUE)){
                        sol = varContext->evaluateExpression(BOUNDARY_ZP_EXP, statePointValues);
                    }
                }
                var->getCurr()[m] = sol;
            }
        }
    }

    // update volume region variable
    if (simulation->getNumVolRegionVariables() > 0) {
        for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
            for (int v = 0; v < simulation->getNumVolRegionVariables(); v ++) {
                VolumeRegionVariable *var = simulation->getVolRegionVariable(v);
                double sol = y_data[count ++];
                var->getCurr()[r] = sol;
            }
        }
    }
    // update membrane region variable
    if (simulation->getNumMemRegionVariables() > 0) {
        for (int r = 0; r < mesh->getNumMembraneRegions(); r ++) {
            for (int v = 0; v < simulation->getNumMemRegionVariables(); v ++) {
                MembraneRegionVariable *var = simulation->getMemRegionVariable(v);
                double sol = y_data[count ++];
                var->getCurr()[r] = sol;
            }
        }
    }
}

void SundialsPdeScheduler::applyVolumeOperatorOld(double t, double* yinput, double* rhs) {
    short advectDirs[6] = {1, 1, 1, -1, -1, -1};
    int dirichletExpIndexes[6] = {BOUNDARY_ZM_EXP, BOUNDARY_YM_EXP, BOUNDARY_XM_EXP, BOUNDARY_XP_EXP, BOUNDARY_YP_EXP, BOUNDARY_ZP_EXP};
    int velocityExpIndexes[6] = {VELOCITY_Z_EXP, VELOCITY_Y_EXP, VELOCITY_X_EXP, VELOCITY_X_EXP, VELOCITY_Y_EXP, VELOCITY_Z_EXP};

    for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
        int activeVarCount = -1;
        for (int v = 0; v < simulation->getNumVolVariables(); v ++) {
            VolumeVariable* var = simulation->getVolVariable(v);

            if (!simulation->isVolumeVariableDefinedInRegion(v, r)) {
                continue;
            }
            activeVarCount ++;
            int firstPointVolIndex = local2Global[regionOffsets[r]];
            Feature* feature = pVolumeElement[firstPointVolIndex].getFeature();
            VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);
            bool bHasConstantDiffusion = false;
            double Di = 0;
            if (varContext->hasConstantDiffusion()) {
                bHasConstantDiffusion = true;
                Di = varContext->evaluateConstantExpression(DIFF_RATE_EXP);
            }

            for (int ri = 0; ri < regionSizes[r]; ri ++) {
                int localIndex = ri + regionOffsets[r];
                int volIndex = local2Global[localIndex];
                int mask = pVolumeElement[volIndex].neighborMask;
                int vectorIndex = getVolumeElementVectorOffset(volIndex, r) + activeVarCount;

                // update values
                if (!var->isDiffusing() || !(mask & BOUNDARY_TYPE_DIRICHLET)){
                    updateVolumeStatePointValues(volIndex, t, yinput, statePointValues);
                    double reactionRate = varContext->evaluateExpression(REACT_RATE_EXP, statePointValues);
                    // add reaction
                    rhs[vectorIndex] += reactionRate;
                }

                if (!var->isDiffusing()) {
                    //validateNumber(var->getName(), volIndex, "RHS", rhs[vectorIndex]);
                    continue;
                }

                if (mask & BOUNDARY_TYPE_DIRICHLET) {// dirichlet
                    rhs[vectorIndex] = 0;
                    continue;
                }

                double lambdaX = oneOverH[0]*oneOverH[0];
                double lambdaY = oneOverH[1]*oneOverH[1];
                double lambdaZ = oneOverH[2]*oneOverH[2];
                double lambdaAreaX = oneOverH[0];
                double lambdaAreaY = oneOverH[1];
                double lambdaAreaZ = oneOverH[2];

                if (mask & NEIGHBOR_BOUNDARY_MASK){   // boundary
                    if (mask & NEIGHBOR_X_BOUNDARY_MASK){
                        lambdaX *= 2.0;
                        lambdaAreaX *= 2.0;
                    }
                    if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
                        lambdaY *= 2.0;
                        lambdaAreaY *= 2.0;
                    }
                    if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
                        lambdaZ *= 2.0;
                        lambdaAreaZ *= 2.0;
                    }

                    // add neumann condition
                    // boundary flux equals -D*DU/DX at the xm and xp walls, -D*DU/DY at the ym and yp walls
                    if ((mask & BOUNDARY_TYPE_MASK) == BOUNDARY_TYPE_NEUMANN) {
                        // updateVolumeStatePointValues(volIndex, t, yinput);  // this is needed, but update already above, if you move this block, uncomment this statement
                        if (mask & NEIGHBOR_XM_BOUNDARY && feature->getXmBoundaryType() == BOUNDARY_FLUX){
                            rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues) * lambdaAreaX;
                        }
                        if (mask & NEIGHBOR_XP_BOUNDARY && feature->getXpBoundaryType() == BOUNDARY_FLUX){
                            rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_XP_EXP, statePointValues) * lambdaAreaX;
                        }
                        if (dimension > 1) {
                            if (mask & NEIGHBOR_YM_BOUNDARY && feature->getYmBoundaryType() == BOUNDARY_FLUX){
                                rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues) * lambdaAreaY;
                            }
                            if (mask & NEIGHBOR_YP_BOUNDARY && feature->getYpBoundaryType() == BOUNDARY_FLUX){
                                rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_YP_EXP, statePointValues) * lambdaAreaY;
                            }

                            if (dimension > 2) {
                                if (mask & NEIGHBOR_ZM_BOUNDARY && feature->getZmBoundaryType() == BOUNDARY_FLUX){
                                    rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_ZM_EXP, statePointValues) * lambdaAreaZ;
                                }
                                if (mask & NEIGHBOR_ZP_BOUNDARY && feature->getZpBoundaryType() == BOUNDARY_FLUX){
                                    rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_ZP_EXP, statePointValues) * lambdaAreaZ;
                                }
                            }
                        }
                    }
                }

                // compute Di, Vi
                if (!bHasConstantDiffusion) {
                    Di = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
                }

                double Vix=0, Viy=0, Viz=0;
                if (var->isAdvecting()) {
                    // updateVolumeStatePointValues(volIndex, t, yinput);  // this is needed, but update already above, if you move this block, uncomment this statement
                    Vix = varContext->evaluateExpression(VELOCITY_X_EXP, statePointValues);
                    if (dimension > 1) {
                        Viy = varContext->evaluateExpression(VELOCITY_Y_EXP, statePointValues);
                    }
                    if (dimension > 2) {
                        Viz = varContext->evaluateExpression(VELOCITY_Z_EXP, statePointValues);
                    }
                }

                // order is ZM, YM, XM, XP, YP, ZP
                int volumeNeighbors[6] = {
                        (dimension < 3 || mask & NEIGHBOR_ZM_MASK) ? -1 : volIndex - Nxy,
                        (dimension < 2 || mask & NEIGHBOR_YM_MASK) ? -1 : volIndex - Nx,
                        (mask & NEIGHBOR_XM_MASK) ? -1 : volIndex - 1,
                        (mask & NEIGHBOR_XP_MASK) ? -1 : volIndex + 1,
                        (dimension < 2 || mask & NEIGHBOR_YP_MASK) ? -1 : volIndex + Nx,
                        (dimension < 3 || mask & NEIGHBOR_ZP_MASK) ? -1 : volIndex + Nxy
                };

                double neighborLambdas[6] = {lambdaZ, lambdaY, lambdaX, lambdaX, lambdaY, lambdaZ};
                double neighborLambdaAreas[6] = {lambdaAreaZ, lambdaAreaY, lambdaAreaX, lambdaAreaX, lambdaAreaY, lambdaAreaZ};
                double Vis[6] = {Viz, Viy, Vix, Vix, Viy, Viz};

                double Aii = 0;
                // add diffusion and convection
                for (int n = 0; n < 6; n ++) {
                    int neighborIndex = volumeNeighbors[n];
                    if (neighborIndex < 0) {
                        continue;
                    }
                    int neighborVectorIndex = getVolumeElementVectorOffset(neighborIndex, r) + activeVarCount;
                    double lambda = neighborLambdas[n];

                    bool bNeighborStatePointValuesComputed = false;
                    double D = Di;
                    if (!bHasConstantDiffusion) {
                        updateVolumeStatePointValues(neighborIndex, t, yinput, statePointValues);
                        bNeighborStatePointValuesComputed = true;
                        double Dj = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
                        D = (Di + Dj < epsilon) ? (0.0) : (2 * Di * Dj/(Di + Dj));
                    }
                    double yneighbor = yinput[neighborVectorIndex];
                    int neighborMask = pVolumeElement[neighborIndex].neighborMask;
                    if (neighborMask & BOUNDARY_TYPE_DIRICHLET) {
                        if (!bNeighborStatePointValuesComputed) {
                            updateVolumeStatePointValues(neighborIndex, t, yinput, statePointValues);
                            bNeighborStatePointValuesComputed = true;
                        }
                        yneighbor = varContext->evaluateExpression(dirichletExpIndexes[n], statePointValues);
                    }

                    double Aij = 0;
                    if (var->isAdvecting()) {
                        if (!bNeighborStatePointValuesComputed) {
                            updateVolumeStatePointValues(neighborIndex, t, yinput, statePointValues);
                            bNeighborStatePointValuesComputed = true;
                        }
                        int advectDir = advectDirs[n];
                        double Vi = Vis[n];
                        double Vj = varContext->evaluateExpression(velocityExpIndexes[n], statePointValues);
                        double V = 0.5 * (Vi + Vj);
                        double lambdaArea = neighborLambdaAreas[n];
                        Aij = max<double>(D * lambda + advectDir * 0.5 * V * lambdaArea, max<double>(advectDir * V * lambdaArea, 0));;
                        Aii += max<double>(D * lambda - advectDir * 0.5 * V * lambdaArea, max<double>(- advectDir * V * lambdaArea, 0));
                    } else {
                        Aij = D * lambda;
                        Aii += Aij;
                    }
                    rhs[vectorIndex] += yneighbor * Aij;
                } // end for n
                rhs[vectorIndex] -=  yinput[vectorIndex] * Aii;
                //validateNumber(var->getName(), volIndex, "RHS", rhs[vectorIndex]);
            } // end for ri
        } // end for v
    } // end for r
}

static void applyAdvectionHybridScheme(double diffTerm, double advectTerm, double& Aii, double& Aij) {
    //Aij = max(diffTerm + 0.5 advectTerm, max(advectTerm, 0));
    //Aii = max(diffTerm - 0.5 advectTerm, max(- advectTerm, 0));

    if (diffTerm < 0.5 * fabs(advectTerm)){ // upwind scheme
        Aij = max<double>(advectTerm, 0);
        Aii = max<double>(- advectTerm, 0);
    } else { // central difference scheme
        Aij = diffTerm + 0.5 * advectTerm;
        Aii = diffTerm - 0.5 * advectTerm;
    }
}

void SundialsPdeScheduler::applyVolumeOperator(double t, double* yinput, double* rhs) {
    if (simulation->getNumVolVariables() == 0) {
        return;
    }
    for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
        if (bRegionHasConstantCoefficients[r]) {
            regionApplyVolumeOperatorConstant(r, t, yinput, rhs);
        } else {
            regionApplyVolumeOperatorVariable(r, t, yinput, rhs);
        }
    }
}

// for dirichlet point, boundary condition has to be used.
void SundialsPdeScheduler::dirichletPointSetup(int volIndex, Feature* feature, VarContext* varContext, int mask, int* volumeNeighbors, double& ypoint) {
    volumeNeighbors[0] = -1;
    volumeNeighbors[1] = -1;
    volumeNeighbors[2] = -1;
    if (dimension > 2 && (mask & NEIGHBOR_ZM_MASK))  {
        if (!(mask & NEIGHBOR_ZP_MASK)) {
            volumeNeighbors[2] = volIndex + Nxy;
        }
        if (feature->getZmBoundaryType() == BOUNDARY_VALUE) {
            ypoint = varContext->evaluateExpression(BOUNDARY_ZM_EXP,  statePointValues);
        }
    }
    if (dimension > 1 && (mask & NEIGHBOR_YM_MASK))  {
        if (!(mask & NEIGHBOR_YP_MASK)) {
            volumeNeighbors[1] = volIndex + Nx;
        }
        if (feature->getYmBoundaryType() == BOUNDARY_VALUE) {
            ypoint = varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues);
        }
    }
    if (mask & NEIGHBOR_XM_MASK)  {
        // make sure we take care of mixed bc (e.g. neumann/dirichlet)
        // then only xp is a neighbor
        if (!(mask & NEIGHBOR_XP_MASK)) {
            volumeNeighbors[0] = volIndex + 1;
        }
        if (feature->getXmBoundaryType() == BOUNDARY_VALUE) {
            ypoint = varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues);
        }
    }
}

// boundary flux equals -D*DU/DX at the xm and xp walls, -D*DU/DY at the ym and yp walls
double SundialsPdeScheduler::computeNeumannCondition(Feature* feature, VarContext* varContext, int mask, double* scaleSs) {
    double boundaryCondition = 0;
    if (mask & NEIGHBOR_XM_BOUNDARY && feature->getXmBoundaryType() == BOUNDARY_FLUX){
        boundaryCondition += varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues) * oneOverH[0] * scaleSs[0];
    }
    if (mask & NEIGHBOR_XP_BOUNDARY && feature->getXpBoundaryType() == BOUNDARY_FLUX){
        boundaryCondition -= varContext->evaluateExpression(BOUNDARY_XP_EXP, statePointValues) * oneOverH[0] * scaleSs[0];
    }
    if (dimension > 1) {
        if (mask & NEIGHBOR_YM_BOUNDARY && feature->getYmBoundaryType() == BOUNDARY_FLUX){
            boundaryCondition += varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues) * oneOverH[1] * scaleSs[1];
        }
        if (mask & NEIGHBOR_YP_BOUNDARY && feature->getYpBoundaryType() == BOUNDARY_FLUX){
            boundaryCondition -= varContext->evaluateExpression(BOUNDARY_YP_EXP, statePointValues) * oneOverH[1] * scaleSs[1];
        }

        if (dimension > 2) {
            if (mask & NEIGHBOR_ZM_BOUNDARY && feature->getZmBoundaryType() == BOUNDARY_FLUX){
                boundaryCondition += varContext->evaluateExpression(BOUNDARY_ZM_EXP, statePointValues) * oneOverH[2] * scaleSs[2];
            }
            if (mask & NEIGHBOR_ZP_BOUNDARY && feature->getZpBoundaryType() == BOUNDARY_FLUX){
                boundaryCondition -= varContext->evaluateExpression(BOUNDARY_ZP_EXP, statePointValues) * oneOverH[2] * scaleSs[2];
            }
        }
    }
    return boundaryCondition;
}

// scale cross sectional area scale
void computeScaleS(int mask, double* scaleS) {
    if (mask & NEIGHBOR_X_BOUNDARY_MASK){
        scaleS[1] /= 2; // Y
        scaleS[2] /= 2; // Z
    }
    if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
        scaleS[0] /= 2; // X
        scaleS[2] /= 2; // Z
    }
    if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
        scaleS[0] /= 2; // X
        scaleS[1] /= 2; // Y
    }
}

// order is XP, YP, ZP
#define defineVolumeNeighbors(volIndex, mask) int volumeNeighbors[3] = { \
									(mask & NEIGHBOR_XP_MASK) ? -1 : volIndex + 1, \
									(dimension < 2 || mask & NEIGHBOR_YP_MASK) ? -1 : volIndex + Nx, \
									(dimension < 3 || mask & NEIGHBOR_ZP_MASK) ? -1 : volIndex + Nxy \
								};


// now we update volume flux symmetrically because D*(Uj - Ui)*S/d is the same (opposite sign) for point i and j.
// but what we computed is D*(Uj - Ui)*S/(d * dV) which will be the same for both too, but at the end, we have to
// scale it with volume scale. So now we only have to process neighbor points whose indexes are bigger.
// because we have to scale the flux, we have to make sure
// 1. we process points from small global index to big global index (we checked this)
// 2. we always add reaction at the end because reaction term doesn't have dV (otherwise we need another loop to process reaction).

void SundialsPdeScheduler::regionApplyVolumeOperatorConstant(int regionID, double t, double* yinput, double* rhs) {
    if (regionDefinedVolVariableSizes[regionID] == 0) {
        return;
    }

    int firstPointVolIndex = local2Global[regionOffsets[regionID]];
    Feature* feature = pVolumeElement[firstPointVolIndex].getFeature();
    double (*advectTerms)[6] = 0;
    if (bHasAdvection) {
        advectTerms = new double[regionDefinedVolVariableSizes[regionID]][6];
        memset(advectTerms, 0, regionDefinedVolVariableSizes[regionID] * 6 * sizeof(double));

        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[regionID]; activeVarCount ++) {
            int varIndex = regionDefinedVolVariableIndexes[regionID][activeVarCount];
            VolumeVariable* var = simulation->getVolVariable(varIndex);

            if (var->isAdvecting()) {

                VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);
                double D = varContext->evaluateConstantExpression(DIFF_RATE_EXP);

                double Vi_XYZ[3] = {0, 0, 0};
                Vi_XYZ[0] = varContext->evaluateConstantExpression(VELOCITY_X_EXP);
                if (dimension > 1) {
                    Vi_XYZ[1] = varContext->evaluateConstantExpression(VELOCITY_Y_EXP);
                }
                if (dimension > 2) {
                    Vi_XYZ[2] = varContext->evaluateConstantExpression(VELOCITY_Z_EXP);
                }

                for (int n = 0; n < 3; n ++) {
                    double diffTerm = D * oneOverH[n];
                    double diffAdvectTerm = 0;
                    double V = Vi_XYZ[n];
                    double advectTerm = -V;

                    double Aii = 0, Aij = 0;
                    applyAdvectionHybridScheme(diffTerm, advectTerm, Aii, Aij);
                    advectTerms[activeVarCount][n*2] = Aii;
                    advectTerms[activeVarCount][n*2 + 1] = Aij;
                } // end for n
            }
        }
    }

    // loop through points
    for (int regionPointIndex = 0; regionPointIndex < regionSizes[regionID]; regionPointIndex ++) {
        int localIndex = regionPointIndex + regionOffsets[regionID];
        int volIndex = local2Global[localIndex];
        int mask = pVolumeElement[volIndex].neighborMask;

        bool bDirichlet = false;
        double scaleS[3] = {1, 1, 1};
        if (mask & NEIGHBOR_BOUNDARY_MASK) {
            if (mask & BOUNDARY_TYPE_DIRICHLET) {// dirichlet
                bDirichlet = true;
            }

            computeScaleS(mask, scaleS);
        }

        // Macro
        defineVolumeNeighbors(volIndex, mask)

        // update values for this point
        updateVolumeStatePointValues(volIndex, t, yinput, statePointValues);

        int vectorIndexOffset = getVolumeElementVectorOffset(volIndex, regionID);

        // loop through defined variables
        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[regionID]; activeVarCount ++) {
            // I can also increment this, but
            int vectorIndex = vectorIndexOffset + activeVarCount;

            int varIndex = regionDefinedVolVariableIndexes[regionID][activeVarCount];
            VolumeVariable* var = (VolumeVariable*)simulation->getVolVariable(varIndex);
            VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);

            double reactionRate = 0;
            if (bDirichlet && var->isDiffusing()) {// pde dirichlet
                rhs[vectorIndex] = 0;
                if (mask & (NEIGHBOR_XP_BOUNDARY | NEIGHBOR_YP_BOUNDARY | NEIGHBOR_ZP_BOUNDARY)) {
                    continue;
                }
            } else {
                reactionRate = varContext->evaluateExpression(REACT_RATE_EXP, statePointValues);
                if (!var->isDiffusing()) {
                    rhs[vectorIndex] += reactionRate;
                    continue;
                }
            }

            double boundaryCondition = 0;
            if (!bDirichlet && (mask & NEIGHBOR_BOUNDARY_MASK)) {   // neumann boundary condition
                if ((mask & BOUNDARY_TYPE_MASK) == BOUNDARY_TYPE_NEUMANN) {
                    boundaryCondition = computeNeumannCondition(feature, varContext, mask, scaleS);
                }
            }

            double D = varContext->evaluateConstantExpression(DIFF_RATE_EXP);

            double ypoint = yinput[vectorIndex];
            if (bDirichlet) {
                dirichletPointSetup(volIndex, feature, varContext, mask, volumeNeighbors, ypoint);
            }

            // add diffusion and convection
            for (int n = 0; n < 3; n ++) {
                int neighborIndex = volumeNeighbors[n];
                if (neighborIndex < 0) {
                    continue;
                }

                int neighborVectorIndex = getVolumeElementVectorOffset(neighborIndex, regionID) + activeVarCount;
                double yneighbor = yinput[neighborVectorIndex];

                //use the real value if the neighbor is dirichlet point
                int neighborMask = pVolumeElement[neighborIndex].neighborMask;
                bool bNeighborDirichlet = false;
                if (neighborMask & BOUNDARY_TYPE_DIRICHLET) {
                    bNeighborDirichlet = true;
                    updateVolumeStatePointValues(neighborIndex, t, 0, txyzValues);
                    yneighbor = varContext->evaluateExpression(dirichletExpIndexes[n], txyzValues);
                }

                double diffAdvectTerm = 0;
                if (var->isAdvecting()) {
                    diffAdvectTerm = yneighbor * advectTerms[activeVarCount][n * 2 + 1] - ypoint * advectTerms[activeVarCount][n * 2];
                } else {
                    diffAdvectTerm = (yneighbor - ypoint) * D * oneOverH[n];
                }
                diffAdvectTerm *= scaleS[n] * oneOverH[n];
                if (!bDirichlet) {
                    rhs[vectorIndex] += diffAdvectTerm;
                }
                if (!bNeighborDirichlet) {
                    rhs[neighborVectorIndex] -= diffAdvectTerm;
                }
            } // end for n
            if (bDirichlet) {
                rhs[vectorIndex] = 0;
            } else {
                // add boundary conditon
                rhs[vectorIndex] += boundaryCondition;
                if (mask & NEIGHBOR_BOUNDARY_MASK){
                    // apply volume scale
                    rhs[vectorIndex] *= (mask&VOLUME_MASK);
                }
                // add reaction
                rhs[vectorIndex] += reactionRate;
            }
        } // end for v
    } // end for ri
    delete[] advectTerms;
}

void SundialsPdeScheduler::precomputeDiffusionCoefficients() {
    if (simulation->getNumVolVariables() == 0) {
        return;
    }

    diffCoeffs = NULL;
    for (int regionIndex = 0; regionIndex < mesh->getNumVolumeRegions(); regionIndex ++) {
        int firstPointVolIndex = local2Global[regionOffsets[regionIndex]];
        Feature* feature = pVolumeElement[firstPointVolIndex].getFeature();

        // loop through defined variables
        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[regionIndex]; activeVarCount ++) {

            int varIndex = regionDefinedVolVariableIndexes[regionIndex][activeVarCount];
            VolumeVariable* var = simulation->getVolVariable(varIndex);
            if (!var->isDiffusing()) {
                continue;
            }
            VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);
            if (!varContext->hasXYZOnlyDiffusion()) {
                continue;
            }

            if (diffCoeffs == NULL) {
                cout << "precomputing diffusion constants" << endl;
                diffCoeffs = new double[numUnknowns];
                memset(diffCoeffs, 0, numUnknowns * sizeof(double));
            }

            // loop through points
            for (int regionPointIndex = 0; regionPointIndex < regionSizes[regionIndex]; regionPointIndex ++) {
                int localIndex = regionPointIndex + regionOffsets[regionIndex];
                int volIndex = local2Global[localIndex];

                // xyz dependent only
                updateVolumeStatePointValues(volIndex, 0, 0, statePointValues);

                int vectorIndexOffset = getVolumeElementVectorOffset(volIndex, regionIndex);
                int vectorIndex = vectorIndexOffset + activeVarCount;

                diffCoeffs[vectorIndex] = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
            } // point
        } // var
    } // region
}

void SundialsPdeScheduler::regionApplyVolumeOperatorVariable(int regionID, double t, double* yinput, double* rhs) {
    if (regionDefinedVolVariableSizes[regionID] == 0) {
        return;
    }

    int firstPointVolIndex = local2Global[regionOffsets[regionID]];
    Feature* feature = pVolumeElement[firstPointVolIndex].getFeature();

    // loop through points
    for (int regionPointIndex = 0; regionPointIndex < regionSizes[regionID]; regionPointIndex ++) {
        int localIndex = regionPointIndex + regionOffsets[regionID];
        int volIndex = local2Global[localIndex];
        int mask = pVolumeElement[volIndex].neighborMask;

        bool bDirichlet = false;
        double scaleS[3] = {1, 1, 1};
        if (mask & NEIGHBOR_BOUNDARY_MASK) {
            if (mask & BOUNDARY_TYPE_DIRICHLET) {// dirichlet
                bDirichlet = true;
            }

            computeScaleS(mask, scaleS);
        }

        // Macro
        defineVolumeNeighbors(volIndex, mask)

        // update values for this point
        updateVolumeStatePointValues(volIndex, t, yinput, statePointValues);

#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
        if (bRegionHasTimeDepdentVariables[regionID]) {
#endif
            // update values for neighbor points
            for (int n = 0; n < 3; n ++) {
                int neighborIndex = volumeNeighbors[n];
                if (neighborIndex < 0) {
                    continue;
                }
                updateVolumeStatePointValues(neighborIndex, t, yinput, neighborStatePointValues[n]);
            }
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
        }
#endif

        int vectorIndexOffset = getVolumeElementVectorOffset(volIndex, regionID);

        // loop through defined variables
        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[regionID]; activeVarCount ++) {
            // I can also increment this, but
            int vectorIndex = vectorIndexOffset + activeVarCount;

            int varIndex = regionDefinedVolVariableIndexes[regionID][activeVarCount];
            VolumeVariable* var = simulation->getVolVariable(varIndex);
            VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);

            double reactionRate = 0;
            if (bDirichlet && var->isDiffusing()) {// pde dirichlet
                rhs[vectorIndex] = 0;
                if (mask & (NEIGHBOR_XP_BOUNDARY | NEIGHBOR_YP_BOUNDARY | NEIGHBOR_ZP_BOUNDARY)) {
                    continue;
                }
            } else {
                reactionRate = varContext->evaluateExpression(REACT_RATE_EXP, statePointValues);
                if (!var->isDiffusing()) {
                    rhs[vectorIndex] += reactionRate;
                    continue;
                }
            }

            double boundaryCondition = 0;
            if (!bDirichlet && (mask & NEIGHBOR_BOUNDARY_MASK)) {   // neumann boundary condition
                if ((mask & BOUNDARY_TYPE_MASK) == BOUNDARY_TYPE_NEUMANN) {
                    boundaryCondition = computeNeumannCondition(feature, varContext, mask, scaleS);
                }
            }

            double Di = 0;
            if (varContext->hasConstantDiffusion()) {
                Di = varContext->evaluateConstantExpression(DIFF_RATE_EXP);
            } else {
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                if (varContext->hasXYZOnlyDiffusion()) {
                    Di = diffCoeffs[vectorIndex];
                } else {
#endif
                    Di = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                }
#endif
            }

            double Vi_XYZ[3] = {0, 0, 0};
            if (var->isAdvecting()) {
                Vi_XYZ[0] = varContext->evaluateExpression(VELOCITY_X_EXP, statePointValues);
                if (dimension > 1) {
                    Vi_XYZ[1] = varContext->evaluateExpression(VELOCITY_Y_EXP, statePointValues);
                }
                if (dimension > 2) {
                    Vi_XYZ[2] = varContext->evaluateExpression(VELOCITY_Z_EXP, statePointValues);
                }
            }

            double ypoint = yinput[vectorIndex];
            if (bDirichlet) {
                dirichletPointSetup(volIndex, feature, varContext, mask, volumeNeighbors, ypoint);
            }

            // add diffusion and convection
            for (int n = 0; n < dimension; n ++) {
                int neighborIndex = volumeNeighbors[n];
                int neighborMask = neighborIndex < 0 ? 0 : pVolumeElement[neighborIndex].neighborMask;
                int neighborVectorIndex  = neighborIndex < 0 ? -1 : getVolumeElementVectorOffset(neighborIndex, regionID) + activeVarCount;
                bool bNeighborDirichlet = (neighborMask & BOUNDARY_TYPE_DIRICHLET);

                if (varContext->hasGradient(n)) {
                    double gradi = varContext->evaluateExpression(gradExpIndexes[n], statePointValues);

                    if (mask & minusMasks[n]) { // no minus neighbor
                        double grad_near = varContext->evaluateExpression(gradExpIndexes[n], neighborStatePointValues[n]);
                        if (!bDirichlet) {
                            rhsGradients[vectorIndex] += (-gradi + grad_near) * oneOverH[n];
                        }
                        if (!bNeighborDirichlet && neighborVectorIndex>=0) {
                            rhsGradients[neighborVectorIndex] -= gradi * oneOverH[n] / 2;
                        }
                    } else if (neighborIndex < 0) { // no plus neighbor
                        if (!bDirichlet) {
                            rhsGradients[vectorIndex] += gradi * oneOverH[n];
                        }
                    } else {
                        double gradj = varContext->evaluateExpression(gradExpIndexes[n], neighborStatePointValues[n]);
                        if (!bDirichlet) {
                            rhsGradients[vectorIndex] += gradj * oneOverH[n] / 2;
                        }
                        if (!bNeighborDirichlet) {
                            if (neighborMask & plusMasks[n]) {
                                rhsGradients[neighborVectorIndex] -= gradi * oneOverH[n]; //low order
                            } else {
                                rhsGradients[neighborVectorIndex] -= gradi * oneOverH[n] / 2;
                            }
                        }
                    }
                }

                if (neighborIndex < 0) {
                    continue;
                }

                double D = Di;
                if (!varContext->hasConstantDiffusion()) {
                    double Dj = 0;
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                    if (varContext->hasXYZOnlyDiffusion()) {
                        Dj = diffCoeffs[neighborVectorIndex];
                    } else {
#endif
                        Dj = varContext->evaluateExpression(DIFF_RATE_EXP, neighborStatePointValues[n]);
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                    }
#endif
                    D = (Di + Dj < epsilon) ? (0.0) : (2 * Di * Dj/(Di + Dj));
                }

                double yneighbor = yinput[neighborVectorIndex];

                //use the real value if the neighbor is dirichlet point
                if (bNeighborDirichlet) {
                    yneighbor = varContext->evaluateExpression(dirichletExpIndexes[n], neighborStatePointValues[n]);
                }

                double diffTerm = D * oneOverH[n];
                double diffAdvectTerm = 0;
                if (var->isAdvecting()) {
                    double Vi = Vi_XYZ[n];
                    double Vj = varContext->evaluateExpression(velocityExpIndexes[n], neighborStatePointValues[n]);
                    double V = 0.5 * (Vi + Vj);
                    double advectTerm = -V;

                    double Aii = 0, Aij = 0;
                    applyAdvectionHybridScheme(diffTerm, advectTerm, Aii, Aij);
                    diffAdvectTerm = yneighbor * Aij - ypoint * Aii;
                } else {
                    diffAdvectTerm = (yneighbor - ypoint) * diffTerm;
                }
                diffAdvectTerm *= scaleS[n] * oneOverH[n];
                if (!bDirichlet) {
                    rhs[vectorIndex] += diffAdvectTerm;
                }
                if (!bNeighborDirichlet) {
                    rhs[neighborVectorIndex] -= diffAdvectTerm;
                }
            } // end for n
            if (bDirichlet) {
                rhs[vectorIndex] = 0;
            } else {
                // add boundary conditon
                rhs[vectorIndex] += boundaryCondition;
                if (mask & NEIGHBOR_BOUNDARY_MASK){
                    // apply volume scale
                    rhs[vectorIndex] *= (mask&VOLUME_MASK);
                }
                // add reaction
                rhs[vectorIndex] += reactionRate;
            }
        } // end for v
    } // end for ri

    if (bHasGradient) {
        double a = 1.0;
        int increment = 1;
        DAXPY(&numUnknowns, &a, rhsGradients, &increment, rhs, &increment);
        //for (int i = 0; i < numUnknowns; i ++) {
        //	int xi = 11 + ((i % 20)/2);

        //	double PI = 3.141592653589793;
        //	double gradExact = PI * exp(-PI*PI*t) * cos(PI*(-1+xi/oneOverH[0]));
        //	if (i%2==1) {
        //		cout << "grad[" << i << "," << xi << "]=" << rhsGradients[i] << " " << gradExact << " " << gradExact - rhsGradients[i] << endl;
        //	}
        //	rhs[i] += rhsGradients[i];
        //}
    }
}

void SundialsPdeScheduler::applyMembraneDiffusionReactionOperator(double t, double* yinput, double* rhs) {
    if (simulation->getNumMemVariables() == 0) {
        return;
    }

    for (int mi = 0; mi < mesh->getNumMembraneElements(); mi ++) {
        Membrane* membrane = pMembraneElement[mi].getMembrane();
        for (int v = 0; v < simulation->getNumMemVariables(); v ++) {
            MembraneVariable* var = (MembraneVariable*)simulation->getMemVariable(v);
            // variable is not defined on this membrane
            if (var->getStructure() != NULL && var->getStructure() != membrane) {
                continue;
            }

            int vectorIndex = getMembraneElementVectorOffset(mi) + v;
            MembraneVarContextExpression *varContext = membrane->getMembraneVarContext(var);
            int mask = mesh->getMembraneNeighborMask(mi);

            if (!var->isDiffusing() || !(mask & BOUNDARY_TYPE_DIRICHLET)) {   // boundary and dirichlet
                // update values
                updateMembraneStatePointValues(pMembraneElement[mi], t, yinput, statePointValues);
                double reactionRate = varContext->evaluateExpression(REACT_RATE_EXP, statePointValues);
                // add reaction
                rhs[vectorIndex] += reactionRate;
            }

            if (!var->isDiffusing()) {
                //validateNumber(var->getName(), volIndex, "RHS", rhs[vectorIndex]);
                continue;
            }

            if (mask & BOUNDARY_TYPE_DIRICHLET) {   // boundary and dirichlet
                rhs[vectorIndex] = 0;
                continue;
            }

            double Di = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
            double volume = mesh->getMembraneCoupling()->getValue(mi, mi);
            if (mask & BOUNDARY_TYPE_NEUMANN) { // boundary and neumann
                if (dimension == 2) { // all the flux area is 1
                    if (mask & NEIGHBOR_XM_BOUNDARY){
                        rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues)/volume;
                    }
                    if (mask & NEIGHBOR_XP_BOUNDARY){
                        rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_XP_EXP, statePointValues)/volume;
                    }
                    if (mask & NEIGHBOR_YM_BOUNDARY){
                        rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues)/volume;
                    }
                    if (mask & NEIGHBOR_YP_BOUNDARY){
                        rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_YP_EXP, statePointValues)/volume;
                    }
                } else if (dimension == 3) {
                    double* boundaryFluxArea = mesh->getMembraneFluxArea(mi);

                    if (mask & NEIGHBOR_XM_BOUNDARY){
                        rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues) * boundaryFluxArea[BL_Xm]/volume;
                    }
                    if (mask & NEIGHBOR_XP_BOUNDARY){
                        rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_XP_EXP, statePointValues) * boundaryFluxArea[BL_Xp]/volume;
                    }
                    if (mask & NEIGHBOR_YM_BOUNDARY){
                        rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues) * boundaryFluxArea[BL_Ym]/volume;
                    }
                    if (mask & NEIGHBOR_YP_BOUNDARY){
                        rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_YP_EXP, statePointValues) * boundaryFluxArea[BL_Yp]/volume;
                    }
                    if (mask & NEIGHBOR_ZM_BOUNDARY){
                        rhs[vectorIndex] += varContext->evaluateExpression(BOUNDARY_ZM_EXP, statePointValues) * boundaryFluxArea[BL_Zm]/volume;
                    }
                    if (mask & NEIGHBOR_ZP_BOUNDARY){
                        rhs[vectorIndex] -= varContext->evaluateExpression(BOUNDARY_ZP_EXP, statePointValues) * boundaryFluxArea[BL_Zp]/volume;
                    }
                }
            }

            int32* membraneNeighbors;
            double* s_over_d;
            int numMembraneNeighbors = mesh->getMembraneCoupling()->getColumns(mi, membraneNeighbors, s_over_d);

            double Aii = 0;
            for (long j = 0; j < numMembraneNeighbors; j ++) {
                int32 neighborIndex = membraneNeighbors[j];
                int neighborVectorIndex = getMembraneElementVectorOffset(neighborIndex) + v;

                updateMembraneStatePointValues(pMembraneElement[neighborIndex], t, yinput, statePointValues);
                double Dj = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
                double D = (Di + Dj < epsilon)?(0.0):(2 * Di * Dj/(Di + Dj));

                double yneighbor = yinput[neighborVectorIndex];
                int neighborMask = mesh->getMembraneNeighborMask(neighborIndex);
                if (neighborMask & BOUNDARY_TYPE_DIRICHLET) {
                    if ((neighborMask & NEIGHBOR_XM_BOUNDARY) && (membrane->getXmBoundaryType() == BOUNDARY_VALUE)){
                        yneighbor = varContext->evaluateExpression(BOUNDARY_XM_EXP, statePointValues);
                    }else if ((neighborMask & NEIGHBOR_XP_BOUNDARY) && (membrane->getXpBoundaryType() == BOUNDARY_VALUE)){
                        yneighbor = varContext->evaluateExpression(BOUNDARY_XP_EXP, statePointValues);
                    } else if ((neighborMask & NEIGHBOR_YM_BOUNDARY) && (membrane->getYmBoundaryType() == BOUNDARY_VALUE)){
                        yneighbor = varContext->evaluateExpression(BOUNDARY_YM_EXP, statePointValues);
                    }else if ((neighborMask & NEIGHBOR_YP_BOUNDARY) && (membrane->getYpBoundaryType() == BOUNDARY_VALUE)){
                        yneighbor = varContext->evaluateExpression(BOUNDARY_YP_EXP, statePointValues);
                    }else if ((neighborMask & NEIGHBOR_ZM_BOUNDARY) && (membrane->getZmBoundaryType() == BOUNDARY_VALUE)){
                        yneighbor = varContext->evaluateExpression(BOUNDARY_ZM_EXP, statePointValues);
                    }else if ((neighborMask & NEIGHBOR_ZP_BOUNDARY) && (membrane->getZpBoundaryType() == BOUNDARY_VALUE)){
                        yneighbor = varContext->evaluateExpression(BOUNDARY_ZP_EXP, statePointValues);
                    }
                }
                double Aij = D * s_over_d[j] / volume;
                Aii += Aij;
                rhs[vectorIndex] += yneighbor * Aij;
            }
            rhs[vectorIndex] -=  yinput[vectorIndex] * Aii;
        }
    }
}

void SundialsPdeScheduler::applyVolumeRegionReactionOperator(double t, double* yinput, double* rhs) {
    if (simulation->getNumVolRegionVariables() == 0) {
        return;
    }

    for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
        VolumeRegion* volRegion = mesh->getVolumeRegion(r);
        Feature* feature = volRegion->getFeature();
        for (int v = 0; v < simulation->getNumVolRegionVariables(); v ++) {
            VolumeRegionVariable* var = simulation->getVolRegionVariable(v);
            VolumeRegionVarContextExpression* volRegionVarContext = feature->getVolumeRegionVarContext(var);
            if (volRegionVarContext == 0) {
                continue;
            }
            int vectorIndex = getVolumeRegionVectorOffset(r) + v;

            updateRegionStatePointValues(r, t, yinput, true, statePointValues);
            rhs[vectorIndex] = volRegionVarContext->evaluateExpression(UNIFORM_RATE_EXP, statePointValues);

            double volume = volRegion->getSize();
            int numElements = volRegion->getNumElements();
            double volumeIntegral = 0.0;
            for(int j = 0; j < numElements; j ++){
                int volIndex = volRegion->getElementIndex(j);
                updateVolumeStatePointValues(volIndex, t, yinput, statePointValues);
                volumeIntegral += volRegionVarContext->evaluateExpression(REACT_RATE_EXP, statePointValues) * mesh->getVolumeOfElement_cu(volIndex);
            }
            rhs[vectorIndex] += volumeIntegral/volume;

            int numMembraneRegions = volRegion->getNumMembraneRegions();
            double surfaceIntegral = 0.0;
            for(int k = 0; k < numMembraneRegions; k ++){
                MembraneRegion *memRegion = volRegion->getMembraneRegion(k);
                numElements = memRegion->getNumElements();
                for(int j = 0; j < numElements; j ++){
                    int memIndex = memRegion->getElementIndex(j);
                    updateMembraneStatePointValues(pMembraneElement[memIndex], t, yinput, statePointValues);
                    surfaceIntegral += volRegionVarContext->evaluateJumpCondition(&pMembraneElement[memIndex], statePointValues) * pMembraneElement[memIndex].area;
                }
            }
            rhs[vectorIndex] += surfaceIntegral/volume;
        }
    }
}

void SundialsPdeScheduler::applyMembraneRegionReactionOperator(double t, double* yinput, double* rhs) {
    if (simulation->getNumMemRegionVariables() == 0) {
        return;
    }
    for (int r = 0; r < mesh->getNumMembraneRegions(); r ++) {
        MembraneRegion *memRegion = mesh->getMembraneRegion(r);
        Membrane* membrane = memRegion->getMembrane();
        for (int v = 0; v < simulation->getNumMemRegionVariables(); v ++) {
            MembraneRegionVariable* var = simulation->getMemRegionVariable(v);
            MembraneRegionVarContextExpression * memRegionvarContext = membrane->getMembraneRegionVarContext(var);
            if (memRegionvarContext == 0) {
                continue;
            }
            int vectorIndex = getMembraneRegionVectorOffset(r) + v;

            updateRegionStatePointValues(r, t, yinput, false, statePointValues);
            rhs[vectorIndex] = memRegionvarContext->evaluateExpression(UNIFORM_RATE_EXP, statePointValues);

            double surface = memRegion->getSize();
            long numElements = memRegion->getNumElements();
            double surfaceIntegral = 0.0;
            for(int j = 0; j < numElements; j ++) {
                int memIndex = memRegion->getElementIndex(j);
                updateMembraneStatePointValues(pMembraneElement[memIndex], t, yinput, statePointValues);
                surfaceIntegral += memRegionvarContext->evaluateExpression(REACT_RATE_EXP, statePointValues) * pMembraneElement[memIndex].area;
            }
            rhs[vectorIndex] += surfaceIntegral/surface;
        }
    }
}

void SundialsPdeScheduler::applyMembraneFluxOperator(double t, double* yinput, double* rhs) {
    if (simulation->getNumVolPde() == 0) {
        return;
    }

    for (int m = 0; m < mesh->getNumMembraneElements(); m ++) {
        MembraneElement& me = pMembraneElement[m];

        int loRegionID = pVolumeElement[me.vindexFeatureLo].getRegionIndex();
        int hiRegionID = pVolumeElement[me.vindexFeatureHi].getRegionIndex();

        double loVolume = mesh->getVolumeOfElement_cu(me.vindexFeatureLo);
        double hiVolume = mesh->getVolumeOfElement_cu(me.vindexFeatureHi);

        // vector index of inside near and outside near
        int vi2 = getVolumeElementVectorOffset(me.vindexFeatureLo, loRegionID);
        int vi3 = getVolumeElementVectorOffset(me.vindexFeatureHi, hiRegionID);

        updateMembraneStatePointValues(me, t, yinput, statePointValues);

        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[loRegionID]; activeVarCount ++) {
            int varIndex = regionDefinedVolVariableIndexes[loRegionID][activeVarCount];
            VolumeVariable* var = simulation->getVolVariable(varIndex);
            if (!var->isDiffusing()) {
                continue;
            }

            VolumeVarContextExpression* varContext = pVolumeElement[me.vindexFeatureLo].getFeature()->getVolumeVarContext(var);
            double flux = varContext->evaluateJumpCondition(&me, statePointValues);
            rhs[vi2 + activeVarCount] += flux * me.area / loVolume;
            //validateNumber(var->getName(), me.insideIndexNear, "Membrane Flux", rhs[vi2 + activeVarCountInside]);
        }
        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[hiRegionID]; activeVarCount ++) {
            int varIndex = regionDefinedVolVariableIndexes[hiRegionID][activeVarCount];
            VolumeVariable* var = simulation->getVolVariable(varIndex);
            if (!var->isDiffusing()) {
                continue;
            }

            VolumeVarContextExpression* varContext = pVolumeElement[me.vindexFeatureHi].getFeature()->getVolumeVarContext(var);
            double flux = varContext->evaluateJumpCondition(&me, statePointValues);
            rhs[vi3 + activeVarCount] += flux * me.area / hiVolume;
            //validateNumber(var->getName(), me.outsideIndexNear, "Membrane Flux", rhs[vi3 + activeVarCountOutside]);
        }
    }
}

int SundialsPdeScheduler::pcSetup_callback(realtype t, N_Vector y, N_Vector fy, booleantype jok, booleantype *jcurPtr, realtype gamma,
                                           void *P_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3) {
    SundialsPdeScheduler* solver = (SundialsPdeScheduler*)P_data;
    return solver->pcSetup(t, y, fy, jok, jcurPtr, gamma);
}

int SundialsPdeScheduler::pcSolve_callback(realtype t, N_Vector y, N_Vector fy, N_Vector r, N_Vector z, realtype gamma, realtype delta,
                                           int lr, void *P_data, N_Vector tmp) {
    SundialsPdeScheduler* solver = (SundialsPdeScheduler*)P_data;
    return solver->pcSolve(t, y, fy, r, z, gamma, delta, lr);
}

// M approximates I - gamma*J
void SundialsPdeScheduler::preallocateM() {
    if (M != 0) {
        return;
    }

    int GENERAL_MAX_NONZERO_PERROW[4] = {0, 3, 5, 7};

    // initialize a sparse matrix for M
    int numNonZeros = GENERAL_MAX_NONZERO_PERROW[dimension] * numUnknowns;
    if (simulation->getNumMemPde() > 0 && dimension == 3) {
        numNonZeros = GENERAL_MAX_NONZERO_PERROW[dimension] * numVolUnknowns   // volume variables
                      + 20 * mesh->getNumMembraneElements() * simulation->getNumMemVariables()  // membrane variable
                      + mesh->getNumVolumeRegions() * simulation->getNumVolRegionVariables() // vol region variable
                      + mesh->getNumMembraneRegions() * simulation->getNumMemRegionVariables(); // mem region variable
    }
    if (dimension == 1) {
        nsp = numNonZeros * 20;
    } else {
        nsp = numNonZeros * 8;
    }
    try {
        delete[] pcg_workspace;
        pcg_workspace = new double[nsp];
    } catch (...) {
        throw "SundialsPDESolver:: Out of Memory : pcg_workspace";
    }
    memset(pcg_workspace, 0, nsp * sizeof(double));

    M = new SparseMatrixPCG(numUnknowns, numNonZeros, MATRIX_GENERAL); // only store upper triangle

    int colCount;
    int32 columns[20];

    if (simulation->getNumVolVariables() > 0) {
        for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
            if (regionDefinedVolVariableSizes[r] == 0) {
                continue;
            }

            for (int ri = 0; ri < regionSizes[r]; ri ++) {
                int localIndex = ri + regionOffsets[r];
                int volIndex = local2Global[localIndex];
                int mask = pVolumeElement[volIndex].neighborMask;

                int volumeNeighbors[6] = {
                        (dimension < 3 || mask & NEIGHBOR_ZM_MASK) ? -1 : volIndex - Nxy,
                        (dimension < 2 || mask & NEIGHBOR_YM_MASK) ? -1 : volIndex - Nx,
                        (mask & NEIGHBOR_XM_MASK) ? -1 : volIndex - 1,
                        (mask & NEIGHBOR_XP_MASK) ? -1 : volIndex + 1,
                        (dimension < 2 || mask & NEIGHBOR_YP_MASK) ? -1 : volIndex + Nx,
                        (dimension < 3 || mask & NEIGHBOR_ZP_MASK) ? -1 : volIndex + Nxy
                };

                for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[r]; activeVarCount ++) {
                    int varIndex = regionDefinedVolVariableIndexes[r][activeVarCount];
                    VolumeVariable* var = (VolumeVariable*)simulation->getVolVariable(varIndex);

                    M->addNewRow();

                    if (!var->isDiffusing()) {
                        M->setRow(1.0, 0, 0, 0);
                        //validateNumber(var->getName(), volIndex, "RHS", rhs[vectorIndex]);
                        continue;
                    }

                    if (mask & BOUNDARY_TYPE_DIRICHLET) {// dirichlet
                        M->setRow(1.0, 0, 0, 0);
                        continue;
                    }

                    colCount = 0;
                    // add diffusion and convection
                    for (int n = 0; n < 6; n ++) {
                        int neighborIndex = volumeNeighbors[n];
                        if (neighborIndex < 0) {
                            continue;
                        }
                        int neighborRi= global2Local[neighborIndex] - regionOffsets[r];
                        int neighborVectorIndex = getVolumeElementVectorOffset(neighborIndex, r) + activeVarCount;
                        columns[colCount] = neighborVectorIndex;
                        colCount ++;
                    } // end for n
                    M->setRow(1.0, colCount, columns, 0);
                } // end for vindex
            } // end for ri
        } // end for r
    }

    // membrane variable
    if (simulation->getNumMemVariables() > 0) {
        int columns[20];
        for (int mi = 0; mi < mesh->getNumMembraneElements(); mi ++) {
            for (int v = 0; v < simulation->getNumMemVariables(); v ++) {
                M->addNewRow();

                if (!simulation->getMemVariable(v)->isDiffusing()) {
                    M->setRow(1.0, 0, 0, 0);
                    continue;
                }

                int32* membraneNeighbors;
                double* s_over_d;
                int numMembraneNeighbors = mesh->getMembraneCoupling()->getColumns(mi, membraneNeighbors, s_over_d);
                for (long j = 0; j < numMembraneNeighbors; j ++) {
                    int32 neighborIndex = membraneNeighbors[j];
                    int neighborVectorIndex = getMembraneElementVectorOffset(neighborIndex) + v;
                    columns[j] = neighborVectorIndex;
                }
                M->setRow(1.0, numMembraneNeighbors, columns, 0);
            }
        }
    }
    // volume region variable
    if (simulation->getNumVolRegionVariables() != 0) {
        for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
            for (int v = 0; v < simulation->getNumVolRegionVariables(); v ++) {
                M->addNewRow();
                M->setRow(1.0, 0, 0, 0);
            }
        }
    }
    // membrane region variable
    if (simulation->getNumMemRegionVariables() != 0) {
        for (int r = 0; r < mesh->getNumMembraneRegions(); r ++) {
            for (int v = 0; v < simulation->getNumMemRegionVariables(); v ++) {
                M->addNewRow();
                M->setRow(1.0, 0, 0, 0);
            }
        }
    }
    M->close();
}

int SundialsPdeScheduler::pcSetup(realtype t, N_Vector y, N_Vector fy, booleantype jok, booleantype *jcurPtr, realtype gamma) {
    if (simulation->hasTimeDependentDiffusionAdvection()) { // has time dependent diffusion
        bPcReinit = true;

        double* yinput = NV_DATA_S(y);
        buildM_Volume(t, yinput, gamma);
        buildM_Membrane(t, yinput, gamma);
    } else if (jok) { // can reuse Jacobian data
        bPcReinit = false;
    } else {
        bPcReinit = true;

        // scale M with gamma/oldGamma
        double ratio = gamma/oldGamma;
        M->scaleOffDiagonals(ratio);
        M->shiftDiagonals(ratio);
        oldGamma = gamma;
    }
    *jcurPtr = bPcReinit;

    return 0;
}

int SundialsPdeScheduler::pcSolve(realtype t, N_Vector y, N_Vector fy, N_Vector r, N_Vector z, realtype gamma, realtype delta, int lr) {
    double* sa = M->getsa();
    int32 *ija = M->getFortranIJA();
    double *r_data = NV_DATA_S(r);
    double *z_data = NV_DATA_S(z);

    if (!bLUcomputed || bPcReinit) {
        int symmetricflg = M->getSymmetricFlag();  // general or symmetric storage format

        int IParm[75];
        memset(IParm, 0, 75 * sizeof(int));

        double RParm[25];
        memset(RParm, 0, 25 * sizeof(double));

        IParm[4] = 1; // max number of iteration
        IParm[13] = 0; // don't reuse all incomplete factorization.
        IParm[14] = 0; // fill-in parameter

        RParm[0] = 0.0;
        RParm[1] = 1.0;

        double pcgTol = 0.1;
        double RHSscale = 1;

        // zero guss
        memset(z_data, 0, numUnknowns * sizeof(double));
        int numRetries = 0; // retry twice
        while (true) {
            bool bRetry = false;
            PCGWRAPPER(&numUnknowns, &nsp, &symmetricflg, ija, sa, r_data, z_data, &pcgTol, IParm, RParm, pcg_workspace, pcg_workspace, &RHSscale);
            if (IParm[50] != 0) {
                switch (IParm[50]) {
                    case 2:
                    case 3:
                    case 4:
                    case 9:
                    case 10:
                    case 15:
                        if (numRetries == 2) {
                            throwPCGExceptions(IParm[50], IParm[53]);
                        } else {
                            try {
                                cout << endl << "!!Note: Insufficient PCG workspace (" << nsp << "), need additional (" << IParm[53] << "), try again" << endl;
                                delete[] pcg_workspace;
                                nsp += IParm[53];
                                pcg_workspace = new double[nsp];
                                memset(pcg_workspace, 0, nsp * sizeof(double));
                                numRetries ++;
                                bRetry = true;
                            } catch (...) {
                                char errMsg[128];
                                snprintf(errMsg, 128, "SundialsPDESolver:: Out of Memory : pcg_workspace allocating (%ld)", nsp);
                                throw errMsg;
                            }
                        }
                        break;
                    case 1:	// maximum iterations reached without satisfying stopping criterion
                    case 8: // stagnant
                        // ignore these two errors.
                        break;
                    default:
                        throwPCGExceptions(IParm[50], IParm[53]);
                        break;
                }
            }
            if (!bRetry) {
                break;
            }
        }

        bPcReinit = false;
        if (IParm[54] == 1) {
            bLUcomputed = true;
        } else {
            bLUcomputed = false;
        }
    } else {
        int icode = -3;
        memcpy(z_data, r_data, numUnknowns * sizeof(double));
        PCILU(&icode, &numUnknowns, ija, sa, z_data, pcg_workspace, pcg_workspace);
    }

    return 0;
}

// M approximates I - gamma*J
void SundialsPdeScheduler::buildM_Volume(double t, double* yinput, double gamma) {
    if (simulation->getNumVolVariables() == 0) {
        return;
    }

    int colCount;
    int32* columns;
    double* values;

    for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[r]; activeVarCount ++) {
            int varIndex = regionDefinedVolVariableIndexes[r][activeVarCount];
            VolumeVariable* var = (VolumeVariable*)simulation->getVolVariable(varIndex);

            if (!var->isDiffusing()) {
                continue;
            }
            int firstPointVolIndex = local2Global[regionOffsets[r]];
            Feature* feature = pVolumeElement[firstPointVolIndex].getFeature();
            VolumeVarContextExpression* varContext = feature->getVolumeVarContext(var);
            double Di = 0;
            if (varContext->hasConstantDiffusion()) {
                Di = varContext->evaluateConstantExpression(DIFF_RATE_EXP);
            }
            for (int ri = 0; ri < regionSizes[r]; ri ++) {
                int localIndex = ri + regionOffsets[r];
                int volIndex = local2Global[localIndex];
                int mask = pVolumeElement[volIndex].neighborMask;
                if (mask & BOUNDARY_TYPE_DIRICHLET) {// dirichlet
                    continue;
                }

                int vectorIndex = getVolumeElementVectorOffset(volIndex, r) + activeVarCount;
                assert(varContext);

                double lambdaX = oneOverH[0] * oneOverH[0];
                double lambdaY = oneOverH[1] * oneOverH[1];
                double lambdaZ = oneOverH[2] * oneOverH[2];
                double lambdaAreaX = oneOverH[0];
                double lambdaAreaY = oneOverH[1];
                double lambdaAreaZ = oneOverH[2];

                // update values
                if (!varContext->hasConstantDiffusion()) {
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                    if (varContext->hasXYZOnlyDiffusion()) {
                        Di = diffCoeffs[vectorIndex];
                    } else {
#endif
                        updateVolumeStatePointValues(volIndex, t, yinput, statePointValues);
                        Di = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                    }
#endif
                }

                double Aii = 0;
                if (mask & NEIGHBOR_BOUNDARY_MASK){   // boundary
                    if (mask & NEIGHBOR_X_BOUNDARY_MASK){
                        lambdaX *= 2.0;
                        lambdaAreaX *= 2.0;
                    }
                    if (mask & NEIGHBOR_Y_BOUNDARY_MASK){
                        lambdaY *= 2.0;
                        lambdaAreaY *= 2.0;
                    }
                    if (mask & NEIGHBOR_Z_BOUNDARY_MASK){
                        lambdaZ *= 2.0;
                        lambdaAreaZ *= 2.0;
                    }
                }

                int volumeNeighbors[6] = {
                        (dimension < 3 || mask & NEIGHBOR_ZM_MASK) ? -1 : volIndex - Nxy,
                        (dimension < 2 || mask & NEIGHBOR_YM_MASK) ? -1 : volIndex - Nx,
                        (mask & NEIGHBOR_XM_MASK) ? -1 : volIndex - 1,
                        (mask & NEIGHBOR_XP_MASK) ? -1 : volIndex + 1,
                        (dimension < 2 || mask & NEIGHBOR_YP_MASK) ? -1 : volIndex + Nx,
                        (dimension < 3 || mask & NEIGHBOR_ZP_MASK) ? -1 : volIndex + Nxy
                };

                double neighborLambdas[6] = {lambdaZ, lambdaY, lambdaX, lambdaX, lambdaY, lambdaZ};
                double neighborLambdaAreas[6] = {lambdaAreaZ, lambdaAreaY, lambdaAreaX, lambdaAreaX, lambdaAreaY, lambdaAreaZ};

                int numColumns = M->getColumns(vectorIndex, columns, values);
                colCount = 0;
                for (int n = 0; n < 6; n ++) {
                    int neighborIndex = volumeNeighbors[n];
                    if (neighborIndex < 0) {
                        continue;
                    }
                    int neighborVectorIndex = getVolumeElementVectorOffset(neighborIndex, r) + activeVarCount;
                    double lambda = neighborLambdas[n];

                    double D = Di;
                    if (!varContext->hasConstantDiffusion()) {
                        double Dj = 0;
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                        if (varContext->hasXYZOnlyDiffusion()) {
                            Dj = diffCoeffs[neighborVectorIndex];
                        } else {
#endif
                            updateVolumeStatePointValues(neighborIndex, t, yinput, statePointValues);
                            Dj = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
#ifdef PRECOMPUTE_DIFFUSION_COEFFICIENT
                        }
#endif
                        D = (Di + Dj < epsilon) ? (0.0) : (2 * Di * Dj/(Di + Dj));
                    }

                    double Aij = 0;
                    Aij = D * lambda;
                    Aii += Aij;
                    assert(columns[colCount] == neighborVectorIndex);
                    values[colCount] = -gamma * Aij;
                    colCount ++;
                } // end for n
                assert(colCount == numColumns);
                M->setDiag(vectorIndex, 1.0 + gamma * Aii);
            } // end for ri
        } // end for v
    } // end for r
}

void SundialsPdeScheduler::buildM_Membrane(double t, double* yinput, double gamma) {
    if (simulation->getNumMemVariables() == 0) {
        return;
    }

    for (int mi = 0; mi < mesh->getNumMembraneElements(); mi ++) {
        Membrane* membrane = pMembraneElement[mi].getMembrane();
        for (int v = 0; v < simulation->getNumMemVariables(); v ++) {
            MembraneVariable* var = (MembraneVariable*)simulation->getMemVariable(v);
            // variable is not defined on this membrane
            if (var->getStructure() != NULL && var->getStructure() != membrane) {
                continue;
            }

            if (!var->isDiffusing()) {
                continue;
            }

            int mask = mesh->getMembraneNeighborMask(mi);
            if ((mask & NEIGHBOR_BOUNDARY_MASK) && (mask & BOUNDARY_TYPE_DIRICHLET)) {   // boundary and dirichlet
                continue;
            }

            int vectorIndex = getMembraneElementVectorOffset(mi) + v;

            int32* columns;
            double* values;
            int numColumns = M->getColumns(vectorIndex, columns, values);

            Membrane* membrane = pMembraneElement[mi].getMembrane();
            MembraneVarContextExpression *varContext = membrane->getMembraneVarContext(var);

            updateMembraneStatePointValues(pMembraneElement[mi], t, yinput, statePointValues);
            double Di = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);

            int32* membraneNeighbors;
            double* s_over_d;
            int numMembraneNeighbors = mesh->getMembraneCoupling()->getColumns(mi, membraneNeighbors, s_over_d);
            assert(numColumns == numMembraneNeighbors);
            double volume = mesh->getMembraneCoupling()->getValue(mi, mi);
            double Aii = 0;
            for (long j = 0; j < numMembraneNeighbors; j ++) {
                int32 neighborIndex = membraneNeighbors[j];
                int neighborVectorIndex = getMembraneElementVectorOffset(neighborIndex) + v;

                updateMembraneStatePointValues(pMembraneElement[neighborIndex], t, yinput, statePointValues);
                double Dj = varContext->evaluateExpression(DIFF_RATE_EXP, statePointValues);
                double D = (Di + Dj < epsilon)?(0.0):(2 * Di * Dj/(Di + Dj));

                assert(columns[j] == neighborVectorIndex);
                double Aij = D * s_over_d[j] / volume;
                values[j] = - gamma * Aij;
                Aii += Aij;
            }
            M->setDiag(vectorIndex, 1.0 + gamma * Aii);
        }
    }
}

int SundialsPdeScheduler::getVolumeElementVectorOffset(int volIndex, int regionID) {
    return volVectorOffsets[regionID] + (global2Local[volIndex] - regionOffsets[regionID]) * regionDefinedVolVariableSizes[regionID];
}

int SundialsPdeScheduler::getMembraneElementVectorOffset(int meindex) {
    return memVectorOffset + meindex * simulation->getNumMemVariables();
}

int SundialsPdeScheduler::getVolumeRegionVectorOffset(int regionID) {
    return volRegionVectorOffset + regionID * simulation->getNumVolRegionVariables();
}

int SundialsPdeScheduler::getMembraneRegionVectorOffset(int regionID) {
    return memRegionVectorOffset + regionID * simulation->getNumMemRegionVariables();
}

void SundialsPdeScheduler::updateVolumeStatePointValues(int volIndex, double t, double* yinput, double* values) {
    values[0] = t;

    WorldCoord wc = mesh->getVolumeWorldCoord(volIndex);

    values[1] = wc.x;
    values[2] = wc.y;
    values[3] = wc.z;

    if (yinput == 0) {
        return;
    }

    simulation->populateRegionSizeVariableValues(values + regionSizeVariableSymbolOffset, true, pVolumeElement[volIndex].getRegionIndex());
    simulation->populateFieldValues(values + fieldDataSymbolOffset, volIndex);
    simulation->populateRandomValues(values + randomVariableSymbolOffset, volIndex);

    int regionID = pVolumeElement[volIndex].getRegionIndex();
    if (simulation->getNumVolVariables() > 0) {
        int vi = getVolumeElementVectorOffset(volIndex, regionID);
        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[regionID]; activeVarCount ++) {
            int varIndex = regionDefinedVolVariableIndexes[regionID][activeVarCount];
            values[volSymbolOffset + varIndex * numSymbolsPerVolVar] = yinput[vi + activeVarCount];
        }
    }

    if (simulation->getNumVolRegionVariables() > 0) {
        // fill in volume region variable values
        int volumeRegionElementVectorOffset = getVolumeRegionVectorOffset(regionID);
        int numVolRegionVariables = simulation->getNumVolRegionVariables();
        for (int varIndex = 0; varIndex < numVolRegionVariables; varIndex ++) {
            values[volRegionSymbolOffset + varIndex * numSymbolsPerVolVar] = yinput[volumeRegionElementVectorOffset + varIndex];
        }
    }
    // if field data is used in expressions other than initial conditions, we
    // need to fill the double array the values from field data from value proxy
    //
    // we also need to fill parameter values for parameter optimization.
}

void SundialsPdeScheduler::updateMembraneStatePointValues(MembraneElement& me, double t, double* yinput, double* values) {

    WorldCoord wc = mesh->getMembraneWorldCoord(&me);

    values[0] = t;
    values[1] = wc.x;
    values[2] = wc.y;
    values[3] = wc.z;

    if (yinput == 0) {
        return;
    }

    simulation->populateRegionSizeVariableValues(values + regionSizeVariableSymbolOffset, false, me.getRegionIndex());
    simulation->populateFieldValues(values + fieldDataSymbolOffset, me.index);
    simulation->populateRandomValues(values + randomVariableSymbolOffset, me.index);

    if (simulation->getNumVolVariables() > 0) {
        // fill in INSIDE and OUTSIDE values
        int loRegionID = pVolumeElement[me.vindexFeatureLo].getRegionIndex();
        int hiRegionID = pVolumeElement[me.vindexFeatureHi].getRegionIndex();

        int vi1 = me.vindexFeatureLoFar < 0 ? -1 : getVolumeElementVectorOffset(me.vindexFeatureLoFar, loRegionID);
        int vi2 = getVolumeElementVectorOffset(me.vindexFeatureLo, loRegionID);
        int vi3 = getVolumeElementVectorOffset(me.vindexFeatureHi, hiRegionID);
        int vi4 = me.vindexFeatureHiFar < 0 ? -1 : getVolumeElementVectorOffset(me.vindexFeatureHiFar, hiRegionID);

        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[loRegionID]; activeVarCount ++) {
            int varIndex = regionDefinedVolVariableIndexes[loRegionID][activeVarCount];

            int iin = volSymbolOffset + varIndex * numSymbolsPerVolVar + 1 + pVolumeElement[me.vindexFeatureLo].getFeature()->getIndex();
            if (vi1 < 0 || sundialsSolverOptions.borderExtrapolationDisable) {
                values[iin] = yinput[vi2 + activeVarCount];
            } else {
                values[iin] = interp_coeff[0] * yinput[vi2 + activeVarCount] + interp_coeff[1] * yinput[vi1 + activeVarCount];
            }
        }

        for (int activeVarCount = 0; activeVarCount < regionDefinedVolVariableSizes[hiRegionID]; activeVarCount ++) {
            int varIndex = regionDefinedVolVariableIndexes[hiRegionID][activeVarCount];
            int iout = volSymbolOffset + varIndex * numSymbolsPerVolVar + 1 + pVolumeElement[me.vindexFeatureHi].getFeature()->getIndex();
            if (vi4 < 0 || sundialsSolverOptions.borderExtrapolationDisable) {
                values[iout] = yinput[vi3 + activeVarCount];
            } else {
                values[iout] = interp_coeff[0] * yinput[vi3 + activeVarCount] + interp_coeff[1] * yinput[vi4 + activeVarCount];
            }
        }
    }

    if (simulation->getNumMemVariables() > 0) {
        // fill in membrane variable values
        int membraneElementVectorOffset = getMembraneElementVectorOffset(me.index);
        memcpy(values + memSymbolOffset, yinput + membraneElementVectorOffset, simulation->getNumMemVariables() * sizeof(double));
    }

    //From Fei Add Volume region variables to expression evaluation values (github pull request opened on Apr 26, 2019 by fgao15)
    if (simulation->getNumVolRegionVariables() > 0) {
        // fill in volume region INSIDE and OUTSIDE values
        MembraneRegion *mr = me.region;

        VolumeRegion *vr1 = mr->getVolumeRegion1();
        VolumeRegion *vr2 = mr->getVolumeRegion2();
        for (int varIndex = 0; varIndex < simulation->getNumVolRegionVariables(); varIndex ++) {
            int offset = volRegionSymbolOffset + varIndex * numSymbolsPerVolVar + 1;
            {
                int volumeRegionElementVectorOffset = getVolumeRegionVectorOffset(vr1->getIndex());
                values[offset + vr1->getFeature()->getIndex()] = yinput[volumeRegionElementVectorOffset + varIndex];
            }
            {

                int volumeRegionElementVectorOffset = getVolumeRegionVectorOffset(vr2->getIndex());
                values[offset + vr2->getFeature()->getIndex()] = yinput[volumeRegionElementVectorOffset + varIndex];
            }
        }
    }

    if (simulation->getNumMemRegionVariables() > 0) {
        // fill in membrane region variable values
        int membraneRegionElementVectorOffset = getMembraneRegionVectorOffset(me.getRegionIndex());
        memcpy(values + memRegionSymbolOffset, yinput + membraneRegionElementVectorOffset, simulation->getNumMemRegionVariables() * sizeof(double));
    }
}

void SundialsPdeScheduler::updateRegionStatePointValues(int regionID, double t, double* yinput, bool bVolumeRegion, double* values) {

    values[0] = t;

    if (yinput == 0) {
        return;
    }

    simulation->populateRegionSizeVariableValues(values + regionSizeVariableSymbolOffset, bVolumeRegion, regionID);

    if (bVolumeRegion) {
        // fill in volume region variable values
        int volumeRegionElementVectorOffset = getVolumeRegionVectorOffset(regionID);
        memcpy(values + volRegionSymbolOffset, yinput + volumeRegionElementVectorOffset, simulation->getNumVolRegionVariables() * sizeof(double));
    } else {
        // fill in membrane region variable values
        int membraneRegionElementVectorOffset = getMembraneRegionVectorOffset(regionID);
        memcpy(values + memRegionSymbolOffset, yinput + membraneRegionElementVectorOffset, simulation->getNumMemRegionVariables() * sizeof(double));

        MembraneRegion *mr = mesh->getMembraneRegion(regionID);

        int numVolRegionVariables = simulation->getNumVolRegionVariables();
        VolumeRegion *vr1 = mr->getVolumeRegion1();
        VolumeRegion *vr2 = mr->getVolumeRegion2();
        for (int varIndex = 0; varIndex < numVolRegionVariables; varIndex ++) {
            int offset = volRegionSymbolOffset + varIndex * numSymbolsPerVolVar + 1;
            {
                int volumeRegionElementVectorOffset = getVolumeRegionVectorOffset(vr1->getIndex());
                values[offset + vr1->getFeature()->getIndex()] = yinput[volumeRegionElementVectorOffset + varIndex];
            }
            {

                int volumeRegionElementVectorOffset = getVolumeRegionVectorOffset(vr2->getIndex());
                values[offset + vr2->getFeature()->getIndex()] = yinput[volumeRegionElementVectorOffset + varIndex];
            }
        }
    }
}
