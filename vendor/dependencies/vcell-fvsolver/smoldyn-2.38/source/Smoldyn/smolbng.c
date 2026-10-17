/* Steven Andrews, started 10/22/2001.
 This is a library of functions for the Smoldyn program.
 See documentation called Smoldyn_doc1.pdf and Smoldyn_doc2.pdf, and the Smoldyn
 website, which is at www.smoldyn.org.
 Copyright 2003-2013 by Steven Andrews.  This work is distributed under the terms
 of the Gnu Lesser General Public License (LGPL). */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#include <unistd.h>
#include "smoldyn.h"
#include "smoldynfuncs.h"
#include "string2.h"




/******************************************************************************/
/*********************************** BioNetGen ********************************/
/******************************************************************************/



/******************************************************************************/
/****************************** Local declarations ****************************/
/******************************************************************************/

// memory management

bngptr bngalloc(bngptr bng,int maxparams,int maxmonomer,int maxbspecies,int maxbrxns);
void bngfree(bngptr bng);
bngssptr bngssalloc(bngssptr bngss,int maxbng);
void bngssfree(bngssptr bngss);

// structure setup

void bngsetcondition(bngssptr bngss,enum StructCond cond,int upgrade);
int bngenablebng(simptr sim,int maxbng);
bngptr bngaddbng(simptr sim,const char *bngname);
int bngparseparameter(bngptr bng,int index);
int bngaddparameter(bngptr bng,const char *name,const char *string);
int bngaddmonomer(bngptr bng,const char *name,int nchar);

int bngmakeshortname(bngptr bng,int index,int totalmn);
double bngmakedifc(bngptr bng,int index,int totalmn);

int bngaddspecies(bngptr bng,int index,const char *name,double count);
int bngaddreaction(bngptr bng,int bindex,const char *reactants,const char *products,const char *rate);
bngptr bngnetreadstring(simptr sim,ParseFilePtr pfp,bngptr bng,const char *word,char *line2);
int bngupdateparams(simptr sim);
int bngupdatelists(simptr sim);
int bngupdate(simptr sim);





/******************************************************************************/
/******************************* memory management ****************************/
/******************************************************************************/


