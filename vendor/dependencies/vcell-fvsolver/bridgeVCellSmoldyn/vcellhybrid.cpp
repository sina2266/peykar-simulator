/* Steven Andrews, started 10/22/2001.
 This is the entry point for the Smoldyn program.  See documentation
 called Smoldyn_doc1.pdf and Smoldyn_doc2.pdf.
 Copyright 2003-2011 by Steven Andrews.  This work is distributed under the terms
 of the Gnu Lesser General Public License (LGPL). */

#include "smoldyn.h"
#include "random2.h"
#include "smoldynfuncs.h"
#include "vcellhybrid.h"
#include <string.h>
#include <stdio.h>
#include <stdexcept>
#include <string>
using std::string;

#include <algorithm>
using namespace std;
#include <VCELL/SimTool.h>
#include <VCELL/SimulationExpression.h>
#include "VCellValueProvider.h"
#include "VCellMesh.h"
#include "VCellSmoldynOutput.h"

extern VCellSmoldynOutput* vcellSmoldynOutput;

/**
 * Disable the particle-file commands VCell writes for "save particle files" (incrementfile and listmols on a
 * *.smoldynOutput file) and report whether there were any. Their Smoldyn-step clock does not match the PDE's
 * output times when Smoldyn takes larger steps than the PDE; SimTool writes the files at the output times.
 */
static bool disableParticleFileCommands(queue_c q) {
	bool found = false;
	if (q == NULL || q->n <= 0) {
		return false;
	}
	for (int i = q->f; i != q->b; i = (i + 1) % q->n) {
		cmdptr cmd = (cmdptr) q->x[i];
		if (cmd == NULL || cmd->str == NULL) {
			continue;
		}
		char word[STRCHAR], file[STRCHAR];
		if (sscanf(cmd->str, "%s %s", word, file) != 2) {
			continue;
		}
		size_t len = strlen(file);
		const char* ext = ".smoldynOutput";
		bool smoldynOutput = len >= strlen(ext) && strcmp(file + len - strlen(ext), ext) == 0;
		if (smoldynOutput && (strcmp(word, "listmols") == 0 || strcmp(word, "incrementfile") == 0)) {
			cmd->str[0] = '\0';  // an empty command line is a no-op (docommand)
			found = true;
		}
	}
	return found;
}

simptr vcellhybrid::smoldynInit(SimTool* simTool, string& fileName) {
	LoggingCallback=NULL;
	ThrowThreshold=10;

	char root[STRCHAR],fname[STRCHAR],flags[STRCHAR],*cptr;

	for(int i=0;i<STRCHAR;i++) root[i]=fname[i]=flags[i]='\0';
	strcpy(root, fileName.c_str());
	cptr=strrpbrk(root,":\\/");
	if(cptr) cptr++;
	else cptr=root;
	strcpy(fname,cptr);
	*cptr='\0';

	simptr sim = NULL;
	int er;

	er=simInitAndLoad(root,fname,&sim,flags,new VCellValueProviderFactory(simTool), new VCellMesh(simTool));
	if (sim != NULL && sim->cmds != NULL) {
		bool inQueue = disableParticleFileCommands(((cmdssptr)sim->cmds)->cmd);
		bool inIntQueue = disableParticleFileCommands(((cmdssptr)sim->cmds)->cmdi);
		bSaveParticlePositions = inQueue || inIntQueue;
	}
	er=simUpdateAndDisplay(sim);
	er=scmdopenfiles((cmdssptr)sim->cmds,1);
	
	vcellSmoldynOutput = new VCellSmoldynOutput(sim);///check it out.
	vcellSmoldynOutput->setSimTool(simTool);

	sim->clockstt=time(NULL);
	er=simdocommands(sim);

	SimulationExpression* vcellSim = simTool->getSimulation();
	SymbolTable* symbolTable = vcellSim->getSymbolTable();
	char erstr[1024];
	//initialization for reaction rates (as expression)
	for(int j = 0; j < MAXORDER; j++)
	{
		rxnssptr rxnssInOrder = sim -> rxnss[j]; //loop through 0th, 1st, 2nd order rxn lists
		if (rxnssInOrder == 0) {
			continue;
		}
		for (int i = 0; i < rxnssInOrder->totrxn; i ++) {
			valueproviderptr valueProvider = rxnssInOrder->rxn[i]->rateValueProvider;
			if(valueProvider != NULL)
			{
				((VCellValueProvider*)valueProvider)->bindExpression(symbolTable);
			}
		}
	}

	//initialization for surface action(asorption, desorption, transmission) rates (as expression)
	if(sim->srfss != NULL)
	{
		surfacessptr surfacess = sim -> srfss;
		if(surfacess->nsrf > 0)
		{
			int numSrfs = surfacess->nsrf;
			surfactionptr actdetails;
			enum MolecState ms,ms2;
			enum PanelFace face;
			int nspecies=sim->mols?sim->mols->nspecies:0;
			HashtableIterator iter = surfacess->snametosrf->getIterator(surfacess->snametosrf);
			while (iter.iter(&iter)) {
				surfaceptr srf=(surfaceptr)iter.value;
				for(int i=0; i<nspecies; i++){
					for(ms=(MolecState)0; ms<MSMAX; ms=(MolecState)(ms+1)){
						for(face=(PanelFace)0; face<3; face=(PanelFace)(face+1)){
							if(srf->actdetails == NULL || srf->actdetails[i] == NULL || srf->actdetails[i][ms] == NULL || srf->actdetails[i][ms][face] == NULL) continue;
							actdetails=srf->actdetails[i][ms][face];
							for(ms2=(MolecState)0;ms2<MSMAX1;ms2=(MolecState)(ms2+1)) {
								if(actdetails == NULL || actdetails->srfRateValueProvider[ms2] == NULL) continue;
								valueproviderptr valueProvider = actdetails->srfRateValueProvider[ms2];
								((VCellValueProvider*)valueProvider)->bindExpression(symbolTable);
							}
						}
					}
				}
			}
		}
	}
	return sim;
}

void vcellhybrid::smoldynOneStep(simptr sim){
	simulatetimestep(sim);
	vcellSmoldynOutput->computeHistogram();
}

void vcellhybrid::smoldynEnd(simptr sim) {
	int er = 0;
	sim->elapsedtime+=difftime(time(NULL),sim->clockstt);
	endsimulate(sim,er);
	simfree(sim);
}

void vcellhybrid::writeParticlePositions(simptr sim, const std::string& fileName) {
	FILE* fp = fopen(fileName.c_str(), "w");
	if (fp == NULL) {
		throw std::runtime_error("cannot open particle file " + fileName + " for writing");
	}
	char state[STRCHAR];
	molssptr mols = sim->mols;
	if (mols != NULL) {
		for (int ll = 0; ll < mols->nlist; ll++) {
			for (int m = 0; m < mols->nl[ll]; m++) {
				moleculeptr mptr = mols->live[ll][m];
				if (mptr->ident <= 0) {
					continue;
				}
				double pos[3] = {0, 0, 0};
				for (int d = 0; d < sim->dim && d < 3; d++) {
					pos[d] = mptr->pos[d];
				}
				fprintf(fp, "%s(%s) %.9g %.9g %.9g\n", mols->spname[mptr->ident], molms2string(mptr->mstate, state),
						pos[0], pos[1], pos[2]);
			}
		}
	}
	if (fclose(fp) != 0) {
		throw std::runtime_error("failed to write particle file " + fileName);
	}
}

bool vcellhybrid::bHybrid = false;
int vcellhybrid::taskID = -1;
bool vcellhybrid::bSaveParticlePositions = false;