/* bngalloc */
bngptr bngalloc(bngptr bng,int maxparams,int maxmonomer,int maxbspecies,int maxbrxns) {
	int i,newmax,oldmax;
	char **strlist;
	double *dbllist,**dblptrlist;
  int *intlist,**intptrlist;
  rxnptr *rxnlist;
	enum MolecState *mslist;

	newmax=oldmax=0;
	strlist=NULL;
	dbllist=NULL;
  intlist=NULL;

	if(!bng) {
		bng=(bngptr)malloc(sizeof(struct bngstruct));
		CHECKMEM(bng);
    bng->bngss=NULL;
		bng->bngname=NULL;
    bng->bngindex=0;
		bng->unirate=1;
		bng->birate=1;
		bng->maxparams=0;
		bng->nparams=0;
		bng->paramnames=NULL;
    bng->paramstrings=NULL;
		bng->paramvalues=NULL;
    bng->maxmonomer=0;
    bng->nmonomer=0;
    bng->monomernames=NULL;
    bng->monomercount=NULL;
		bng->monomerdifc=NULL;
		bng->monomerdisplaysize=NULL;
		bng->monomercolor=NULL;
		bng->monomerstate=NULL;
		bng->maxbspecies=0;
		bng->nbspecies=0;
		bng->bsplongnames=NULL;
		bng->bspshortnames=NULL;
		bng->bspstate=NULL;
		bng->bspcountstr=NULL;
		bng->bspcount=NULL;
		bng->spindex=NULL;
		bng->maxbrxns=0;
		bng->nbrxns=0;
		bng->brxnreactstr=NULL;
		bng->brxnprodstr=NULL;
    bng->brxnratestr=NULL;
    bng->brxnreact=NULL;
    bng->brxnprod=NULL;
    bng->brxnorder=NULL;
    bng->brxnnprod=NULL;
    bng->brxn=NULL; }

	if(maxparams>bng->maxparams) {					// enlarge params
		newmax=maxparams;
		oldmax=bng->maxparams;
		CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->paramnames[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->paramnames);
		bng->paramnames=strlist;
    
		CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->paramstrings[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->paramstrings);
		bng->paramstrings=strlist;
    
		CHECKMEM(dbllist=(double*)calloc(newmax,sizeof(double)));
		for(i=0;i<oldmax;i++) dbllist[i]=bng->paramvalues[i];
		for(;i<newmax;i++) dbllist[i]=0;
		free(bng->paramvalues);
		bng->paramvalues=dbllist;

    bng->maxparams=newmax; }
  
  if(maxmonomer>bng->maxmonomer) {          // enlarge monomers
		newmax=maxmonomer;
		oldmax=bng->maxmonomer;
		CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->monomernames[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->monomernames);
		bng->monomernames=strlist;

    CHECKMEM(intlist=(int*)calloc(newmax,sizeof(int)));
		for(i=0;i<oldmax;i++) intlist[i]=bng->monomercount[i];
		for(;i<newmax;i++) intlist[i]=0;
		free(bng->monomercount);
		bng->monomercount=intlist;

		CHECKMEM(dbllist=(double*)calloc(newmax,sizeof(double)));
		for(i=0;i<oldmax;i++) dbllist[i]=bng->monomerdifc[i];
		for(;i<newmax;i++) dbllist[i]=0;
		free(bng->monomerdifc);
		bng->monomerdifc=dbllist;

		CHECKMEM(dbllist=(double*)calloc(newmax,sizeof(double)));
		for(i=0;i<oldmax;i++) dbllist[i]=bng->monomerdisplaysize[i];
		for(;i<newmax;i++) dbllist[i]=0;
		free(bng->monomerdisplaysize);
		bng->monomerdisplaysize=dbllist;

		CHECKMEM(dblptrlist=(double**)calloc(newmax,sizeof(double *)));
		for(i=0;i<oldmax;i++) dblptrlist[i]=bng->monomercolor[i];
		for(;i<newmax;i++) {
			CHECKMEM(dblptrlist[i]=(double*)calloc(3,sizeof(double)));
			dblptrlist[i][0]=0;
			dblptrlist[i][1]=0;
			dblptrlist[i][2]=0; }
		free(bng->monomercolor);
		bng->monomercolor=dblptrlist;

		CHECKMEM(mslist=(enum MolecState *)calloc(newmax,sizeof(enum MolecState)));
		for(i=0;i<oldmax;i++) mslist[i]=bng->monomerstate[i];
		for(;i<newmax;i++) mslist[i]=MSsoln;
		free(bng->monomerstate);
		bng->monomerstate=mslist;

		bng->maxmonomer=newmax; }

	if(maxbspecies>bng->maxbspecies) {				// enlarge bspecies
		newmax=maxbspecies;
		oldmax=bng->maxbspecies;
		CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->bsplongnames[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->bsplongnames);
		bng->bsplongnames=strlist;

    CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->bspshortnames[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->bspshortnames);
		bng->bspshortnames=strlist;

		CHECKMEM(mslist=(enum MolecState *)calloc(newmax,sizeof(enum MolecState)));
		for(i=0;i<oldmax;i++) mslist[i]=bng->bspstate[i];
		for(;i<newmax;i++) mslist[i]=MSsoln;
		free(bng->bspstate);
		bng->bspstate=mslist;

    CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->bspcountstr[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->bspcountstr);
		bng->bspcountstr=strlist;

		CHECKMEM(dbllist=(double*)calloc(newmax,sizeof(double)));
		for(i=0;i<oldmax;i++) dbllist[i]=bng->bspcount[i];
		for(;i<newmax;i++) dbllist[i]=0;
		free(bng->bspcount);
		bng->bspcount=dbllist;

    CHECKMEM(intlist=(int*)calloc(newmax,sizeof(int)));
		for(i=0;i<oldmax;i++) intlist[i]=bng->spindex[i];
		for(;i<newmax;i++) intlist[i]=0;
		free(bng->spindex);
		bng->spindex=intlist;

    bng->maxbspecies=newmax; }

	if(maxbrxns>bng->maxbrxns) {				// enlarge brxns
		newmax=maxbrxns;
		oldmax=bng->maxbrxns;
		CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->brxnreactstr[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->brxnreactstr);
		bng->brxnreactstr=strlist;
    
		CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->brxnprodstr[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->brxnprodstr);
		bng->brxnprodstr=strlist;
    
		CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bng->brxnratestr[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bng->brxnratestr);
		bng->brxnratestr=strlist;

    CHECKMEM(intptrlist=(int**)calloc(newmax,sizeof(int *)));
		for(i=0;i<oldmax;i++) intptrlist[i]=bng->brxnreact[i];
		for(;i<newmax;i++) {
      CHECKMEM(intptrlist[i]=(int*)calloc(2,sizeof(int)));
      intptrlist[i][0]=0;
      intptrlist[i][1]=0; }
		free(bng->brxnreact);
		bng->brxnreact=intptrlist;

    CHECKMEM(intptrlist=(int**)calloc(newmax,sizeof(int *)));
		for(i=0;i<oldmax;i++) intptrlist[i]=bng->brxnprod[i];
		for(;i<newmax;i++) {
      CHECKMEM(intptrlist[i]=(int*)calloc(2,sizeof(int)));
      intptrlist[i][0]=0;
      intptrlist[i][1]=0; }
		free(bng->brxnprod);
		bng->brxnprod=intptrlist;
    
    CHECKMEM(intlist=(int*)calloc(newmax,sizeof(int)));
		for(i=0;i<oldmax;i++) intlist[i]=bng->brxnorder[i];
		for(;i<newmax;i++) intlist[i]=0;
		free(bng->brxnorder);
		bng->brxnorder=intlist;
    
    CHECKMEM(intlist=(int*)calloc(newmax,sizeof(int)));
		for(i=0;i<oldmax;i++) intlist[i]=bng->brxnnprod[i];
		for(;i<newmax;i++) intlist[i]=0;
		free(bng->brxnnprod);
		bng->brxnnprod=intlist;
    
    CHECKMEM(rxnlist=(rxnptr*)calloc(newmax,sizeof(rxnptr)));
		for(i=0;i<oldmax;i++) rxnlist[i]=bng->brxn[i];
		for(;i<newmax;i++) rxnlist[i]=NULL;
		free(bng->brxn);
		bng->brxn=rxnlist;

    bng->maxbrxns=newmax; }
  
		return bng;
failure:
	bngfree(bng);
	simLog(NULL,10,"Unable to allocate memory in bngalloc");
	return NULL; }


/* bngfree */
void bngfree(bngptr bng) {
	int i;

	if(!bng) return;

	for(i=0;i<bng->maxbrxns;i++) {
		free(bng->brxnreactstr[i]);
		free(bng->brxnprodstr[i]);
    free(bng->brxnratestr[i]);
    free(bng->brxnreact[i]);
    free(bng->brxnprod[i]); }
	free(bng->brxnreactstr);
	free(bng->brxnprodstr);
  free(bng->brxnreact);
  free(bng->brxnprod);
  free(bng->brxnorder);
  free(bng->brxnnprod);
  free(bng->brxn);

	for(i=0;i<bng->maxbspecies;i++) {
		free(bng->bsplongnames[i]);
		free(bng->bspshortnames[i]);
		free(bng->bspcountstr[i]); }
	free(bng->bsplongnames);
	free(bng->bspshortnames);
	free(bng->bspstate);
	free(bng->bspcountstr);
	free(bng->bspcount);
	free(bng->spindex);

  for(i=0;i<bng->maxmonomer;i++) {
    free(bng->monomernames[i]); }
  free(bng->monomernames);
  free(bng->monomercount);
	free(bng->monomerdifc);
	free(bng->monomerdisplaysize);
	for(i=0;i<bng->maxmonomer;i++)
		free(bng->monomercolor[i]);
	free(bng->monomercolor);
	free(bng->monomerstate);

	for(i=0;i<bng->maxparams;i++) {
		free(bng->paramnames[i]);
		free(bng->paramstrings[i]); }
	free(bng->paramnames);
	free(bng->paramstrings);
	free(bng->paramvalues);

	free(bng);
	return; }


/* bngssalloc */
bngssptr bngssalloc(bngssptr bngss,int maxbng) {
	int i,newmax,oldmax;
	char **strlist;
	bngptr *newbnglist;

	if(!bngss) {
		CHECKMEM(bngss=(bngssptr) malloc(sizeof(struct bngsuperstruct)));
		bngss->condition=SCinit;
		bngss->sim=NULL;
		bngss->BNG2path=NULL;
		bngss->maxbng=0;
		bngss->nbng=0;
		bngss->bngnames=NULL;
		bngss->bnglist=NULL; }

	if(!bngss->BNG2path) {
		CHECKMEM(bngss->BNG2path=EmptyString());
		strcpy(bngss->BNG2path,"/usr/local/bin/BNG2.pl"); }

	if(maxbng>bngss->maxbng) {
		newmax=maxbng;
		oldmax=bngss->maxbng;
    CHECKMEM(strlist=(char**)calloc(newmax,sizeof(char *)));
		for(i=0;i<oldmax;i++) strlist[i]=bngss->bngnames[i];
		for(;i<newmax;i++) {CHECKMEM(strlist[i]=EmptyString());}
		free(bngss->bngnames);
		bngss->bngnames=strlist;

    CHECKMEM(newbnglist=(bngptr*)calloc(newmax,sizeof(bngptr)));
		for(i=0;i<oldmax;i++) newbnglist[i]=bngss->bnglist[i];
		for(;i<newmax;i++) {
			CHECKMEM(newbnglist[i]=bngalloc(NULL,1,1,1,1));
			newbnglist[i]->bngss=bngss;
			newbnglist[i]->bngname=bngss->bngnames[i];
      newbnglist[i]->bngindex=i; }
		free(bngss->bnglist);
		bngss->bnglist=newbnglist;

    bngss->maxbng=newmax; }
	return bngss;

failure:
	bngssfree(bngss);
	simLog(NULL,10,"Unable to allocate memory in bngssalloc");
	return NULL; }


/* bngssfree */
void bngssfree(bngssptr bngss) {
	int i;

	if(!bngss) return;
	for(i=0;i<bngss->maxbng;i++) {
		bngfree(bngss->bnglist[i]);
		free(bngss->bngnames[i]); }
	free(bngss->bnglist);
	free(bngss->bngnames);
	free(bngss->BNG2path);
	free(bngss);
	return; }


/******************************************************************************/
/***************************** data structure output **************************/
/******************************************************************************/


/* bngoutput */
void bngoutput(simptr sim) {
	bngssptr bngss;
	bngptr bng;
	int b,i;
	char string[STRCHAR];

	bngss=sim->bngss;
	if(!bngss) return;
	simLog(sim,2,"BioNetGen parameters\n");
	simLog(sim,2," BNG2.pl path: %s\n",bngss->BNG2path);
	simLog(sim,2," BNG allocated: %i, BNG defined: %i\n",bngss->maxbng,bngss->nbng);
	for(b=0;b<bngss->nbng;b++) {
		bng=bngss->bnglist[b];
		simLog(sim,2," BNG: %s\n",bng->bngname);
		if(bng->unirate!=1 || bng->birate!=1)
			simLog(sim,2,"  rate multipliers: unimolecular: %g, bimolecular: %g\n",bng->unirate,bng->birate);
		simLog(sim,2,"  parameters allocated: %i, defined: %i\n",bng->maxparams,bng->nparams);
		for(i=0;i<bng->nparams;i++) {
			simLog(sim,2,"   %i %s %g\n",i,bng->paramnames[i],bng->paramvalues[i]); }
		simLog(sim,2,"  monomers allocated: %i, defined: %i\n",bng->maxmonomer,bng->nmonomer);
		for(i=0;i<bng->nmonomer;i++) {
			simLog(sim,2,"   %s: default state: %s, diffusion coeff.: %g\n",bng->monomernames[i],molms2string(bng->monomerstate[i],string),bng->monomerdifc[i]);
			simLog(sim,2,"    display size: %g, color: %g %g %g\n",bng->monomerdisplaysize[i],bng->monomercolor[i][0],bng->monomercolor[i][1],bng->monomercolor[i][2]); }
		simLog(sim,2,"  species allocated: %i, defined: %i\n",bng->maxbspecies,bng->nbspecies);
		for(i=0;i<bng->nbspecies;i++) {
			if(bng->spindex[i]>0)
				simLog(sim,2,"   %i %s (%s), count: %g, longname: %s\n",i,bng->bspshortnames[i],molms2string(bng->bspstate[i],string),bng->bspcount[i],bng->bsplongnames[i]); }
		simLog(sim,2,"  reactions allocated: %i, defined: %i\n",bng->maxbrxns,bng->nbrxns);
		for(i=0;i<bng->nbrxns;i++)
			if(bng->brxn[i]) {
				simLog(sim,2,"   %i",i);
				if(bng->brxnorder[i]>=1) simLog(sim,2," %s",bng->bspshortnames[bng->brxnreact[i][0]]);
				if(bng->brxnorder[i]==2) simLog(sim,2," + %s",bng->bspshortnames[bng->brxnreact[i][1]]);
				simLog(sim,2," ->");
				if(bng->brxnnprod[i]>=1) simLog(sim,2," %s",bng->bspshortnames[bng->brxnprod[i][0]]);
				if(bng->brxnnprod[i]==2) simLog(sim,2," + %s",bng->bspshortnames[bng->brxnprod[i][1]]);
				simLog(sim,2,"  rate: %g",bng->brxn[i]->rate);
				simLog(sim,2,"\n"); }}
	simLog(sim,2,"\n");
	return; }


/* checkbngparams */
int checkbngparams(simptr sim,int *warnptr) {
	int error,warn,b,i;
	bngssptr bngss;
	bngptr bng;
	char string[STRCHAR];

	error=warn=0;
	bngss=sim->bngss;
	if(!bngss) {
		if(warnptr) *warnptr=warn;
		return 0; }

	if(bngss->condition!=SCok) {
		warn++;
		simLog(sim,7," WARNING: bng structure condition is %s\n",simsc2string(bngss->condition,string)); }

	for(b=0;b<bngss->nbng;b++) {
		bng=bngss->bnglist[b];
		for(i=0;i<bng->nbspecies;i++) {
			if(bng->bspcount[i]>0 && bng->bspcount[i]<1) simLog(sim,7," WARNING: count for %s is very low\n",bng->bspshortnames[i]); }}

	if(warnptr) *warnptr=warn;
	return error; }


/******************************************************************************/
/******************************** structure set up ****************************/
/******************************************************************************/


/* bngsetcondition */
void bngsetcondition(bngssptr bngss,enum StructCond cond,int upgrade) {
	if(!bngss) return;
	if(upgrade==0 && bngss->condition>cond) bngss->condition=cond;
	else if(upgrade==1 && bngss->condition<cond) bngss->condition=cond;
	else if(upgrade==2) bngss->condition=cond;
	if(bngss->sim && bngss->condition<bngss->sim->condition) {
		cond=bngss->condition;
		simsetcondition(bngss->sim,cond==SCinit?SClists:cond,0); }
	return; }


/* bngenablebng */
int bngenablebng(simptr sim,int maxbng) {
	bngssptr bngss;

	if(sim->bngss)									// check for redundant function call
		if(maxbng==-1 || sim->bngss->maxbng==maxbng)
			return 0;
	bngss=bngssalloc(sim->bngss,maxbng<0?1:maxbng);
	if(!bngss) return 1;
	sim->bngss=bngss;
	bngss->sim=sim;
	bngsetcondition(sim->bngss,SClists,0);
	return 0; }


/* bngaddbng */
bngptr bngaddbng(simptr sim,const char *bngname) {
	int er,i;
	bngssptr bngss;
	bngptr bng;

	if(!sim->bngss) {
	 er=bngenablebng(sim,-1);
	 if(er) return NULL; }
	bngss=sim->bngss;

	i=stringfind(bngss->bngnames,bngss->nbng,bngname);
	if(i<0) {
		if(bngss->nbng==bngss->maxbng) {
			er=bngenablebng(sim,bngss->nbng*2+1);
			if(er) return NULL; }
		i=bngss->nbng++;
		strncpy(bngss->bngnames[i],bngname,STRCHAR-1);
		bngss->bngnames[i][STRCHAR-1]='\0';
		bng=bngss->bnglist[i]; }
	else
		bng=bngss->bnglist[i];

	bngsetcondition(bngss,SClists,0);
	return bng; }


/* bngparseparameter */
int bngparseparameter(bngptr bng,int index) {
  int er;
  double value;

  er=0;
  if(bng->paramstrings[index]) {
    value=strmatheval(bng->paramstrings[index],bng->paramnames,bng->paramvalues,bng->nparams);
    er=strmatherror(NULL);
    bng->paramvalues[index]=value; }
  return (er==0)?0:1; }


/* bngaddparameter */
int bngaddparameter(bngptr bng,const char *name,const char *string) {
	int i,er;

	if(bng->nparams==bng->maxparams) {		// enlarge list if needed
		bng=bngalloc(bng,bng->maxparams*2+1,0,0,0);
		if(!bng) return -1; }

  i=stringfind(bng->paramnames,bng->nparams,name);
  if(i<0) {
    i=bng->nparams;
    bng->nparams++;
    strcpy(bng->paramnames[i],name); }
  if(string) strcpy(bng->paramstrings[i],string);
  else bng->paramstrings[i][0]='\0';
  er=bngparseparameter(bng,i);
  return (er==0)?i:-2; }


/* bngaddmonomer */
int bngaddmonomer(bngptr bng,const char *name,int nchar) {
  int i,j;
	simptr sim;

	sim=bng->bngss->sim;

	if(bng->nmonomer==bng->maxmonomer) {		// enlarge list if needed
		bng=bngalloc(bng,0,bng->maxmonomer*2+1,0,0);
		if(!bng) return -1; }

  i=stringnfind(bng->monomernames,bng->nmonomer,name,nchar);
  if(i<0) {
    i=bng->nmonomer;
    bng->nmonomer++;
    strncpy(bng->monomernames[i],name,nchar);

		j=stringnfind(sim->mols->spname,sim->mols->nspecies,name,nchar);
		if(j>0) {
			bng->monomerdifc[i]=sim->mols->difc[j][MSsoln];
			bng->monomerdisplaysize[i]=sim->mols->display[j][MSsoln];
			bng->monomercolor[i][0]=sim->mols->color[j][MSsoln][0];
			bng->monomercolor[i][1]=sim->mols->color[j][MSsoln][1];
			bng->monomercolor[i][2]=sim->mols->color[j][MSsoln][2]; }
		else {
			bng->monomerdifc[i]=0;
			bng->monomerdisplaysize[i]=0;
			bng->monomercolor[i][0]=0;
			bng->monomercolor[i][1]=0;
			bng->monomercolor[i][2]=0; }}

  return i; }


/* bngsetmonomerstate */
int bngsetmonomerstate(bngptr bng,char *name,enum MolecState ms) {
	int i,j;
	simptr sim;

	sim=bng->bngss->sim;

	i=bngaddmonomer(bng,name,strlen(name));
	if(i<0) return -1;
	bng->monomerstate[i]=ms;

	j=stringfind(sim->mols->spname,sim->mols->nspecies,name);
	if(j>0) {
		bng->monomerdifc[i]=sim->mols->difc[j][MSsoln];
		bng->monomerdisplaysize[i]=sim->mols->display[j][MSsoln];
		bng->monomercolor[i][0]=sim->mols->color[j][MSsoln][0];
		bng->monomercolor[i][1]=sim->mols->color[j][MSsoln][1];
		bng->monomercolor[i][2]=sim->mols->color[j][MSsoln][2]; }
	else {
		bng->monomerdifc[i]=0;
		bng->monomerdisplaysize[i]=0;
		bng->monomercolor[i][0]=0;
		bng->monomercolor[i][1]=0;
		bng->monomercolor[i][2]=0; }

	return 0; }


/* bngmakeshortname */
int bngmakeshortname(bngptr bng,int index,int totalmn) {
	char *shortname,string[STRCHAR];
	int length,mn,i1,i2;

	shortname=bng->bspshortnames[index];       // generate the short name root, diffusion coefficient, and default state
	shortname[0]='\0';
	length=STRCHAR-5;     // length remaining

	if(totalmn==1) {      // just 1 monomer
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				strcpy(shortname,bng->monomernames[mn]);
				mn=bng->nmonomer; }}

	else {                // multiple monomers
		for(mn=0;mn<bng->nmonomer && length>0;mn++)
			if(bng->monomercount[mn]>0) {
				sprintf(string,"%s.%i.",bng->monomernames[mn],bng->monomercount[mn]);
				strncat(shortname,string,length);
				length-=strlen(string); }
		length=strlen(shortname);                  // append the short name isomer number
		i2=0;
		for(i1=0;i1<index;i1++)
			if(!(strncmp(shortname,bng->bspshortnames[i1],length))) i2++;
		sprintf(string,"%i",i2);
		strcat(shortname,string); }

	return 0; }


/* bngmakedefaultstate */
enum MolecState bngmakedefaultstate(bngptr bng,int index,int totalmn) {
	enum MolecState ms;
	int mn;

	ms=MSsoln;

	if(totalmn==1) {      // just 1 monomer
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				ms=bng->monomerstate[mn];
				mn=bng->nmonomer; }}

	else {                // multiple monomers
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				if(bng->monomerstate[mn]>ms) ms=bng->monomerstate[mn]; }
		bng->bspstate[index]=ms; }

	return ms; }


/* bngmakedifc */
double bngmakedifc(bngptr bng,int index,int totalmn) {
	double difc;
	int mn;

	difc=-1;
	if(totalmn==1) {      // just 1 monomer
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				difc=bng->monomerdifc[mn];
				mn=bng->nmonomer; }}
	else {                // multiple monomers
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				if(difc==-1 && bng->monomerdifc[mn]==0) difc=0;
				else if(difc==-1) difc=bng->monomercount[mn]*pow(bng->monomerdifc[mn],-3.0);
				else if(bng->monomerdifc[mn]==0) difc=0;
				else difc+=bng->monomercount[mn]*pow(bng->monomerdifc[mn],-3.0); }
		if(difc!=0) difc=pow(difc,-1.0/3.0); }

	return difc; }


/* bngmakedisplaysize */
double bngmakedisplaysize(bngptr bng,int index,int totalmn) {
	double displaysize;
	int mn;

	displaysize=0;
	if(totalmn==1) {      // just 1 monomer
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				displaysize=bng->monomerdisplaysize[mn];
				mn=bng->nmonomer; }}
	else {                // multiple monomers
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				displaysize+=bng->monomercount[mn]*pow(bng->monomerdisplaysize[mn],3.0); }
		if(displaysize>0) displaysize=pow(displaysize,1.0/3.0); }

	return displaysize; }


/* bngmakecolor */
int bngmakecolor(bngptr bng,int index,int totalmn,double *color) {
	int mn;
	double weight,totalweight;

	color[0]=color[1]=color[2]=0;
	weight=0;
	totalweight=0;
	if(totalmn==1) {      // just 1 monomer
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				color[0]=bng->monomercolor[mn][0];
				color[1]=bng->monomercolor[mn][1];
				color[2]=bng->monomercolor[mn][2];
				mn=bng->nmonomer; }}
	else {                // multiple monomers
		for(mn=0;mn<bng->nmonomer;mn++)
			if(bng->monomercount[mn]>0) {
				weight=bng->monomercount[mn]*bng->monomerdisplaysize[mn];
				totalweight+=weight;
				color[0]+=weight*bng->monomercolor[mn][0];
				color[1]+=weight*bng->monomercolor[mn][1];
				color[2]+=weight*bng->monomercolor[mn][2]; }
		color[0]/=totalweight;
		color[1]/=totalweight;
		color[2]/=totalweight; }
	return 0; }


/* bngparsespecies */
int bngparsespecies(bngptr bng,int index) {
  int i1,i2,length,mn,totalmn,er;
  char *longname,ch;
  simptr sim;
  double value,poslo[DIMMAX],poshi[DIMMAX],difc,displaysize,color[3];
	enum MolecState ms;

  sim=bng->bngss->sim;

  for(mn=0;mn<bng->nmonomer;mn++)           // clear monomer counts
    bng->monomercount[mn]=0;

  longname=bng->bsplongnames[index];        // count numbers of each monomer in the longname
  length=strlen(longname);
  totalmn=0;
  i1=i2=0;
  while(longname[i2]!='\0') {			// i1 and i2 walk along string, i2 is ahead, they bracket monomer names
    while((ch=longname[i2])!='(' && ch!='.' && ch!='\0') i2++;
    mn=stringnfind(bng->monomernames,bng->nmonomer,longname+i1,i2-i1);
    if(mn<0) {
      mn=bngaddmonomer(bng,longname+i1,i2-i1);
      if(mn<0) return -1; }
    bng->monomercount[mn]++;
    totalmn++;
    if(ch=='.') i1=i2=i2+1;
    else if(ch=='(') {
      i1=i2=strparenmatch(longname,i2)+1;
      if(i1<=0) return -2;		// cannot parse name
			while(longname[i1]=='.') i1=i2=i1+1; }}

	bngmakeshortname(bng,index,totalmn);
	ms=bngmakedefaultstate(bng,index,totalmn);
	difc=bngmakedifc(bng,index,totalmn);
	displaysize=bngmakedisplaysize(bng,index,totalmn);
	bngmakecolor(bng,index,totalmn,color);

  i1=moladdspecies(sim,bng->bspshortnames[index]);            // add to Smoldyn simulation
  if(i1==-1) return -1;         // out of memory
  else if(i1==-4) return -3;    // illegal name
  else if(i1==-5)               // already exists
    i1=stringfind(sim->mols->spname,sim->mols->nspecies,bng->bspshortnames[index]);
  else if(i1==-6) return -3;    // illegal name
    
  bng->spindex[index]=i1;												// set spindex element

	molsetdifc(sim,i1,NULL,ms,difc);							// set diffusion coefficient
	molsetdisplaysize(sim,i1,NULL,MSall,displaysize);
	molsetcolor(sim,i1,NULL,MSall,color);

  if(bng->bspcountstr[index]) {                 // parse count information
    value=strmatheval(bng->bspcountstr[index],bng->paramnames,bng->paramvalues,bng->nparams);
    if(strmatherror(NULL)) return -4;   // cannot parse count string
    bng->bspcount[index]=value;
    if(value>0.5) {                             // add molecule to Smoldyn simulation
      systemcorners(sim,poslo,poshi);
			if(ms==MSsoln)
				er=addmol(sim,(int)(value+0.5),bng->spindex[index],poslo,poshi,0);
			else {
				er=addsurfmol(sim,(int)(value+0.5),bng->spindex[index],bng->bspstate[index],NULL,NULL,-1,PSall,NULL); }
      if(er==1) return -1;
			else if(er==2) return -6;			// no surface panels
      else if(er==3) return -5; }}  // too many molecules
    
  return 0; }


/* bngaddspecies */
int bngaddspecies(bngptr bng,int bindex,const char *longname,const char *countstr) {
  int er;

	if(bindex>=bng->maxbspecies) {		// enlarge list if needed
		bng=bngalloc(bng,0,0,2*bindex+1,0);
		if(!bng) return -1; }

	if(longname) strncpy(bng->bsplongnames[bindex],longname,STRCHAR-1);
  else bng->bsplongnames[bindex][0]='\0';
	if(countstr) strncpy(bng->bspcountstr[bindex],countstr,STRCHAR-1);
  else bng->bspcountstr[bindex][0]='\0';
	if(bng->nbspecies<=bindex)
		bng->nbspecies=bindex+1;
  er=bngparsespecies(bng,bindex);
  return er; }


/* bngparsereaction */
int bngparsereaction(bngptr bng,int index) {
  int i1,i2,order,nprod,react[2],prod[2],er;
  char string[STRCHAR];
  enum MolecState rctstate[2],prdstate[2];
	double rate;
  simptr sim;
  rxnptr rxn;

  sim=bng->bngss->sim;
  order=sscanf(bng->brxnreactstr[index],"%i,%i",&i1,&i2);   // reactant list
  bng->brxnorder[index]=order;
  if(order>=1) {
    bng->brxnreact[index][0]=i1;
    react[0]=bng->spindex[i1];
		rctstate[0]=bng->bspstate[i1]; }
  else {
    bng->brxnreact[index][0]=0;
    react[0]=0;
		rctstate[0]=MSsoln; }
  if(order==2) {
    bng->brxnreact[index][1]=i2;
    react[1]=bng->spindex[i2];
		rctstate[1]=bng->bspstate[i2]; }
  else {
    bng->brxnreact[index][1]=0;
    react[1]=0;
		rctstate[1]=MSsoln; }

  nprod=sscanf(bng->brxnprodstr[index],"%i,%i",&i1,&i2);    // product list
  bng->brxnnprod[index]=nprod;
  if(nprod>=1) {
    bng->brxnprod[index][0]=i1;
    prod[0]=bng->spindex[i1];
		prdstate[0]=bng->bspstate[i1]; }
  else {
    bng->brxnprod[index][0]=0;
    prod[0]=0;
		prdstate[0]=MSsoln; }
  if(nprod==2) {
    bng->brxnprod[index][1]=i2;
    prod[1]=bng->spindex[i2];
		prdstate[1]=bng->bspstate[i2]; }
  else {
    bng->brxnprod[index][1]=0;
    prod[1]=0;
		prdstate[1]=MSsoln; }
	if(rctstate[0]==MSsoln && rctstate[1]==MSsoln)
		prdstate[0]=prdstate[1]=MSsoln;

  sprintf(string,"bng%i_%i",bng->bngindex,index);     // reaction name

  rxn=RxnAddReaction(sim,string,order,react,rctstate,nprod,prod,prdstate,NULL,NULL);
  if(!rxn) return 1;
  bng->brxn[index]=rxn;

	rate=strmatheval(bng->brxnratestr[index],bng->paramnames,bng->paramvalues,bng->nparams);
	if(strmatherror(NULL)) return 2;   // cannot parse rate string
	rate*=(order==1)?bng->unirate:bng->birate;
	er=RxnSetValue(sim,"rate",rxn,rate);
	if(er==4) return 2;
  return 0; }


/* bngaddreaction */
int bngaddreaction(bngptr bng,int bindex,const char *reactants,const char *products,const char *rate) {
  int er;

	if(bindex>=bng->maxbrxns) {		// enlarge list if needed
		bng=bngalloc(bng,0,0,0,2*bindex+1);
		if(!bng) return 1; }

	if(reactants) strcpy(bng->brxnreactstr[bindex],reactants);
  else bng->brxnreactstr[bindex][0]='\0';
	if(products) strcpy(bng->brxnprodstr[bindex],products);
  else bng->brxnprodstr[bindex][0]='\0';
	if(rate) strcpy(bng->brxnratestr[bindex],rate);
  else bng->brxnratestr[bindex][0]='\0';
	if(bng->nbrxns<=bindex)
		bng->nbrxns=bindex+1;
  er=bngparsereaction(bng,bindex);
  return er; }


/* bngsetparameter */
int bngsetparam(bngptr bng,char *parameter,double amount) {
	if(!strcmp(parameter,"unimolecular_rate")) {
		if(amount<0) return 2;
		bng->unirate=amount; }
	else if(!strcmp(parameter,"bimolecular_rate")) {
		if(amount<0) return 2;
		bng->birate=amount; }
	else
		return 1;
	return 0; }


/* bngsetBNG2path */
int bngsetBNG2path(bngptr bng,char *path) {
	strcpy(bng->bngss->BNG2path,path);
	return 0; }


/* bngrunBNGL2 */
int bngrunBNGL2(bngptr bng,char *filename,char *outname) {
	char string[STRCHAR],*dot,sepchar;
	FILE *fptr;

#ifdef WINDOWS_BUILD
	sepchar='\\';
#else
	sepchar='/';
#endif
	sprintf(string,"%s%c%s",bng->bngss->BNG2path,sepchar,filename);
	fptr=fopen(string,"r");
	if(!fptr) return 1;
	fclose(fptr);
//	if(access(bng->bngss->BNG2path,X_OK)!=0) return 1;	//??
//	if(access(filename,R_OK)!=0) return 2;	//??
	sprintf(string,"perl %s %s",bng->bngss->BNG2path,filename);
	system(string);
	strcpy(outname,filename);
	dot=strrchr(outname,'.');
	if(!dot) dot=outname+strlen(outname);
	strcpy(dot,".net");
	return 0; }


/* bngnetreadstring */
bngptr bngnetreadstring(simptr sim,ParseFilePtr pfp,bngptr bng,const char *word,char *line2) {
	char str1[STRCHAR],str2[STRCHAR],str3[STRCHAR],*str4;
	bngssptr bngss;
	int itct,index,er,pfpcode;
	enum MolecState ms;
	static int inparams=0,inspecies=0,inrxns=0;
	double f1;

	bngss=sim->bngss;

	if(inparams) {																// parameters
		if(!strcmp(word,"end")) inparams=0;
		else {
			itct=sscanf(line2,"%s %s",str1,str2);
			CHECKS(itct==2,"failed to read parameter line");
			CHECKS(bngaddparameter(bng,str1,str2)>=0,"failed to add parameter"); }}

	else if(inspecies) {													// species
		if(!strcmp(word,"end")) inspecies=0;
		else {
			itct=sscanf(word,"%i",&index);
			CHECKS(itct==1,"failed to read species index value");
			itct=sscanf(line2,"%s %s",str1,str2);
			CHECKS(itct>=1,"failed to read species on species line");
			if(itct==1) strcpy(str2,"0");
			CHECKS(bngaddspecies(bng,index,str1,str2)==0,"failed to add species"); }}

	else if(inrxns) {															// reactions
		if(!strcmp(word,"end")) inrxns=0;
		else {
			itct=sscanf(word,"%i",&index);
			CHECKS(itct==1,"failed to read reaction index value");
			itct=sscanf(line2,"%s %s %s",str1,str2,str3);
			CHECKS(itct==3,"failed to read all 3 values of reaction line");
			CHECKS(bngaddreaction(bng,index,str1,str2,str3)==0,"failed to add reaction"); }}

	else if(!strcmp(word,"name")) {								// name
		itct=sscanf(line2,"%s",str1);
		CHECKS(itct==1,"error reading bngnet name");
		bng=bngaddbng(sim,str1);
		CHECKS(bng,"failed to add bng structure");
		CHECKS(!strnword(line2,2),"unexpected text following name"); }

	else if(!strcmp(word,"BNG2_path")) {					// BNG2_path
		itct=sscanf(line2,"%s",str1);
		if(!itct) str1[0]='\0';
		er=bngsetBNG2path(bng,str1);
		CHECKS(er!=1,"BNG2.pl not found or not executable at path '%s'",str1);
		CHECKS(!strnword(line2,2),"unexpected text following BNG2_path"); }

	else if(!strcmp(word,"bng_file")) {						// bng_file
		itct=sscanf(line2,"%s",str1);
		CHECKS(itct==1,"error reading file name");
		er=bngrunBNGL2(bng,str1,str2);
		CHECKS(er!=1,"BNG2.pl not found or not executable at path '%s'",str1);
		CHECKS(er!=2,"BioNetGen file not found");
		strcpy(str3,"read_file");
		str4=str2;
		pfpcode=Parse_ReadLine(&pfp,str3,&str4,NULL);
		CHECKS(pfpcode==0,"BUG in bngnetreadstring");
		CHECKS(!strnword(line2,2),"unexpected text following file name"); }

	else if(!strcmp(word,"default_state")) {			// default_state
		itct=sscanf(line2,"%s %s",str1,str2);
		CHECKS(itct==2,"default_state format: monomer state");
		ms=molstring2ms(str2);
		CHECKS(ms<MSMAX,"molecule state name not allowed or not recognized");
		er=bngsetmonomerstate(bng,str1,ms);
		CHECKS(!er,"failed to set monomer default state");
		CHECKS(!strnword(line2,3),"unexpected text following default_state"); }

	else if(!strcmp(word,"multiply")) {						// multiply
		itct=sscanf(line2,"%s %s",str1,str2);
		CHECKS(itct==2,"multiply format: parameter amount");
		f1=strmatheval(str2,NULL,NULL,0);
		CHECKS(!strmatherror(str3),"cannot compute amount: %s",str3);
		er=bngsetparam(bng,str1,f1);
		CHECKS(er!=1,"unrecognized parameter");
		CHECKS(er!=2,"multiply amount needs to be greater than 0");
		CHECKS(!strnword(line2,3),"unrecognized text following multiply statement"); }

	else if(!strncmp(word,"setOption",9));				// setOption...

	else if(!strcmp(word,"begin")) {							// begin
    CHECKS(bng,"need to enter a name for this set of BNG complexes before entering its data");
		itct=sscanf(line2,"%s",str1);
		CHECKS(itct==1,"error reading because 'begin' needs to followed by 'parameters', 'species', or 'reactions'");
		if(!strcmp(str1,"parameters")) inparams=1;
		else if(!strcmp(str1,"species")) inspecies=1;
		else if(!strcmp(str1,"reactions")) inrxns=1;
		CHECKS(!strnword(line2,2),"unexpected text at end of begin statement"); }

	else {																				// unknown word
		CHECKS(0,"syntax error within bngnet block: statement not recognized"); }

	return bng;

failure:
	simParseError(sim,pfp);
	return NULL; }


/* loadbng */
int loadbng(simptr sim,ParseFilePtr *pfpptr,char* line2) {
	ParseFilePtr pfp;
	char word[STRCHAR],errstring[STRCHAR];
	int done,pfpcode,firstline2;
	bngptr bng;

	pfp=*pfpptr;
	done=0;
	bng=NULL;
	firstline2=line2?1:0;

	while(!done) {
		if(pfp->lctr==0)
			simLog(sim,2," Reading file: '%s'\n",pfp->fname);
		if(firstline2) {
			strcpy(word,"name");
			pfpcode=1;
			firstline2=0; }
		else
			pfpcode=Parse_ReadLine(&pfp,word,&line2,errstring);
		printf("word=%s, line2=%s\n",word,line2); //??
		*pfpptr=pfp;
		CHECKS(pfpcode!=3,"%s",errstring);

		if(pfpcode==0);																// already taken care of
		else if(pfpcode==2) {													// end reading
			done=1; }
		else if(!strcmp(word,"end_bngnet")) {					// end_bngnet
			CHECKS(!line2,"unexpected text following end_bngnet");
			return 0; }
		else {
			bng=bngnetreadstring(sim,pfp,bng,word,line2);
			CHECK(bng); }}															// failed but error has already been reported

	return 0;																				// end of file ** NOTE: end of file implies end_bngnet

failure:																					// failure
	if(ErrorType!=1) simParseError(sim,pfp);
	*pfpptr=pfp=NULL;
	return 1; }


/* bngupdateparams */
int bngupdateparams(simptr sim) {
  return 0; }


/* bngupdatelists */
int bngupdatelists(simptr sim) {
  return 0; }


/* bngupdate */
int bngupdate(simptr sim) {
	int er;
	bngssptr bngss;
	
	bngss=sim->bngss;
	if(bngss) {
		if(bngss->condition<=SClists) {
			er=bngupdatelists(sim);
			if(er) return er;
			bngsetcondition(bngss,SCparams,1); }
		if(bngss->condition==SCparams) {
			er=bngupdateparams(sim);
			if(er) return er;
			bngsetcondition(bngss,SCok,1); }}
  return 0; }



/******************************************************************************/
/*************************** core simulation functions ************************/
/******************************************************************************/



