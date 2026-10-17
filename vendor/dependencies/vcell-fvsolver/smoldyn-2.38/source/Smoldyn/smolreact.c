/* Steven Andrews, started 10/22/2001.
 This is a library of functions for the Smoldyn program.  See documentation
 called Smoldyn_doc1.pdf and Smoldyn_doc2.pdf.
 Copyright 2003-2011 by Steven Andrews.  This work is distributed under the terms
 of the Gnu Lesser General Public License (LGPL). */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include "math2.h"
#include "random2.h"
#include "Rn.h"
#include "rxnparam.h"
#include "smoldyn.h"
#include "smoldynfuncs.h"
#include "SimCommand.h"
#include "Sphere.h"
#include "Zn.h"
#include "Geometry.h"

#include "smoldynconfigure.h"

/******************************************************************************/
/********************************** Reactions *********************************/
/******************************************************************************/


/******************************************************************************/
/****************************** Local declarations ****************************/
/******************************************************************************/

// enumerated types
char *rxnrp2string(enum RevParam rp,char *string);

// low level utilities
int rxnpackident(int order,int maxspecies,int *ident);
void rxnunpackident(int order,int maxspecies,int ipack,int *ident);
enum MolecState rxnpackstate(int order,enum MolecState *mstate);
void rxnunpackstate(int order,enum MolecState mspack,enum MolecState *mstate);
int rxnreactantstate(rxnptr rxn,enum MolecState *mstate,int convertb2f);
int rxnallstates(rxnptr rxn);
int findreverserxn(simptr sim,int order,int r,int *optr,int *rptr);

// memory management
rxnptr rxnalloc(int order);
void rxnfree(rxnptr rxn);
rxnssptr rxnssalloc(rxnssptr rxnss,int order,int maxspecies);

// data structure output

// parameter calculations
int rxnsetrates(simptr sim,int order);
int rxnsetproduct(simptr sim,int order,int r);
int rxnsetproducts(simptr sim,int order);
double rxncalcrate(simptr sim,int order,int r,double *pgemptr);
void rxncalctau(simptr sim,int order);

// structure set up
rxnssptr rxnreadstring(simptr sim,ParseFilePtr pfp,rxnssptr rxnss,char *word,char *line2);
int rxnsupdateparams(simptr sim);
int rxnsupdatelists(simptr sim,int order);

// core simulation functions
int morebireact(simptr sim,rxnptr rxn,moleculeptr mptr1,moleculeptr mptr2,int ll1,int m1,int ll2,enum EventType et,double *vect);


/******************************************************************************/
/********************************* enumerated types ***************************/
/******************************************************************************/


/* rxnstring2rp */
enum RevParam rxnstring2rp(char *string) {
	enum RevParam ans;

	if(!strcmp(string,"i")) ans=RPirrev;
	else if(!strcmp(string,"a")) ans=RPconfspread;
	else if(!strcmp(string,"p")) ans=RPpgem;
	else if(!strcmp(string,"x")) ans=RPpgemmax;
	else if(!strcmp(string,"r")) ans=RPratio;
	else if(!strcmp(string,"b")) ans=RPunbindrad;
	else if(!strcmp(string,"q")) ans=RPpgem2;
	else if(!strcmp(string,"y")) ans=RPpgemmax2;
	else if(!strcmp(string,"s")) ans=RPratio2;
	else if(!strcmp(string,"o")) ans=RPoffset;
	else if(!strcmp(string,"f")) ans=RPfixed;
	else if(!strcmp(string,"irrev")) ans=RPirrev;
	else if(!strcmp(string,"confspread")) ans=RPconfspread;
	else if(!strcmp(string,"bounce")) ans=RPbounce;
	else if(!strcmp(string,"pgem")) ans=RPpgem;
	else if(!strcmp(string,"pgemmax")) ans=RPpgemmax;
	else if(!strcmp(string,"ratio")) ans=RPratio;
	else if(!strcmp(string,"unbindrad")) ans=RPunbindrad;
	else if(!strcmp(string,"pgem2")) ans=RPpgem2;
	else if(!strcmp(string,"pgemmax2")) ans=RPpgemmax2;
	else if(!strcmp(string,"ratio2")) ans=RPratio2;
	else if(!strcmp(string,"offset")) ans=RPoffset;
	else if(!strcmp(string,"fixed")) ans=RPfixed;
	else ans=RPnone;
	return ans; }


/* rxnrp2string */
char *rxnrp2string(enum RevParam rp,char *string) {
	if(rp==RPirrev) strcpy(string,"irrev");
	else if(rp==RPconfspread) strcpy(string,"confspread");
	else if(rp==RPbounce) strcpy(string,"bounce");
	else if(rp==RPpgem) strcpy(string,"pgem");
	else if(rp==RPpgemmax) strcpy(string,"pgemmax");
	else if(rp==RPpgemmaxw) strcpy(string,"pgemmaxw");
	else if(rp==RPratio) strcpy(string,"ratio");
	else if(rp==RPunbindrad) strcpy(string,"unbindrad");
	else if(rp==RPpgem2) strcpy(string,"pgem2");
	else if(rp==RPpgemmax2) strcpy(string,"pgemmax2");
	else if(rp==RPratio2) strcpy(string,"ratio2");
	else if(rp==RPoffset) strcpy(string,"offset");
	else if(rp==RPfixed) strcpy(string,"fixed");
	else strcpy(string,"none");
	return string; }


/******************************************************************************/
/***************************** low level utilities ****************************/
/******************************************************************************/


/* readrxnname. */
int readrxnname(simptr sim,char *rname,int *orderptr,rxnptr *rxnpt) {
	int r,order;

	r=-1;
	for(order=0;order<MAXORDER && r==-1;order++)
		if(sim->rxnss[order])
			r=stringfind(sim->rxnss[order]->rname,sim->rxnss[order]->totrxn,rname);
	order--;
	if(r>=0) {
		if(orderptr) *orderptr=order;
		if(rxnpt) *rxnpt=sim->rxnss[order]->rxn[r]; }
	return r; }


/* rxnpackident */
int rxnpackident(int order,int maxspecies,int *ident) {
	if(order==0) return 0;
	if(order==1) return ident[0];
	if(order==2) return ident[0]*maxspecies+ident[1];
	return 0; }


/* rxnunpackident */
void rxnunpackident(int order,int maxspecies,int ipack,int *ident) {
	if(order==0);
	else if(order==1) ident[0]=ipack;
	else if(order==2) {ident[0]=ipack/maxspecies;ident[1]=ipack%maxspecies;}
	return; }


/* rxnpackstate */
enum MolecState rxnpackstate(int order,enum MolecState *mstate) {
	if(order==0) return (MolecState)0;
	if(order==1) return mstate[0];
	if(order==2) return (MolecState)(mstate[0]*MSMAX1+mstate[1]);
	return (MolecState)0; }


/* rxnunpackstate */
void rxnunpackstate(int order,enum MolecState mspack,enum MolecState *mstate) {
	if(order==0);
	else if(order==1) mstate[0]=mspack;
	else if(order==2) {mstate[0]=(MolecState)(mspack/MSMAX1);mstate[1]=(MolecState)(mspack%MSMAX1);}
	return; }


/* rxnallstates */
int rxnallstates(rxnptr rxn) {
	enum MolecState ms;
	int nms2o;

	if(rxn->rxnss->order==0) return 0;
	nms2o=intpower(MSMAX1,rxn->rxnss->order);
	for(ms=(MolecState)0;ms<nms2o && rxn->permit[ms];ms=(MolecState)(ms+1));
	if(ms==nms2o) return 1;
	return 0; }


/* rxnreactantstate */
int rxnreactantstate(rxnptr rxn,enum MolecState *mstate,int convertb2f) {
	int order,permit;
	enum MolecState ms,ms1,ms2;
	int mspair;

	order=rxn->rxnss->order;
	permit=0;

	if(order==0) permit=1;

	else if(order==1) {
		if(rxn->permit[MSsoln]) {
			ms=MSsoln;
			permit=1; }
		else if(rxn->permit[MSbsoln]) {
			ms=MSbsoln;
			permit=1; }
		else {
			for(ms=(MolecState)0;ms<MSMAX1 && !rxn->permit[ms];ms=(MolecState)(ms+1));
			if(ms<MSMAX1) permit=1; }
		if(permit && convertb2f && ms==MSbsoln) ms=MSsoln;
		if(mstate) {
			if(permit) mstate[0]=ms;
			else mstate[0]=MSnone; }}

	else if(order==2) {
		if(rxn->permit[MSsoln*MSMAX1+MSsoln]) {
			ms1=ms2=MSsoln;
			permit=1; }
		else if(rxn->permit[MSsoln*MSMAX1+MSbsoln]) {
			ms1=MSsoln;
			ms2=MSbsoln;
			permit=1; }
		else if(rxn->permit[MSbsoln*MSMAX1+MSsoln]) {
			ms1=MSbsoln;
			ms2=MSsoln;
			permit=1; }
		else if(rxn->permit[MSbsoln*MSMAX1+MSbsoln]) {
			ms1=MSbsoln;
			ms2=MSbsoln;
			permit=1; }
		if(!permit) {
			for(ms1=(MolecState)0;ms1<MSMAX1 && !rxn->permit[ms1*MSMAX1+MSsoln];ms1=(MolecState)(ms1+1));
			if(ms1<MSMAX1) {
				ms2=MSsoln;
				permit=1; }}
		if(!permit) {
			for(ms2=(MolecState)0;ms2<MSMAX1 && !rxn->permit[MSsoln*MSMAX1+ms2];ms2=(MolecState)(ms2+1));
			if(ms2<MSMAX1) {
				ms1=MSsoln;
				permit=1; }}
		if(!permit) {
			for(mspair=0;mspair<MSMAX1*MSMAX1 && !rxn->permit[mspair];mspair++);
			if(mspair<MSMAX1*MSMAX1) {
				ms1=(MolecState)(mspair/MSMAX1);
				ms2=(MolecState)(mspair%MSMAX1);
				permit=1; }}
		if(permit && convertb2f) {
			if(ms1==MSbsoln) ms1=MSsoln;
			if(ms2==MSbsoln) ms2=MSsoln; }
		if(mstate) {
			mstate[0]=permit?ms1:MSnone;
			mstate[1]=permit?ms2:MSnone; }}

	return permit; }


/* findreverserxn */
int findreverserxn(simptr sim,int order,int r,int *optr,int *rptr) {
	rxnssptr rxnss,rxnssr;
	rxnptr rxn,rxnr;
	int orderr,rr,rrreturn,rev,identr,identrprd,j,jr,work[MAXORDER];
	enum MolecState mstater,mstaterprd;

	if(!sim || order<0 || order>MAXORDER || r<0) return -1;
	rxnss=sim->rxnss[order];
	if(!rxnss || r>=rxnss->totrxn) return -1;
	rxn=rxnss->rxn[r];

	orderr=rrreturn=0;
	if(order==0 || rxn->nprod==0 || rxn->nprod>=MAXORDER || !sim->rxnss[rxn->nprod]) rev=0;
	else {
		orderr=rxn->nprod;
		rxnssr=sim->rxnss[orderr];
		identr=rxnpackident(orderr,rxnssr->maxspecies,rxn->prdident);
		mstater=rxnpackstate(orderr,rxn->prdstate);

		rev=0;
		for(j=0;j<rxnssr->nrxn[identr];j++) {
			rr=rxnssr->table[identr][j];
			rxnr=rxnssr->rxn[rr];
			if(rxnr->permit[mstater]) {
				if(rev!=1 && rxnr->nprod==order && Zn_sameset(rxn->rctident,rxnr->prdident,work,order)) {
					identrprd=rxnpackident(order,rxnss->maxspecies,rxnr->prdident);
					mstaterprd=rxnpackstate(order,rxnr->prdstate);
					for(jr=0;jr<rxnss->nrxn[identrprd];jr++)
						if(rxnss->table[identrprd][jr]==r && rxnss->rxn[r]->permit[mstaterprd]) {
							rev=1;
							rrreturn=rr; }}
				if(!rev) {
					rev=2;
					rrreturn=rr; }}}}

	if(optr) *optr=orderr;
	if(rptr) *rptr=rrreturn;
	return rev; }


/* rxnisprod */
int rxnisprod(simptr sim,int i,enum MolecState ms,int code) {
	int order,r,prd;
	rxnssptr rxnss;
	rxnptr rxn;

	for(order=0;order<MAXORDER;order++) {
		rxnss=sim->rxnss[order];
		if(rxnss) {
			for(r=0;r<rxnss->totrxn;r++) {
				rxn=rxnss->rxn[r];
				for(prd=0;prd<rxn->nprod;prd++)
					if(rxn->prdident[prd]==i && rxn->prdstate[prd]==ms) {
						if(code==0) return 1;
						if(rxn->rparamt==RPconfspread || rxn->unbindrad>0) return 1;
						if(dotVVD(rxn->prdpos[prd],rxn->prdpos[prd],sim->dim)>0) return 1; }}}}
	return 0; }


/******************************************************************************/
/****************************** memory management *****************************/
/******************************************************************************/


/* rxnalloc */
rxnptr rxnalloc(int order) {
	rxnptr rxn;
	int rct,nms2o;
	enum MolecState ms;

	CHECKMEM(rxn=(rxnptr) malloc(sizeof(struct rxnstruct)));
	rxn->rxnss=NULL;
	rxn->rname=NULL;
	rxn->rctident=NULL;
	rxn->rctstate=NULL;
	rxn->permit=NULL;
	rxn->nprod=0;
	rxn->prdident=NULL;
	rxn->prdstate=NULL;
	rxn->prdserno=NULL;
	rxn->prdintersurf=NULL;
	rxn->logserno=NULL;
	rxn->logfile=NULL;
	rxn->rate=-1;
#ifdef OPTION_VCELL
	rxn->rateValueProvider=NULL;
#endif
	rxn->bindrad2=-1;
	rxn->prob=-1;
	rxn->tau=-1;
	rxn->rparamt=RPnone;
	rxn->rparam=0;
	rxn->unbindrad=-1;
	rxn->prdpos=NULL;
	rxn->disable=0;
	rxn->cmpt=NULL;
	rxn->srf=NULL;

	if(order>0) {
		CHECKMEM(rxn->rctident=(int*)calloc(order,sizeof(int)));
		for(rct=0;rct<order;rct++) rxn->rctident[rct]=0;
		CHECKMEM(rxn->rctstate=(enum MolecState*)calloc(order,sizeof(int)));
		for(rct=0;rct<order;rct++) rxn->rctstate[rct]=MSnone;
		nms2o=intpower(MSMAX1,order);
		CHECKMEM(rxn->permit=(int*)calloc(nms2o,sizeof(int)));
		for(ms=(MolecState)0;ms<nms2o;ms=(MolecState)(ms+1)) rxn->permit[ms]=0; }
	return rxn;

 failure:
	rxnfree(rxn);
	simLog(NULL,10,"Unable to allocate memory in rxnalloc");
	return NULL; }


/* rxnfree */
void rxnfree(rxnptr rxn) {
	int prd;

	if(!rxn) return;
	if(rxn->prdpos)
		for(prd=0;prd<rxn->nprod;prd++)
			free(rxn->prdpos[prd]);
	free(rxn->prdpos);
	free(rxn->prdstate);
	free(rxn->prdserno);
	free(rxn->prdintersurf);
	List_FreeLI(rxn->logserno);
	free(rxn->logfile);
	free(rxn->prdident);
	free(rxn->permit);
	free(rxn->rctstate);
	free(rxn->rctident);
	free(rxn);
	return; }


/* rxnssalloc */
rxnssptr rxnssalloc(rxnssptr rxnss,int order,int maxspecies) {
	int newni2o,oldni2o,i,i2,failfree;
	int ilist[MAXORDER],*newnrxn,**newtable;

	failfree=0;
	if(!rxnss) {																				// new reaction superstructure
		rxnss=(rxnssptr) malloc(sizeof(struct rxnsuperstruct));
		CHECKMEM(rxnss);
		failfree=1;
		rxnss->condition=SCinit;
		rxnss->sim=NULL;
		rxnss->order=order;
		rxnss->maxspecies=0;
		rxnss->maxlist=0;
		rxnss->nrxn=NULL;
		rxnss->table=NULL;
		rxnss->maxrxn=0;
		rxnss->totrxn=0;
		rxnss->rname=NULL;
		rxnss->rxn=NULL;
		rxnss->rxnmollist=NULL; }

	if(maxspecies>rxnss->maxspecies) {									// initialize or expand nrxn and table
		if(order>0) {
			newni2o=intpower(maxspecies,order);							// allocate new stuff
			CHECKMEM(newnrxn=(int*) calloc(newni2o,sizeof(int)));
			for(i=0;i<newni2o;i++) newnrxn[i]=0;
			CHECKMEM(newtable=(int**) calloc(newni2o,sizeof(int*)));
			for(i=0;i<newni2o;i++) newtable[i]=NULL;

			oldni2o=intpower(rxnss->maxspecies,order);
			for(i=0;i<oldni2o;i++) {												// copy over old nrxn and table
				rxnunpackident(order,rxnss->maxspecies,i,ilist);
				i2=rxnpackident(order,maxspecies,ilist);
				newnrxn[i2]=rxnss->nrxn[i];
				newtable[i2]=rxnss->table[i]; }

			free(rxnss->nrxn);															// replace nrxn and table with new ones
			rxnss->nrxn=newnrxn;
			free(rxnss->table);
			rxnss->table=newtable; }
		rxnss->maxspecies=maxspecies; }										// set maxspecies

	return rxnss;

 failure:
	if(failfree) rxnssfree(rxnss);
	simLog(NULL,10,"Unable to allocate memory in rxnssalloc");
	return NULL; }


/* rxnssfree */
void rxnssfree(rxnssptr rxnss) {
	int r,i,ni2o;

	if(!rxnss) return;
	free(rxnss->rxnmollist);
	if(rxnss->rxn)
		for(r=0;r<rxnss->maxrxn;r++) rxnfree(rxnss->rxn[r]);
	free(rxnss->rxn);
	if(rxnss->rname)
		for(r=0;r<rxnss->maxrxn;r++) free(rxnss->rname[r]);
	free(rxnss->rname);
	if(rxnss->table) {
		ni2o=intpower(rxnss->maxspecies,rxnss->order);
		for(i=0;i<ni2o;i++) free(rxnss->table[i]);
		free(rxnss->table); }
	free(rxnss->nrxn);
	free(rxnss);
	return; }


/* rxnexpandmaxspecies */
int rxnexpandmaxspecies(simptr sim,int maxspecies) {
	rxnssptr rxnss;
	int order;

	for(order=0;order<MAXORDER;order++) {
		if(sim->rxnss[order] && sim->rxnss[order]->maxspecies<maxspecies) {
			rxnss=sim->rxnss[order];
			rxnss=rxnssalloc(rxnss,order,maxspecies);
			if(!rxnss) return order+1; }}
	return 0; }


/******************************************************************************/
/**************************** data structure output ***************************/
/******************************************************************************/


/* rxnoutput */
void rxnoutput(simptr sim,int order) {
	rxnssptr rxnss;
	int d,dim,maxlist,maxll2o,ll,ord,ni2o,i,j,r,rct,prd,rev,identlist[MAXORDER],orderr,rr,i1,i2,o2,r2;
	long int serno;
	rxnptr rxn,rxnr;
	enum MolecState ms,ms1,ms2,nms2o,statelist[MAXORDER];
	double dsum,step,pgem,rate3,bindrad,rparam,ratio;
	char string[STRCHAR];
	enum RevParam rparamt;

	simLog(sim,2,"ORDER %i REACTION PARAMETERS\n",order);
	if(!sim || !sim->mols || !sim->rxnss[order]) {
		simLog(sim,2," No reactions of order %i\n\n",order);
		return; }
	rxnss=sim->rxnss[order];
	if(rxnss->condition!=SCok)
		simLog(sim,2," structure condition: %s\n",simsc2string(rxnss->condition,string));
	dim=sim->dim;

	simLog(sim,1," allocated for %i species\n",rxnss->maxspecies-1);
	simLog(sim,1," allocated for %i molecule lists\n",rxnss->maxlist);

	simLog(sim,2," %i reactions defined",rxnss->totrxn);
	simLog(sim,1,", of %i allocated",rxnss->maxrxn);
	simLog(sim,2,"\n");
	
	if(order>0) {
		simLog(sim,2," Reactive molecule lists:");
		if(!rxnss->rxnmollist || !sim->mols->nlist)
			simLog(sim,2," not set up yet");
		else {
			maxlist=sim->mols->maxlist;
			maxll2o=intpower(maxlist,order);
			for(ll=0;ll<maxll2o;ll++)
				if(rxnss->rxnmollist[ll]) {
					simLog(sim,2," ");
					for(ord=0;ord<order;ord++)
						simLog(sim,2,"%s%s",sim->mols->listname[(ll/intpower(maxlist,ord))%maxlist],ord<order-1?"+":""); }}
		simLog(sim,2,"\n"); }

	if(order>0) {
		simLog(sim,2," Reactants, sorted by molecule species:\n");
		ni2o=intpower(rxnss->maxspecies,order);
		for(i=0;i<ni2o;i++)
			if(rxnss->nrxn[i]) {
				rxnunpackident(order,rxnss->maxspecies,i,identlist);
				if(Zn_issort(identlist,order)>=1) {
					simLog(sim,2,"  ");
					for(ord=0;ord<order;ord++)
						simLog(sim,2,"%s%s",sim->mols->spname[identlist[ord]],ord<order-1?"+":"");
					simLog(sim,2," :");
					for(j=0;j<rxnss->nrxn[i];j++) simLog(sim,2," %s",rxnss->rname[rxnss->table[i][j]]);
					simLog(sim,2,"\n"); }}}

	simLog(sim,2," Reaction details:\n");
	for(r=0;r<rxnss->totrxn;r++) {
		rxn=rxnss->rxn[r];
		simLog(sim,2,"  Reaction %s:",rxn->rname);
		if(order==0) simLog(sim,2," 0");							// reactants
		for(rct=0;rct<order;rct++) {
			simLog(sim,2," %s",sim->mols->spname[rxn->rctident[rct]]);
			if(rxn->rctstate[rct]!=MSsoln) simLog(sim,2," (%s)",molms2string(rxn->rctstate[rct],string));
			if(rct<order-1) simLog(sim,2," +"); }
		simLog(sim,2," ->");
		if(rxn->nprod==0) simLog(sim,2," 0");					// products
		for(prd=0;prd<rxn->nprod;prd++) {
			simLog(sim,2," %s",sim->mols->spname[rxn->prdident[prd]]);
			if(rxn->prdstate[prd]!=MSsoln) simLog(sim,2," (%s)",molms2string(rxn->prdstate[prd],string));
			if(prd<rxn->nprod-1) simLog(sim,2," +"); }
		simLog(sim,2,"\n");

		for(rct=0;rct<order;rct++)							// permit
			if(rxn->rctstate[rct]==MSsome) rct=order+1;
		if(rct==order+1) {
			simLog(sim,2,"   permit:");
			nms2o=MolecState(intpower(MSMAX1,order));
			for(ms=MolecState(0);ms<nms2o;ms=MolecState(ms + 1)) {
				if(rxn->permit[ms]) {
					rxnunpackstate(order,ms,statelist);
					simLog(sim,2," ");
					for(ord=0;ord<order;ord++)
						simLog(sim,2,"%s%s",molms2string(statelist[ms],string),ord<order-1?"+":""); }}}

		if(rxn->prdserno) {											// product serial number rules
			simLog(sim,2,"   serial number rules:");
			for(prd=0;prd<rxn->nprod;prd++) {
				serno=rxn->prdserno[prd];
				if(serno==0) simLog(sim,2," new");
				else if(serno==-1) simLog(sim,2," r1");
				else if(serno==-2) simLog(sim,2," r2");
				else if(serno<=-10) simLog(sim,2," p%i",(int)(-9-serno));
				else simLog(sim,2," %li",serno); }
			simLog(sim,2,"\n"); }

		if(rxn->prdintersurf) {									// product intersurface rules
			simLog(sim,2,"   intersurface rules:");
			if(!rxn->nprod)
				simLog(sim,2," allowed");
			else {
				for(prd=0;prd<rxn->nprod;prd++) {
					i1=rxn->prdintersurf[prd];
					if(i1==1) simLog(sim,2," r1");
					else simLog(sim,2," r2"); }}
			simLog(sim,2,"\n"); }

		if(rxn->logserno) {											// reaction log
			simLog(sim,2,"   reaction logged to file %s",rxn->logfile);
			if(rxn->logserno->n==0)
				simLog(sim,2," for all molecules\n");
			else {
				simLog(sim,2,"\n    molecules:");
				for(i=0;i<rxn->logserno->n && i<5;i++)
					simLog(sim,2," %li",rxn->logserno->xs[i]);
				if(i<rxn->logserno->n)
					simLog(sim,2,"...%li",rxn->logserno->xs[rxn->logserno->n-1]);
				simLog(sim,2,"\n"); }}

		if(rxn->cmpt) simLog(sim,2,"   compartment: %s\n",rxn->cmpt->cname);
		if(rxn->srf) simLog(sim,2,"   surface: %s\n",rxn->srf->sname);
		if(rxn->rate>=0) simLog(sim,2,"   requested and actual rate constants: %g, %g\n",rxn->rate,rxncalcrate(sim,order,r,&pgem));
		else simLog(sim,2,"   actual rate constant: %g\n",rxncalcrate(sim,order,r,&pgem));
		if(pgem>=0) simLog(sim,2,"   geminate recombination probability: %g\n",pgem);
		if(rxn->rparamt==RPconfspread) simLog(sim,2,"   conformational spread reaction\n");
		if(rxn->tau>=0) simLog(sim,2,"   characteristic time: %g\n",rxn->tau);
		if(order==0) simLog(sim,2,"   average reactions per time step: %g\n",rxn->prob);
		else if(order==1) simLog(sim,2,"   conditional reaction probability per time step: %g\n",rxn->prob);			// this is the conditional probability, with the condition that prior possible reactions did not happen
		else if(rxn->prob>=0 && rxn->prob!=1) simLog(sim,2,"   reaction probability after collision: %g\n",rxn->prob);
		if(rxn->bindrad2>=0) simLog(sim,2,"   binding radius: %g\n",sqrt(rxn->bindrad2));

		if(rxn->nprod==2) {
			if(rxn->unbindrad>=0) simLog(sim,2,"   unbinding radius: %g\n",rxn->unbindrad);
			else if(rxn->rparamt==RPbounce) simLog(sim,2,"   unbinding radius: calculated from molecule overlap\n");
			else simLog(sim,2,"   unbinding radius: 0\n");
			if(rxn->rparamt!=RPconfspread && rxn->rparamt!=RPbounce && findreverserxn(sim,order,r,&o2,&r2)==1) {
				rxnr=sim->rxnss[o2]->rxn[r2];
				dsum=MolCalcDifcSum(sim,rxn->prdident[0],rxn->prdstate[0],rxn->prdident[1],rxn->prdstate[1]);
				rate3=rxncalcrate(sim,o2,r2,NULL);
				if(rxn->prdident[0]==rxn->prdident[1]) rate3*=2;				// same reactants
				rparamt=rxn->rparamt;
				rparam=rxn->rparam;
				if(rparamt==RPunbindrad) bindrad=bindingradius(rate3,0,dsum,rparam,0);
				else if(rparamt==RPratio) bindrad=bindingradius(rate3,0,dsum,rparam,1);
				else if(rparamt==RPpgem || rparamt==RPpgemmax || rparamt==RPpgemmaxw) {
					bindrad=bindingradius(rate3*(1.0-rparam),0,dsum,-1,0);
					pgem=rparam; }
				else bindrad=bindingradius(rate3,0,dsum,-1,0);
				simLog(sim,2,"   unbinding radius if dt were 0: %g\n",unbindingradius(pgem,0,dsum,bindrad)); }}

		if(order==2 && rxnreactantstate(rxn,statelist,1)) {
			ms1=statelist[0];
			ms2=statelist[1];
			i1=rxn->rctident[0];
			i2=rxn->rctident[1];
			dsum=MolCalcDifcSum(sim,i1,ms1,i2,ms2);
			rev=findreverserxn(sim,2,r,&o2,&r2);
			rate3=rxncalcrate(sim,order,r,NULL);
			if(i1==i2) rate3*=2;				// same reactants
			if(rev==1) {
				rparamt=sim->rxnss[o2]->rxn[r2]->rparamt;
				rparam=sim->rxnss[o2]->rxn[r2]->rparam; }
			else {
				rparamt=RPnone;
				rparam=0; }
			if(rparamt==RPconfspread) bindrad=-1;
			else if(rparamt==RPbounce) bindrad=-1;
			else if(rparamt==RPunbindrad) bindrad=bindingradius(rate3,0,dsum,rparam,0);
			else if(rparamt==RPratio) bindrad=bindingradius(rate3,0,dsum,rparam,1);
			else if(rparamt==RPpgem || rparamt==RPpgemmax || rparamt==RPpgemmaxw) bindrad=bindingradius(rate3*(1.0-rparam),0,dsum,-1,0);
			else bindrad=bindingradius(rate3,0,dsum,-1,0);
			if(bindrad>=0) simLog(sim,2,"   binding radius if dt were 0: %g\n",bindrad);
			step=sqrt(2.0*dsum*sim->dt);
			ratio=step/sqrt(rxn->bindrad2);
			simLog(sim,2,"   mutual rms step length: %g\n",step);
			if(step>0) {
				simLog(sim,2,"   step length / binding radius: %g (%s %s-limited)\n",ratio,ratio>0.1 && ratio<10?"somewhat":"very",ratio>1?"activation":"diffusion");
				simLog(sim,2,"   effective activation limited reaction rate: %g\n",actrxnrate(step,sqrt(rxn->bindrad2))/sim->dt); }}

		if(rxn->rparamt!=RPbounce)
			if(findreverserxn(sim,order,r,&orderr,&rr)==1)
				simLog(sim,2,"   with reverse reaction %s, equilibrium constant is: %g\n",sim->rxnss[orderr]->rname[rr],rxn->rate/sim->rxnss[orderr]->rxn[rr]->rate);
		if(rxn->rparamt!=RPnone) simLog(sim,2,"   product placement method and parameter: %s %g\n",rxnrp2string(rxn->rparamt,string),rxn->rparam);
		for(prd=0;prd<rxn->nprod;prd++) {
			if(dotVVD(rxn->prdpos[prd],rxn->prdpos[prd],dim)>0) {
				simLog(sim,2,"   product %s displacement:",sim->mols->spname[rxn->prdident[prd]]);
				for(d=0;d<dim;d++) simLog(sim,2," %g",rxn->prdpos[prd][d]);
				simLog(sim,2,"\n"); }}

		// The following segment does not account for non-1 reaction probabilities.
		if(rxn->nprod==2 && sim->rxnss[2] && rxn->rparamt!=RPconfspread && rxn->rparamt!=RPbounce) {
			i=rxnpackident(2,rxnss->maxspecies,rxn->prdident);
			for(j=0;j<sim->rxnss[2]->nrxn[i];j++) {
				rr=sim->rxnss[2]->table[i][j];
				rxnr=sim->rxnss[2]->rxn[rr];
				if(rxnr->bindrad2>=0 && rxnr->rparamt!=RPconfspread) {
					dsum=MolCalcDifcSum(sim,rxn->prdident[0],rxn->prdstate[0],rxn->prdident[1],rxn->prdstate[1]);
					step=sqrt(2.0*sim->dt*dsum);
					pgem=1.0-numrxnrate(step,sqrt(rxnr->bindrad2),-1)/numrxnrate(step,sqrt(rxnr->bindrad2),rxn->unbindrad);
					rev=(rxnr->nprod==order);
					rev=rev && Zn_sameset(rxnr->prdident,rxn->rctident,identlist,order);
					simLog(sim,2,"   probability of geminate %s reaction '%s' is %g\n",rev?"reverse":"continuation",rxnr->rname,pgem); }}}}

	simLog(sim,2,"\n");
	return; }


/* writereactions */
void writereactions(simptr sim,FILE *fptr) {
	int order,r,prd,d,rct,i,i1;
	long int serno;
	rxnptr rxn;
	rxnssptr rxnss;
	char string[STRCHAR],string2[STRCHAR];
	enum MolecState ms,ms1,ms2;
	enum RevParam rparamt;

	fprintf(fptr,"# Reaction parameters\n");
	for(order=0;order<=2;order++)
		if(sim->rxnss[order]) {
			rxnss=sim->rxnss[order];
			for(r=0;r<rxnss->totrxn;r++) {
				rxn=rxnss->rxn[r];

				if(rxn->cmpt) fprintf(fptr,"reaction_cmpt %s",rxn->cmpt->cname);
				else if(rxn->srf) fprintf(fptr,"reaction_surface %s",rxn->srf->sname);
				else fprintf(fptr,"reaction");
				fprintf(fptr," %s",rxn->rname);
				if(order==0) fprintf(fptr," 0");							// reactants
				for(rct=0;rct<order;rct++) {
					fprintf(fptr," %s",sim->mols->spname[rxn->rctident[rct]]);
					if(rxn->rctstate[rct]!=MSsoln) fprintf(fptr,"(%s)",molms2string(rxn->rctstate[rct],string));
					if(rct<order-1) fprintf(fptr," +"); }
				fprintf(fptr," ->");
				if(rxn->nprod==0) fprintf(fptr," 0");					// products
				for(prd=0;prd<rxn->nprod;prd++) {
					fprintf(fptr," %s",sim->mols->spname[rxn->prdident[prd]]);
					if(rxn->prdstate[prd]!=MSsoln) fprintf(fptr,"(%s)",molms2string(rxn->prdstate[prd],string));
					if(prd<rxn->nprod-1) fprintf(fptr," +"); }
				if(rxn->rate>=0) fprintf(fptr," %g",rxn->rate);
				fprintf(fptr,"\n");

				if(order==1 && rxn->rctstate[0]==MSsome) {
					for(ms=(MolecState)0;ms<MSMAX;ms=(MolecState)(ms+1))
						if(rxn->permit[ms])
							fprintf(fptr,"reaction_permit %s %s\n",rxn->rname,molms2string(ms,string)); }
				else if(order==2 && (rxn->rctstate[0]==MSsome || rxn->rctstate[1]==MSsome)) {
					for(ms1=(MolecState)0;ms1<MSMAX1;ms1=(MolecState)(ms1+1))
						for(ms2=(MolecState)0;ms2<MSMAX1;ms2=(MolecState)(ms2+1))
							if(rxn->permit[ms1*MSMAX1+ms2])
								fprintf(fptr,"reaction_permit %s %s %s\n",rxn->rname,molms2string(ms1,string),molms2string(ms2,string2)); }

				if(rxn->rparamt==RPconfspread)
					fprintf(fptr,"confspread_radius %s %g\n",rxn->rname,rxn->bindrad2<0?0:sqrt(rxn->bindrad2));
				if(rxn->rate<0) {
					if(order==0 && rxn->prob>=0) fprintf(fptr,"reaction_production %s %g\n",rxn->rname,rxn->prob);
					else if(order==1 && rxn->prob>=0) fprintf(fptr,"reaction_probability %s %g\n",rxn->rname,rxn->prob);
					else if(order==2 && rxn->bindrad2>=0) fprintf(fptr,"binding_radius %s %g\n",rxn->rname,sqrt(rxn->bindrad2)); }
				if((order==2 && rxn->prob!=1 && rxn->rparamt!=RPconfspread) || (rxn->rparamt==RPconfspread && rxn->rate<0))
					fprintf(fptr,"reaction_probability %s %g\n",rxn->rname,rxn->prob);

				rparamt=rxn->rparamt;
				if(rparamt==RPirrev)
					fprintf(fptr,"product_placement %s irrev\n",rxn->rname);
				else if(rparamt==RPbounce || rparamt==RPpgem || rparamt==RPpgemmax || rparamt==RPratio || rparamt==RPunbindrad || rparamt==RPpgem2 || rparamt==RPpgemmax2 || rparamt==RPratio2)
					fprintf(fptr,"product_placement %s %s %g\n",rxn->rname,rxnrp2string(rparamt,string),rxn->rparam);
				else if(rparamt==RPoffset || rparamt==RPfixed) {
					for(prd=0;prd<rxn->nprod;prd++) {
						fprintf(fptr,"product_placement %s %s %s\n",rxn->rname,rxnrp2string(rparamt,string),sim->mols->spname[rxn->prdident[prd]]);
						for(d=0;d<sim->dim;d++) fprintf(fptr," %g",rxn->prdpos[prd][d]);
						fprintf(fptr,"\n"); }}

				if(rxn->prdserno) {
					fprintf(fptr,"reaction_serialnum %s",rxn->rname);
					for(prd=0;prd<rxn->nprod;prd++) {
						serno=rxn->prdserno[prd];
						if(serno==0) fprintf(fptr," new");
						else if(serno==-1) fprintf(fptr," r1");
						else if(serno==-2) fprintf(fptr," r2");
						else if(serno<=-10) fprintf(fptr," p%i",(int)(-9-serno));
						else fprintf(fptr," %li",serno); }
					fprintf(fptr,"\n"); }

				if(rxn->prdintersurf) {
					fprintf(fptr,"reaction_intersurface %s",rxn->rname);
					if(!rxn->nprod)
						fprintf(fptr," on");
					else {
						for(prd=0;prd<rxn->nprod;prd++) {
							i1=rxn->prdserno[prd];
							if(i1==1) fprintf(fptr," r1");
							else fprintf(fptr," r2"); }}
					fprintf(fptr,"\n"); }

				if(rxn->logserno) {											// reaction log
					fprintf(fptr,"reaction_log %s %s",rxn->logfile,rxn->rname);
					if(rxn->logserno->n==0)
						fprintf(fptr," all\n");
					else {
						for(i=0;i<rxn->logserno->n;i++)
							fprintf(fptr," %li",rxn->logserno->xs[i]);
						fprintf(fptr,"\n"); }}}}

	fprintf(fptr,"\n");
	return; }


/* checkrxnparams */
int checkrxnparams(simptr sim,int *warnptr) {
	int d,dim,warn,error,i1,i2,j,nspecies,r,i,ct,j1,j2,order,prd,prd2;
	long int serno;
	molssptr mols;
	double minboxsize,vol,amax,vol2,vol3;
	rxnptr rxn,rxn1,rxn2;
	rxnssptr rxnss;
	char **spname,string[STRCHAR],string2[STRCHAR];
	enum MolecState ms1,ms2,ms;

	error=warn=0;
	dim=sim->dim;
	nspecies=sim->mols->nspecies;
	mols=sim->mols;
	spname=sim->mols->spname;

	for(order=0;order<=2;order++) {										// condition
		rxnss=sim->rxnss[order];
		if(rxnss) {
			if(rxnss->condition!=SCok) {
				warn++;
				simLog(sim,5," WARNING: order %i reaction structure %s\n",order,simsc2string(rxnss->condition,string)); }}}

	for(order=0;order<=2;order++) {										// maxspecies
		rxnss=sim->rxnss[order];
		if(rxnss) {
			if(!sim->mols) {error++;simLog(sim,10," SMOLDYN BUG: Reactions are declared, but not molecules\n");}
			else if(sim->mols->maxspecies!=rxnss->maxspecies) {error++;simLog(sim,10," SMOLDYN BUG: number of molecule species differ between mols and rxnss\n");} }}

	for(order=1;order<=2;order++) {									// reversible parameters
		rxnss=sim->rxnss[order];
		if(rxnss)
			for(r=0;r<rxnss->totrxn;r++) {
				rxn=rxnss->rxn[r];
				if(rxn->rparamt==RPpgemmaxw) {
					simLog(sim,5," WARNING: unspecified product parameter for reversible reaction %s.  Defaults are used.\n",rxn->rname);
					warn++; }}}

	rxnss=sim->rxnss[2];															// check for multiple bimolecular reactions with same reactants
	if(rxnss) {
		for(i1=1;i1<nspecies;i1++)
			for(i2=1;i2<=i1;i2++)	{
				i=i1*rxnss->maxspecies+i2;
				for(j1=0;j1<rxnss->nrxn[i];j1++) {
					rxn1=rxnss->rxn[rxnss->table[i][j1]];
					for(j2=0;j2<j1;j2++) {
						rxn2=rxnss->rxn[rxnss->table[i][j2]];
						if(rxnallstates(rxn1) && rxnallstates(rxn2)) {
							simLog(sim,5," WARNING: multiply defined bimolecular reactions: %s(all) + %s(all)\n",spname[i1],spname[i2]);
							warn++; }
						else if(rxnallstates(rxn1)) {
							for(ms2=(MolecState)0;ms2<=MSMAX1;ms2=(MolecState)(ms2+1)) {
								ms=(MolecState)(MSsoln*MSMAX1+ms2);
								if(rxn2->permit[ms]) {
									simLog(sim,5," WARNING: multiply defined bimolecular reactions: %s(all) + %s(%s)\n",spname[i1],spname[i2],molms2string(ms2,string2));
									warn++; }}}
						else if(rxnallstates(rxn2)) {
							for(ms1=(MolecState)0;ms1<=MSMAX1;ms1=(MolecState)(ms1+1)) {
								ms=(MolecState)(ms1*MSMAX1+MSsoln);
								if(rxn1->permit[ms]) {
									simLog(sim,5," WARNING: multiply defined bimolecular reactions: %s(%s) + %s(all)\n",spname[i1],molms2string(ms1,string),spname[i2]);
									warn++; }}}
						else {
							for(ms1=(MolecState)0;ms1<MSMAX1;ms1=(MolecState)(ms1+1))
								for(ms2=(MolecState)0;ms2<=ms1;ms2=(MolecState)(ms2+1)) {
									ms=(MolecState)(ms1*MSMAX1+ms2);
									if(rxn1->permit[ms] && rxn2->permit[ms]) {
										simLog(sim,5," WARNING: multiply defined bimolecular reactions: %s(%s) + %s(%s)\n",spname[i1],molms2string(ms1,string),spname[i2],molms2string(ms2,string2));
										warn++; }}}}}}}

	rxnss=sim->rxnss[2];														// warn that difm ignored for reaction rates
	if(rxnss)
		for(i1=1;i1<nspecies;i1++)
			if(mols->difm[i1][MSsoln])
				for(i2=1;i2<i1;i2++)
					for(j=0;j<rxnss->nrxn[i=i1*rxnss->maxspecies+i2];j++) {
						rxn=rxnss->rxn[rxnss->table[i][j]];
						if(rxn->rate) {
							simLog(sim,5," WARNING: diffusion matrix for %s was ignored for calculating rate for reaction %s\n",spname[i1],rxn->rname);
							warn++; }}

	rxnss=sim->rxnss[2];														// warn that drift ignored for reaction rates
	if(rxnss)
		for(i1=1;i1<nspecies;i1++)
			if(mols->drift[i1][MSsoln])
				for(i2=1;i2<i1;i2++)
					for(j=0;j<rxnss->nrxn[i=i1*rxnss->maxspecies+i2];j++) {
						rxn=rxnss->rxn[rxnss->table[i][j]];
						if(rxn->rate) {
							simLog(sim,5," WARNING: drift vector for %s was ignored for calculating rate for reaction %s\n",spname[i1],rxn->rname);
							warn++; }}

	for(order=1;order<=2;order++) {									// product surface-bound states imply reactant surface-bound
		rxnss=sim->rxnss[order];
		if(rxnss) {
			for(i=1;i<intpower(rxnss->maxspecies,order);i++)
				for(j=0;j<rxnss->nrxn[i];j++) {
					rxn=rxnss->rxn[rxnss->table[i][j]];
					if(rxn->permit[order==1?MSsoln:MSsoln*MSMAX1+MSsoln]) {
						for(prd=0;prd<rxn->nprod;prd++)
							if(rxn->prdstate[prd]!=MSsoln) {
								simLog(sim,10," ERROR: a product of reaction %s is surface-bound, but no reactant is\n",rxn->rname);
								error++; }}}}}

	for(order=0;order<=2;order++) {									// reaction compartment
		rxnss=sim->rxnss[order];
		if(rxnss)
			for(r=0;r<rxnss->totrxn;r++) {
				rxn=rxnss->rxn[r];
				if(rxn->cmpt) {
					if(order==0 && rxn->cmpt->volume<=0) {
						simLog(sim,10," ERROR: reaction %s cannot work in compartment %s because the compartment has no volume\n",rxn->rname,rxn->cmpt->cname);
						error++; }}}}

	for(order=0;order<=2;order++) {									// reaction surface
		rxnss=sim->rxnss[order];
		if(rxnss)
			for(r=0;r<rxnss->totrxn;r++) {
				rxn=rxnss->rxn[r];
				if(rxn->srf) {
					if(order==0 && surfacearea(rxn->srf,sim->dim,NULL)<=0) {
						simLog(sim,10," ERROR: reaction %s cannot work on surface %s because the surface has no area\n",rxn->rname,rxn->srf->sname);
						error++; }}}}

	for(order=0;order<=2;order++) {									// reaction serial numbers
		rxnss=sim->rxnss[order];
		if(rxnss)
			for(r=0;r<rxnss->totrxn;r++) {
				rxn=rxnss->rxn[r];
				if(rxn->prdserno) {
					j=0;
					for(prd=0;prd<rxn->nprod;prd++) {
						serno=rxn->prdserno[prd];
						if(serno<=-10) j=1;
						else if(serno>0) j=1;
						else if(serno==-1 || serno==-2)
							for(prd2=prd+1;prd2<rxn->nprod;prd2++)
								if(rxn->prdserno[prd2]==serno) j=1; }
					if(j) {
						simLog(sim,5," WARNING: multiple molecules might have the same serial number due to reaction %s\n",rxn->rname);
						warn++; }}}}

	rxnss=sim->rxnss[0];														// order 0 reactions
	if(rxnss)
		for(r=0;r<rxnss->totrxn;r++) {
			rxn=rxnss->rxn[r];
			if(!rxn->srf) {
				for(prd=0;prd<rxn->nprod;prd++)
					if(rxn->prdstate[prd]!=MSsoln) {
						simLog(sim,10," ERROR: order 0 reaction %s has surface-bound products but no surface listed\n",rxn->rname);
						error++; }}
			if(rxn->prob<0) {
				simLog(sim,5," WARNING: reaction rate not set for reaction order 0, name %s\n",rxn->rname);
				rxn->prob=0;
				warn++; }}

	rxnss=sim->rxnss[1];														// order 1 reactions
	if(rxnss)
		for(r=0;r<rxnss->totrxn;r++) {
			rxn=rxnss->rxn[r];
			if(rxn->prob<0) {
				simLog(sim,5," WARNING: reaction rate not set for reaction order 1, name %s\n",rxn->rname);
				rxn->prob=0;
				warn++; }
			else if(rxn->prob>0 && rxn->prob<10.0/(double)RAND2_MAX) {
				simLog(sim,5," WARNING: order 1 reaction %s probability is at lower end of random number generator resolution\n",rxn->rname);
				warn++; }
			else if(rxn->prob>((double)RAND2_MAX-10.0)/(double)RAND2_MAX && rxn->prob<1.0) {
				simLog(sim,5," WARNING: order 1 reaction %s probability is at upper end of random number generator resolution\n",rxn->rname);
				warn++; }
			else if(rxn->prob>0.2) {
				simLog(sim,5," WARNING: order 1 reaction %s probability is quite high\n",rxn->rname);
				warn++; }
			if(rxn->tau<5*sim->dt) {
				simLog(sim,5," WARNING: order 1 reaction %s time constant is only %g times longer than the simulation time step\n",rxn->rname,rxn->tau/sim->dt);
				warn++; }}

	minboxsize=sim->boxs->size[0];
	for(d=1;d<dim;d++)
		if(sim->boxs->size[d]<minboxsize) minboxsize=sim->boxs->size[d];

	rxnss=sim->rxnss[2];															// order 2 reactions
	if(rxnss) {
		for(r=0;r<rxnss->totrxn;r++) {
			rxn=rxnss->rxn[r];
			if(rxn->bindrad2<0) {
				if(rxn->rparamt==RPconfspread) simLog(sim,5," WARNING: confspread radius not set for order 2 reaction %s\n",rxn->rname);
				else simLog(sim,5," WARNING: reaction rate not set for reaction order 2, name %s\n",rxn->rname);
				rxn->bindrad2=0;
				warn++; }
			else if(sqrt(rxn->bindrad2)>minboxsize) {
				if(rxn->rparamt==RPconfspread) simLog(sim,5," WARNING: confspread radius for order 2 reaction %s is larger than box dimensions\n",rxn->rname);
				else if(rxn->rparamt==RPbounce) simLog(sim,5," WARNING: bounce radius for order 2 reaction %s is larger than box dimensions\n",rxn->rname);
				else simLog(sim,5," WARNING: binding radius for order 2 reaction %s is larger than box dimensions\n",rxn->rname);
				warn++; }
			if(rxn->prob<0 || rxn->prob>1) {
				simLog(sim,10," ERROR: reaction %s probability is not between 0 and 1\n",rxn->rname);
				error++; }
			else if(rxn->prob<1 && rxn->rparamt!=RPconfspread && rxn->rparamt!=RPbounce) {
				simLog(sim,5," WARNING: reaction %s probability is not accounted for in rate calculation\n",rxn->rname);
				warn++; }
			if(rxn->tau<5*sim->dt) {
				simLog(sim,5," WARNING: order 2 reaction %s time constant is only %g times longer than the simulation time step\n",rxn->rname,rxn->tau/sim->dt);
				warn++; }}}

	rxnss=sim->rxnss[2];															// more order 2 reactions
	if(rxnss) {
		vol=systemvolume(sim);
		vol2=0;
		for(i=1;i<nspecies;i++) {
			amax=0;
			for(i1=1;i1<nspecies;i1++)
				for(j=0;j<rxnss->nrxn[i*rxnss->maxspecies+i1];j++) {
					r=rxnss->table[i*rxnss->maxspecies+i1][j];
					rxn=rxnss->rxn[r];
					if(amax<sqrt(rxn->bindrad2)) amax=sqrt(rxn->bindrad2); }
			ct=molcount(sim,i,NULL,MSsoln,NULL,-1);
			vol3=ct*4.0/3.0*PI*amax*amax*amax;
			vol2+=vol3;
			if(vol3>vol/10.0) {
				simLog(sim,5," WARNING: reactive volume of %s is %g %% of total volume\n",spname[i],vol3/vol*100);
				warn++; }}
		if(vol2>vol/10.0) {
			simLog(sim,5," WARNING: total reactive volume is a large fraction of total volume\n");
			warn++; }}

	if(warnptr) *warnptr=warn;
	return error; }


/******************************************************************************/
/*************************** parameter calculations ***************************/
/******************************************************************************/


/* rxnsetrate */
int rxnsetrate(simptr sim,int order,int r,char *erstr) {
	rxnssptr rxnss;
	int i,j,i1,i2,rev,o2,r2,permit;
	rxnptr rxn,rxn2;
	double vol,rate3,dsum,rparam,unbindrad,prob,product,sum;
	enum MolecState ms,ms1,ms2,statelist[MAXORDER];
	enum RevParam rparamt;

	rxnss=sim->rxnss[order];
	rxn=rxnss->rxn[r];

	if(rxn->disable) {															// disabled reaction
		rxn->prob=0; }

	else if(rxn->rparamt==RPconfspread) {						// confspread
		if(rxn->rate<0) {sprintf(erstr,"reaction %s rate is undefined",rxn->rname);return 1;}
		if(rxn->rate>=0) rxn->prob=1.0-exp(-sim->dt*rxn->rate); }

	else if(order==0) {															// order 0
		if(rxn->rate<0) {sprintf(erstr,"reaction %s rate is undefined",rxn->rname);return 1;}
		if(rxn->cmpt) vol=rxn->cmpt->volume;
		else if(rxn->srf) vol=surfacearea(rxn->srf,sim->dim,NULL);
		else vol=systemvolume(sim);
		rxn->prob=rxn->rate*sim->dt*vol; }

	else if(order==1) {															// order 1
		for(ms=(MolecState)0;ms<MSMAX && !rxn->permit[ms];ms=(MolecState)(ms+1));		// determine first state that this is permitted for (usually only state)
		if(rxn->rate<0) {sprintf(erstr,"reaction %s rate is undefined",rxn->rname);return 1;}
		else if(rxn->rate==0) rxn->prob=0;
		else if(ms==MSMAX) rxn->prob=0;
		else {
			i=rxn->rctident[0];
			sum=0;
			for(j=0;j<rxnss->nrxn[i];j++) {
				rxn2=rxnss->rxn[rxnss->table[i][j]];
				if(rxn2->permit[ms] && rxn2->rate>0) {
					for(ms1=(MolecState)0;ms1<MSMAX;ms1=(MolecState)(ms1+1))
						if(rxn->permit[ms1]!=rxn2->permit[ms1]) {sprintf(erstr,"reactions %s and %s have the same reactant but different state permissions, which is not allowed",rxn->rname,rxn2->rname);return 2;}
					sum+=rxn2->rate; }}
			rxn->prob=rxn->rate/sum*(1.0-exp(-sum*sim->dt));				// desired reaction probability

			product=1.0;
			for(j=0;j<rxnss->nrxn[i];j++) {		// increase reaction probability to account for prior reactions in the list
				rxn2=rxnss->rxn[rxnss->table[i][j]];
				if(rxn2==rxn) break;
				if(rxn2->permit[ms]) {
					prob=rxn2->prob*product;
					product*=1.0-prob; }}
			rxn->prob/=product;								// probability, accounting for prior reactions, see documentation
			if(!(rxn->prob>=0 && rxn->prob<=1)) {sprintf(erstr,"reaction %s probability is %g, which is out of range",rxn->rname,rxn->prob);return 5;}}

			rev=findreverserxn(sim,1,r,&o2,&r2);	// set own reversible parameter if needed
			if(rev>0 && o2==2) {
				if(rxn->rparamt==RPnone) {
					rxn->rparamt=RPpgemmaxw;
					rxn->rparam=0.2; }}}

	else if(order==2) {															// order 2
		if(rxn->rate<0) {
			if(rxn->prob<0) rxn->prob=1;
			sprintf(erstr,"reaction rate %s is undefined",rxn->rname);
			return 1; }
		i1=rxn->rctident[0];
		i2=rxn->rctident[1];

		permit=rxnreactantstate(rxn,statelist,1);
		ms1=statelist[0];
		ms2=statelist[1];

		rate3=rxn->rate;
		if(i1==i2) rate3*=2;				// same reactants
		if((ms1==MSsoln && ms2!=MSsoln) || (ms1!=MSsoln && ms2==MSsoln)) rate3*=2;	// one surface, one solution
		dsum=MolCalcDifcSum(sim,i1,ms1,i2,ms2);
		rev=findreverserxn(sim,2,r,&o2,&r2);

		if(rev>0 && o2==2) {						// set own reversible parameter if needed
			if(rxn->rparamt==RPnone) {
				rxn->rparamt=RPpgemmaxw;
				rxn->rparam=0.2; }}

		if(rev==1) {										// set incoming reversible parameter if needed, and then use it
			if(sim->rxnss[o2]->rxn[r2]->rparamt==RPnone) {
				sim->rxnss[o2]->rxn[r2]->rparamt=RPpgemmaxw;
				sim->rxnss[o2]->rxn[r2]->rparam=0.2; }
			rparamt=sim->rxnss[o2]->rxn[r2]->rparamt;
			rparam=sim->rxnss[o2]->rxn[r2]->rparam; }
		else {
			rparamt=RPnone;
			rparam=0; }

		if(rxn->prob<0) rxn->prob=1;
		if(!permit) rxn->bindrad2=0;
		else if(rate3<=0) rxn->bindrad2=0;
		else if(dsum<=0) {sprintf(erstr,"Both diffusion coefficients are 0");return 4;}
		else if(rparamt==RPunbindrad) rxn->bindrad2=bindingradius(rate3,sim->dt,dsum,rparam,0);
		else if(rparamt==RPbounce) rxn->bindrad2=bindingradius(rate3,sim->dt,dsum,rparam,0);
		else if(rparamt==RPratio) rxn->bindrad2=bindingradius(rate3,sim->dt,dsum,rparam,1);
		else if(rparamt==RPpgem) rxn->bindrad2=bindingradius(rate3*(1.0-rparam),sim->dt,dsum,-1,0);
		else if(rparamt==RPpgemmax || rparamt==RPpgemmaxw) {
			rxn->bindrad2=bindingradius(rate3,sim->dt,dsum,0,0);
			unbindrad=unbindingradius(rparam,sim->dt,dsum,rxn->bindrad2);
			if(unbindrad>0) rxn->bindrad2=bindingradius(rate3*(1.0-rparam),sim->dt,dsum,-1,0); }
		else rxn->bindrad2=bindingradius(rate3,sim->dt,dsum,-1,0);

		rxn->bindrad2*=rxn->bindrad2; }

	return 0; }


/* rxnsetrates */
int rxnsetrates(simptr sim,int order,char *erstr) {
	rxnssptr rxnss;
	int r,er;

	rxnss=sim->rxnss[order];
	if(!rxnss) return -1;

	for(r=0;r<rxnss->totrxn;r++) {
		er=rxnsetrate(sim,order,r,erstr);
		if(er>1) return r; }		// error code of 1 is just a warning, not an error

	return -1; }


/* rxnsetproduct */
int rxnsetproduct(simptr sim,int order,int r,char *erstr) {
	rxnssptr rxnss;
	rxnptr rxn,rxnr;
	int er,dim,nprod,orderr,rr,rev,d,prd;
	double rpar,dc1,dc2,dsum,bindradr,dist;
	enum RevParam rparamt;
	enum MolecState ms1,ms2;

	rxnss=sim->rxnss[order];
	rxn=rxnss->rxn[r];
	nprod=rxn->nprod;
	rpar=rxn->rparam;
	rparamt=rxn->rparamt;
	er=0;
	dim=sim->dim;

	if(nprod==0) {																		// nprod==0
		if(rparamt==RPnone || rparamt==RPirrev || rparamt==RPconfspread)
			rxn->unbindrad=0;
		else {
			sprintf(erstr,"Illegal product parameter because reaction has no products");
			er=1; }}

	else if(rparamt==RPoffset || rparamt==RPfixed) {	// nprod>0, offset and fixed
		dist=0;
		if(nprod==1)
			for(d=0;d<dim;d++) dist=rxn->prdpos[0][d];
		else
			for(d=0;d<dim;d++) dist+=rxn->prdpos[0][d]-rxn->prdpos[1][d];
		rxn->unbindrad=sqrt(dist); }

	else if(nprod==1) {																// nprod==1, all others (i.e. not offset or fixed)
		if(rparamt==RPnone || rparamt==RPirrev || rparamt==RPconfspread) {
			rxn->unbindrad=0;						// only 1 product so no unbinding radius
			for(d=0;d<dim;d++) rxn->prdpos[0][d]=0; }
		else {
			sprintf(erstr,"Illegal product parameter because reaction only has one product");
			er=2; }}

	else if(rparamt==RPbounce) {										// nprod>=2, bounce
		ms1=rxn->rctstate[0];
		ms2=rxn->rctstate[1];
		if(ms1==MSbsoln || ms1==MSnone || ms1==MSall || ms1==MSsome) ms1=MSsoln;
		if(ms2==MSbsoln || ms2==MSnone || ms2==MSall || ms2==MSsome) ms2=MSsoln;
		dc1=MolCalcDifcSum(sim,rxn->rctident[0],ms1,0,MSnone);
		dc2=MolCalcDifcSum(sim,rxn->rctident[1],ms2,0,MSnone);
		dsum=dc1+dc2;
		if(dsum==0)
			dc1=dc2=0.5;
		else {
			dc1/=dsum;				// dc1 and dc2 are diffusion coefficients relative to dsum
			dc2/=dsum; }
		if(rpar>=0) {
			rxn->unbindrad=rpar;
			rxn->prdpos[0][0]=rpar*dc1;
			rxn->prdpos[1][0]=rpar*dc2;
			for(prd=2;prd<nprod;prd++)
				rxn->prdpos[prd][0]=0; }
		else {
			rxn->unbindrad=-1;
			rxn->prdpos[0][0]=sqrt(rxn->bindrad2)*dc1;
			rxn->prdpos[1][0]=sqrt(rxn->bindrad2)*dc2;
			for(prd=2;prd<nprod;prd++)
				rxn->prdpos[prd][0]=0; }}

	else if(nprod>=2) {																// nprod>=2, not offset or fixed
		ms1=rxn->prdstate[0];
		ms2=rxn->prdstate[1];
		if(ms1==MSbsoln) ms1=MSsoln;
		if(ms2==MSbsoln) ms2=MSsoln;
		dc1=MolCalcDifcSum(sim,rxn->prdident[0],ms1,0,MSnone);
		dc2=MolCalcDifcSum(sim,rxn->prdident[1],ms2,0,MSnone);
		dsum=dc1+dc2;
		if(dsum==0)
			dc1=dc2=0.5;
		else {
			dc1/=dsum;				// dc1 and dc2 are diffusion coefficients relative to dsum
			dc2/=dsum; }
		rev=findreverserxn(sim,order,r,&orderr,&rr);

		if(rparamt==RPirrev || rparamt==RPconfspread) {	// nprod>=2, irrev or confspread
			rxn->unbindrad=0;
			for(prd=0;prd<nprod;prd++)
				for(d=0;d<dim;d++)
					rxn->prdpos[prd][d]=0; }

		else if(rev==0) {																// nprod>=2, irreversible, not offset, fixed, irrev, confspread, or bounce
			if(rparamt==RPpgem || rparamt==RPpgemmax || rparamt==RPpgemmaxw || rparamt==RPratio || rparamt==RPpgem2 || rparamt==RPpgemmax2 || rparamt==RPratio2) {
				sprintf(erstr,"Illegal product parameter because products don't react");er=3; }
			else if(rparamt==RPunbindrad) {								// nprod>=2, irrev., unbindrad
				rxn->unbindrad=rpar;
				rxn->prdpos[0][0]=rpar*dc1;
				rxn->prdpos[1][0]=-rpar*dc2; }
			else {																				// nprod>=2, irreversible, none
				rxn->unbindrad=0;
				rxn->prdpos[0][0]=0;
				rxn->prdpos[1][0]=0; }}

		else {																					// nprod>=2, reversible, not offset, fixed, irrev, confspread, or bounce
			rxnr=sim->rxnss[orderr]->rxn[rr];
			if(rxnr->bindrad2>=0) bindradr=sqrt(rxnr->bindrad2);
			else bindradr=-1;

			if(rparamt==RPnone) {
				sprintf(erstr,"Undefined product placement for reversible reaction");er=5; }
			else if(rparamt==RPunbindrad) {
				rxn->unbindrad=rpar;
				rxn->prdpos[0][0]=rpar*dc1;
				rxn->prdpos[1][0]=-rpar*dc2; }
			else if(rxnr->bindrad2<0) {			// all below options require bindrad2 >= 0
				sprintf(erstr,"Binding radius of reaction products is undefined");er=6; }
      else if(rxnr->bindrad2==0) {    // set to 0 if bindrad2 == 0
        rxn->unbindrad=0;
        rxn->prdpos[0][0]=0;
        rxn->prdpos[1][0]=0; }
			else if(rparamt==RPratio || rparamt==RPratio2) {
				rxn->unbindrad=rpar*bindradr;
				rxn->prdpos[0][0]=rpar*bindradr*dc1;
				rxn->prdpos[1][0]=-rpar*bindradr*dc2; }
			else if(dsum<=0) {						// all below options require dsum > 0
				sprintf(erstr,"Cannot set unbinding distance because sum of product diffusion constants is 0");er=4; }		
			else if(rparamt==RPpgem || rparamt==RPpgem2) {
				rpar=unbindingradius(rpar,sim->dt,dsum,bindradr);
				if(rpar==-2) {
					sprintf(erstr,"Cannot create an unbinding radius due to illegal input values");er=7; }		
				else if(rpar<0) {
					sprintf(erstr,"Maximum possible geminate binding probability is %g",-rpar);er=8; }		
				else {
					rxn->unbindrad=rpar;
					rxn->prdpos[0][0]=rpar*dc1;
					rxn->prdpos[1][0]=-rpar*dc2; }}
			else if(rparamt==RPpgemmax || rparamt==RPpgemmaxw || rparamt==RPpgemmax2) {
				rpar=unbindingradius(rpar,sim->dt,dsum,bindradr);
				if(rpar==-2) {
					sprintf(erstr,"Illegal input values");er=9; }		
				else if(rpar<=0) {
					rxn->unbindrad=0; }
				else if(rpar>0) {
					rxn->unbindrad=rpar;
					rxn->prdpos[0][0]=rpar*dc1;
					rxn->prdpos[1][0]=-rpar*dc2; }}
			else {
				simLog(sim,10,"BUG in rxnsetproduct");er=10; }}}		

	return er; }


/* rxnsetproducts */
int rxnsetproducts(simptr sim,int order,char *erstr) {
	rxnssptr rxnss;
	int r,er;

	rxnss=sim->rxnss[order];
	if(!rxnss) return -1;
	for(r=0;r<rxnss->totrxn;r++) {
		er=rxnsetproduct(sim,order,r,erstr);
		if(er) return r; }
	return -1; }


/* rxncalcrate */
double rxncalcrate(simptr sim,int order,int r,double *pgemptr) {
	rxnssptr rxnss;
	double ans,vol,rpar;
	int i1,i2,i,j,r2,rev,o2,permit;
	double step,a,bval,product,probthisrxn,prob,sum,ratesum;
	rxnptr rxn,rxnr,rxn2;
	enum MolecState ms1,ms2,statelist[MAXORDER];
	enum RevParam rparamt;

	if(!sim) return -1;
	rxnss=sim->rxnss[order];
	if(!rxnss || r<0 || r>=rxnss->totrxn) return -1;
	rxn=rxnss->rxn[r];

	if(order==0) {																	// order 0
		if(rxn->cmpt) vol=rxn->cmpt->volume;
		else if(rxn->srf) vol=surfacearea(rxn->srf,sim->dim,NULL);
		else vol=systemvolume(sim);
		if(rxn->prob<0) ans=0;
		else ans=rxn->prob/sim->dt/vol; }

	else if(order==1) {															// order 1
		ans=0;
		for(ms1=(MolecState)0;ms1<MSMAX && !rxn->permit[ms1];ms1=(MolecState)(ms1+1));
		if(rxn->prob>0 && ms1!=MSMAX) {
			i1=rxn->rctident[0];

			probthisrxn=0;
			product=1.0;
			sum=0;
			for(j=0;j<rxnss->nrxn[i1];j++) {		// increase reaction probability to account for prior reactions in the list
				rxn2=rxnss->rxn[rxnss->table[i1][j]];
				if(rxn2->permit[ms1] && rxn2->prob>0) {
					prob=rxn2->prob*product;				// prob is probability that rxn2 is chosen
					if(rxn2==rxn) probthisrxn=prob;
					sum+=prob;											// sum is the probability that some reaction is chosen
					product*=1.0-prob; }}						// product is the probability that no reaction is chosen

			ratesum=-log(1.0-sum)/sim->dt;
			ans=ratesum*probthisrxn/sum; }}

	else if(order==2) {															// order 2
		if(rxn->bindrad2<0 || rxn->prob<0) ans=0;
		else {
			i1=rxn->rctident[0];
			i2=rxn->rctident[1];
			i=rxnpackident(order,rxnss->maxspecies,rxn->rctident);
			for(j=0;j<rxnss->nrxn[i] && rxnss->table[i][j]!=r;j++);
			if(rxnss->table[i][j]!=r) return -1;
			permit=rxnreactantstate(rxn,statelist,1);
			ms1=statelist[0];
			ms2=statelist[1];
			if(!permit) return 0;
			if(rxn->rparamt==RPconfspread) return -log(1.0-rxn->prob)/sim->dt;
			step=sqrt(2.0*MolCalcDifcSum(sim,i1,ms1,i2,ms2)*sim->dt);
			a=sqrt(rxn->bindrad2);
			rev=findreverserxn(sim,order,r,&o2,&r2);
			if(rev==1) {
				rparamt=sim->rxnss[o2]->rxn[r2]->rparamt;
				rpar=sim->rxnss[o2]->rxn[r2]->rparam; }
			else {
				rparamt=RPnone;
				rpar=0; }
			if(rparamt==RPpgem || (rparamt==RPbounce && rpar>=0) || rparamt==RPpgemmax || rparamt==RPpgemmaxw || rparamt==RPratio || rparamt==RPoffset || rparamt==RPfixed) {
				rxnr=sim->rxnss[o2]->rxn[r2];
				bval=distanceVVD(rxnr->prdpos[0],rxnr->prdpos[1],sim->dim);
				ans=numrxnrate(step,a,bval); }
			else
				ans=numrxnrate(step,a,-1);
			ans/=sim->dt;
			if(i1==i2) ans/=2.0;
			if(!rxn->permit[MSsoln*MSMAX1+MSsoln]) ans/=2.0; }}

	else ans=0;

	if(pgemptr) {
		if(rxn->nprod!=2 || findreverserxn(sim,order,r,&o2,&r2)==0) *pgemptr=-1;
		else {
			step=sqrt(2.0*MolCalcDifcSum(sim,rxn->prdident[0],rxn->prdstate[0],rxn->prdident[1],rxn->prdstate[1])*sim->dt);
			bval=distanceVVD(rxn->prdpos[0],rxn->prdpos[1],sim->dim);
			rxnr=sim->rxnss[o2]->rxn[r2];
			a=sqrt(rxnr->bindrad2);
			*pgemptr=1.0-numrxnrate(step,a,-1)/numrxnrate(step,a,bval); }}

	return ans; }



/* rxncalctau */
void rxncalctau(simptr sim,int order) {
	rxnssptr rxnss;
	rxnptr rxn;
	int r;
	double rate,vol,conc1,conc2;

	rxnss=sim->rxnss[order];
	if(!rxnss) return;

	if(order==1) {
		for(r=0;r<rxnss->totrxn;r++) {
			rxn=rxnss->rxn[r];
			rate=rxncalcrate(sim,1,r,NULL);
			rxn->tau=1.0/rate; }}

	else if(order==2) {
		vol=systemvolume(sim);
		for(r=0;r<rxnss->totrxn;r++) {
			rxn=rxnss->rxn[r];
			conc1=(double)molcount(sim,rxn->rctident[0],NULL,MSall,NULL,-1)/vol;
			conc2=(double)molcount(sim,rxn->rctident[1],NULL,MSall,NULL,-1)/vol;
			rate=rxncalcrate(sim,2,r,NULL);
			if(rxn->rparamt==RPconfspread) rxn->tau=1.0/rate;
			else rxn->tau=(conc1+conc2)/(rate*conc1*conc2); }}

	return; }


/******************************************************************************/
/****************************** structure set up ******************************/
/******************************************************************************/


/* rxnsetcondition */
void rxnsetcondition(simptr sim,int order,enum StructCond cond,int upgrade) {
	int o1,o2;

	if(!sim) return;
	if(order<0) {
		o1=0;
		o2=2; }
	else if(order<=2)
		o1=o2=order;
	else
		return;

	for(order=o1;order<=o2;order++) {
		if(sim->rxnss[order]) {
			if(upgrade==0 && sim->rxnss[order]->condition>cond) sim->rxnss[order]->condition=cond;
			else if(upgrade==1 && sim->rxnss[order]->condition<cond) sim->rxnss[order]->condition=cond;
			else if(upgrade==2) sim->rxnss[order]->condition=cond;
			if(sim->rxnss[order]->condition<sim->condition) {
				cond=sim->rxnss[order]->condition;
				simsetcondition(sim,cond==SCinit?SClists:cond,0); }}}
	
	return; }


/* RxnSetValue */
int RxnSetValue(simptr sim,const char *option,rxnptr rxn,double value) {
	int er;

	er=0;
	if(!rxn || !option) er=1;

	else if(!strcmp(option,"rate")) {
		if(rxn->rate!=-1) er=3;
		if(value<0) er=4;
		rxn->rate=value; }

	else if(!strcmp(option,"confspreadrad")) {
		if(rxn->rparamt==RPconfspread) er=3;
		rxn->rparamt=RPconfspread;
		if(value<0) er=4;
		rxn->bindrad2=value*value; }

	else if(!strcmp(option,"bindrad")) {
		if(rxn->rparamt==RPconfspread) er=3;
		if(value<0) er=4;
		rxn->bindrad2=value*value; }

	else if(!strcmp(option,"prob")) {
		if(value<0) er=4;
		if(rxn->rxnss->order>0 && value>1) er=4;
		rxn->prob=value; }

	else if(!strcmp(option,"disable")) {
		rxn->disable=(int) value; }

	else er=2;
	rxnsetcondition(sim,-1,SClists,0);
	return er; }


/* RxnSetRevparam */
int RxnSetRevparam(simptr sim,rxnptr rxn,enum RevParam rparamt,double rparam,int prd,double *pos,int dim) {
	int d,er;

	er=0;
	if(rxn->rparamt!=RPnone) er=1;
	rxn->rparamt=rparamt;

	if(rparamt==RPnone || rparamt==RPirrev || rparamt==RPconfspread);
	else if(rparamt==RPbounce)
		rxn->rparam=rparam;
	else if(rparamt==RPpgem || rparamt==RPpgemmax || rparamt==RPpgemmaxw || rparamt==RPpgem2 || rparamt==RPpgemmax2) {
		if(!(rparam>0 && rparam<=1)) er=2;
		rxn->rparam=rparam; }
	else if(rparamt==RPratio || rparamt==RPunbindrad || rparamt==RPratio2) {
		if(rparam<0) er=2;
		rxn->rparam=rparam; }
	else if(rparamt==RPoffset || rparamt==RPfixed) {
		er=0;
		if(prd<0 || prd>=rxn->nprod) er=4;
		else if(!pos) er=5;
		else for(d=0;d<dim;d++) rxn->prdpos[prd][d]=pos[d]; }
	else
		er=3;
	rxnsetcondition(sim,-1,SCparams,0);
	return er; }


/* RxnSetPermit */
void RxnSetPermit(simptr sim,rxnptr rxn,int order,enum MolecState *rctstate,int value) {
	enum MolecState ms,nms2o,mslist[MSMAX1];
	int set,ord;
	static int recurse;

	if(order==0) return;
	nms2o=(MolecState)intpower(MSMAX1,order);
	for(ms=(MolecState)0;ms<nms2o;ms=(MolecState)(ms+1)) {
		rxnunpackstate(order,ms,mslist);
		set=1;
		for(ord=0;ord<order && set;ord++)
			if(!(rctstate[ord]==MSall || rctstate[ord]==mslist[ord])) set=0;
		if(set) rxn->permit[ms]=value; }

	if(order==2 && rxn->rctident[0]==rxn->rctident[1] && recurse==0) {
		recurse=1;
		mslist[0]=rctstate[1];
		mslist[1]=rctstate[0];
		RxnSetPermit(sim,rxn,order,mslist,value);
		recurse=0; }

	rxnsetcondition(sim,-1,SCparams,0);
	surfsetcondition(sim->srfss,SClists,0);
	return; }


/* RxnSetCmpt */
void RxnSetCmpt(rxnptr rxn,compartptr cmpt) {
	rxn->cmpt=cmpt;
	return; }


/* RxnSetSurface */
void RxnSetSurface(rxnptr rxn,surfaceptr srf) {
	rxn->srf=srf;
	return; }


/* RxnSetPrdSerno */
int RxnSetPrdSerno(rxnptr rxn,long int *prdserno) {
	int prd;

	if(!rxn->prdserno) {
		rxn->prdserno=(long int*) calloc(rxn->nprod,sizeof(long int));
		if(!rxn->prdserno) return 1;
		for(prd=0;prd<rxn->nprod;prd++)
			rxn->prdserno[prd]=0; }

	for(prd=0;prd<rxn->nprod;prd++)
		rxn->prdserno[prd]=prdserno[prd];

	return 0; }


/* RxnSetIntersurfaceRules */
int RxnSetIntersurfaceRules(rxnptr rxn,int *rules) {
	int prd;

	if(!rxn->prdintersurf) {
		rxn->prdintersurf=(int *) calloc(rxn->nprod>0?rxn->nprod:1,sizeof(int));
		if(!rxn->prdintersurf) return 1;
		for(prd=0;prd<rxn->nprod;prd++)
			rxn->prdintersurf[prd]=0; }

	if(rules[0]==-1) {
		free(rxn->prdintersurf);
		rxn->prdintersurf=NULL; }

	else if(rxn->nprod==0)
		rxn->prdintersurf[0]=0;

	else
		for(prd=0;prd<rxn->nprod;prd++)
			rxn->prdintersurf[prd]=rules[prd];

	return 0; }


/* RxnSetLog */
int RxnSetLog(simptr sim,char *filename,rxnptr rxn,listptrli list,int turnon) {
	int order,r;
	rxnssptr rxnss;

	if(!rxn) {												// all reactions
		for(order=0;order<=2;order++) {
			rxnss=sim->rxnss[order];
			if(rxnss)
				for(r=0;r<rxnss->totrxn;r++) {
					rxn=rxnss->rxn[r];
					RxnSetLog(sim,filename,rxn,list,turnon); }}}

	if(turnon) {												// turn on logging
		if(!list) {
			CHECKMEM(rxn->logserno=List_AllocLI(0)); }
		else {
			if(!rxn->logserno) rxn->logserno=list;
			else {
				CHECKMEM(List_AppendToLI(rxn->logserno,list)==0);
				List_FreeLI(list); }}

		if(!rxn->logfile) {
			CHECKMEM(rxn->logfile=EmptyString()); }
		strcpy(rxn->logfile,filename); }

	else {															// turn off logging
		if(!list) {
			List_FreeLI(rxn->logserno);
			rxn->logserno=NULL; }
		else {
			List_RemoveFromLI(rxn->logserno,list);
			if(rxn->logserno->n==0) {
				List_FreeLI(rxn->logserno);
				rxn->logserno=NULL; }
			List_FreeLI(list); }}

	return 0;

 failure:
	return 1; }


/* RxnAddReaction */
rxnptr RxnAddReaction(simptr sim,const char *rname,int order,int *rctident,enum MolecState *rctstate,int nprod,int *prdident,enum MolecState *prdstate,compartptr cmpt,surfaceptr srf) {
	char **newrname;
	rxnptr *newrxn;
	int *newtable,identlist[MAXORDER];
	rxnssptr rxnss;
	rxnptr rxn;
	int maxrxn,maxspecies,i,j,r,rct,prd,d,k,done,freerxn;

	rxnss=NULL;
	rxn=NULL;
	newrname=NULL;
	newrxn=NULL;
	newtable=NULL;
	maxrxn=0;
	freerxn=1;

	if(!sim->rxnss[order]) {													// allocate reaction superstructure, if needed
		CHECKS(sim->mols,"Cannot add reaction because no molecules defined");
		CHECKMEM(sim->rxnss[order]=rxnssalloc(NULL,order,sim->mols->maxspecies));
		sim->rxnss[order]->sim=sim;
		rxnsetcondition(sim,order,SCinit,0);
		rxnsetcondition(sim,-1,SClists,0); }
	rxnss=sim->rxnss[order];
	maxspecies=rxnss->maxspecies;
	r=stringfind(rxnss->rname,rxnss->totrxn,rname);		// r is reaction index

	if(r>=0) {
		CHECKBUG(rxnss->rxn[r]->nprod==0,"RxnAddReaction cannot be called for a reaction that already has products");
		rxn=rxnss->rxn[r]; }
	else {
		if(rxnss->totrxn==rxnss->maxrxn) {							// make more reaction space in superstructure, if needed
			maxrxn=(rxnss->maxrxn>0)?2*rxnss->maxrxn:2;
			CHECKMEM(newrname=(char**)calloc(maxrxn,sizeof(char*)));
			for(r=0;r<rxnss->maxrxn;r++) newrname[r]=rxnss->rname[r];
			for(r=rxnss->maxrxn;r<maxrxn;r++) newrname[r]=NULL;
			for(r=rxnss->maxrxn;r<maxrxn;r++) CHECK(newrname[r]=EmptyString());
			CHECKMEM(newrxn=(rxnptr*)calloc(maxrxn,sizeof(rxnptr)));
			for(r=0;r<rxnss->maxrxn;r++) newrxn[r]=rxnss->rxn[r];
			for(r=rxnss->maxrxn;r<maxrxn;r++) newrxn[r]=NULL;
			rxnss->maxrxn=maxrxn;
			free(rxnss->rname);
			rxnss->rname=newrname;
			newrname=NULL;
			free(rxnss->rxn);
			rxnss->rxn=newrxn;
			newrxn=NULL; }

		CHECKMEM(rxn=rxnalloc(order));									// create reaction and set up reactants
		rxn->rxnss=rxnss;
		rxn->rname=rxnss->rname[rxnss->totrxn];
		if(order>0) {
			for(rct=0;rct<order;rct++) rxn->rctident[rct]=rctident[rct];
			for(rct=0;rct<order;rct++) rxn->rctstate[rct]=rctstate[rct];
			RxnSetPermit(sim,rxn,order,rctstate,1); }

		if(order>0) {																		// set up nrxn and table; table is automatically enlarged
			k=0;
			done=0;
			while(!done) {
				k=Zn_permute(rctident,identlist,order,k);
				CHECKBUG(k!=-1,"SMOLDYN BUG: Zn_permute.\n");
				if(k==0) done=1;
				i=rxnpackident(order,maxspecies,identlist);
				CHECKMEM(newtable=(int*)calloc(rxnss->nrxn[i]+1,sizeof(int)));
				for(j=0;j<rxnss->nrxn[i];j++) newtable[j]=rxnss->table[i][j];
				newtable[j]=rxnss->totrxn;
				free(rxnss->table[i]);
				rxnss->table[i]=newtable;
				newtable=NULL;
				rxnss->nrxn[i]++; }}

		strncpy(rxnss->rname[rxnss->totrxn],rname,STRCHAR-1);		// plug in reaction
		rxnss->rname[rxnss->totrxn][STRCHAR-1]='\0';
		rxnss->totrxn++;
		rxnss->rxn[rxnss->totrxn-1]=rxn; }
	freerxn=0;

	rxn->nprod=nprod;																					// set up products
	if(nprod) {
		CHECKMEM(rxn->prdident=(int*)calloc(nprod,sizeof(int)));
		for(prd=0;prd<nprod;prd++) rxn->prdident[prd]=prdident[prd];
		CHECKMEM(rxn->prdstate=(enum MolecState*)calloc(nprod,sizeof(enum MolecState)));
		for(prd=0;prd<nprod;prd++) rxn->prdstate[prd]=prdstate[prd];
		CHECKMEM(rxn->prdpos=(double**)calloc(nprod,sizeof(double*)));
		for(prd=0;prd<nprod;prd++) rxn->prdpos[prd]=NULL;
		for(prd=0;prd<nprod;prd++) {
			CHECKMEM(rxn->prdpos[prd]=(double*)calloc(sim->dim,sizeof(double)));
			for(d=0;d<sim->dim;d++) rxn->prdpos[prd][d]=0; }}

	RxnSetCmpt(rxn,cmpt);																						// add reaction compartment
	RxnSetSurface(rxn,srf);																							// add reaction surface
	rxnsetcondition(sim,-1,SClists,0);
	surfsetcondition(sim->srfss,SClists,0);
	return rxn;

 failure:
	if(!rxnss) return NULL;
	if(newrname) {
		for(r=rxnss->maxrxn;r<maxrxn;r++) free(newrname[r]);
		free(newrname); }
	free(newrxn);
	if(freerxn) rxnfree(rxn);
	if(ErrorType==2) simLog(sim,8,"%s",ErrorString);
	else simLog(sim,10,"%s",ErrorString);
	return NULL; }


/* RxnAddReactionCheck */
rxnptr RxnAddReactionCheck(simptr sim,char *rname,int order,int *rctident,enum MolecState *rctstate,int nprod,int *prdident,enum MolecState *prdstate,compartptr cmpt,surfaceptr srf) {
	rxnptr rxn;
	int i;

	CHECKBUG(sim,"sim undefined");
	CHECKBUG(sim->mols,"sim is missing molecule superstructure");
	CHECKBUG(rname,"rname is missing");
	CHECKBUG(strlen(rname)<STRCHAR,"rname is too long");
	CHECKBUG(order>=0 && order<=2,"order is out of bounds");
	if(order>0) {
		CHECKBUG(rctident,"rctident is missing"); }
	for(i=0;i<order;i++) {
		CHECKBUG(rctident[i]>0 && rctident[i]<sim->mols->nspecies,"reactant identity out of bounds");
		CHECKBUG(rctstate[i]>=0 && rctstate[i]<MSMAX1,"reactant state out of bounds"); }
	CHECKBUG(nprod>=0,"nprod out of bounds");
	for(i=0;i<nprod;i++) {
		CHECKBUG(prdident[i]>0 && prdident[i]<sim->mols->nspecies,"reactant identity out of bounds");
		CHECKBUG(prdstate[i]>=0 && prdstate[i]<MSMAX1,"reactant state out of bounds"); }
	if(cmpt) {
		CHECKBUG(sim->cmptss,"sim is missing compartment superstructure"); }
	if(srf)	{
		CHECKBUG(sim->srfss,"sim is missing surface superstructure"); }
	rxn=RxnAddReaction(sim,rname,order,rctident,rctstate,nprod,prdident,prdstate,cmpt,srf);
	return rxn;
failure:
	simLog(sim,10,"%s",ErrorString);
	return NULL; }


/* rxnreadstring */
rxnssptr rxnreadstring(simptr sim,ParseFilePtr pfp,rxnssptr rxnss,char *word,char *line2) {
	int order,maxspecies,itct;
	char nm[STRCHAR],nm2[STRCHAR],rxnnm[STRCHAR];
	int i,r,prd,j,i1,i2,i3,nptemp,identlist[MAXPRODUCT],d;
	double rtemp,postemp[DIMMAX];
	enum MolecState ms,ms1,ms2,mslist[MAXPRODUCT];
	rxnptr rxn;
	enum RevParam rparamt;
	
	order=-1;
	maxspecies=sim->mols->maxspecies;
	
	if(!strcmp(word,"order")) {							// order
		itct=sscanf(line2,"%i",&order);
		CHECKS(itct==1,"error reading order");
		CHECKS(order>=0 && order<=2,"order needs to be between 0 and 2");
		if(!sim->rxnss[order]) {
			CHECKS(sim->rxnss[order]=rxnssalloc(NULL,order,maxspecies),"out of memory creating reaction superstructure");
			rxnsetcondition(sim,order,SCinit,0);
			sim->rxnss[order]->sim=sim; }
		rxnss=sim->rxnss[order];
		CHECKS(!strnword(line2,2),"unexpected text following order"); }
	
	else if(!strcmp(word,"max_rxn")) {						// max_rxn
		}
	
	else if(!strcmp(word,"reactant") && order==0) {	// reactant, 0
		CHECKS(order>=0,"order needs to be entered before reactant");
		j=wordcount(line2);
		CHECKS(j>0,"number of reactions needs to be >0");
		for(j--;j>=0;j--) {
			itct=sscanf(line2,"%s",nm);
			CHECKS(itct==1,"missing reaction name in reactant");
			CHECKS(stringfind(rxnss->rname,rxnss->totrxn,nm)<0,"reaction name has already been used");
			CHECKS(RxnAddReaction(sim,nm,0,NULL,NULL,0,NULL,NULL,NULL,NULL),"faied to add 0th order reaction");
			line2=strnword(line2,2); }}
	
	else if(!strcmp(word,"reactant") && order==1) {	// reactant, 1
		CHECKS(order>=0,"order needs to be entered before reactant");
		i=readmolname(sim,line2,&ms,0);
		CHECKS(i!=0,"empty molecules cannot react");
		CHECKS(i!=-1,"reactant format: name[(state)] rxn_name");
		CHECKS(i!=-2,"mismatched or improper parentheses around molecule state");
		CHECKS(i!=-3,"cannot read molecule state value");
		CHECKS(i!=-4,"molecule name not recognized");
		CHECKS(i!=-5,"molecule name cannot be set to 'all'");
		CHECKS(i!=-6,"molecule name cannot include wildcards");
		CHECKS(ms!=MSbsoln,"bsoln is not an allowed state for first order reactants");
		identlist[0]=i;
		mslist[0]=ms;
		CHECKS(line2=strnword(line2,2),"no reactions listed");
		j=wordcount(line2);
		for(j--;j>=0;j--) {
			itct=sscanf(line2,"%s",nm);
			CHECKS(itct==1,"missing reaction name in reactant");
			CHECKS(stringfind(rxnss->rname,rxnss->totrxn,nm)<0,"reaction name has already been used");
			CHECKS(RxnAddReaction(sim,nm,1,identlist,mslist,0,NULL,NULL,NULL,NULL),"faied to add 1st order reaction");
			line2=strnword(line2,2); }}
	
	else if(!strcmp(word,"reactant") && order==2) {	// reactant, 2
		CHECKS(order>=0,"order needs to be entered before reactants");
		i1=readmolname(sim,line2,&ms1,0);
		CHECKS(i1!=0,"empty molecules cannot react");
		CHECKS(i1!=-1,"reactant format: name[(state)] + name[(state)] rxn_name");
		CHECKS(i1!=-2,"mismatched or improper parentheses around molecule state");
		CHECKS(i1!=-3,"cannot read molecule state value");
		CHECKS(i1!=-4,"molecule name not recognized");
		CHECKS(i1!=-5,"molecule name cannot be set to 'all'");
		CHECKS(i1!=-6,"molecule name cannot include wildcards");
		identlist[0]=i1;
		mslist[0]=ms1;
		CHECKS(line2=strnword(line2,3),"reactant format: name[(state)] + name[(state)] rxn_list");
		i2=readmolname(sim,line2,&ms2,0);
		CHECKS(i2!=0,"empty molecules cannot react");
		CHECKS(i2!=-1,"reactant format: name[(state)] + name[(state)] rxn_name value");
		CHECKS(i2!=-2,"mismatched or improper parentheses around molecule state");
		CHECKS(i2!=-3,"cannot read molecule state value");
		CHECKS(i2!=-4,"molecule name not recognized");
		CHECKS(i2!=-5,"molecule name cannot be set to 'all'");
		CHECKS(i2!=-6,"molecule name cannot include wildcards");
		identlist[1]=i2;
		mslist[1]=ms2;
		CHECKS(line2=strnword(line2,2),"no reactions listed");
		j=wordcount(line2);
		for(j--;j>=0;j--) {
			itct=sscanf(line2,"%s",nm);
			CHECKS(itct==1,"missing reaction name in reactant");
			CHECKS(stringfind(rxnss->rname,rxnss->totrxn,nm)<0,"reaction name has already been used");
			CHECKS(RxnAddReaction(sim,nm,2,identlist,mslist,0,NULL,NULL,NULL,NULL),"faied to add 1st order reaction");
			line2=strnword(line2,2); }}
	
	else if(!strcmp(word,"permit") && order==0) {		// permit, 0
		CHECKS(0,"reaction permissions are not allowed for order 0 reactions"); }
	
	else if(!strcmp(word,"permit") && order==1) {		// permit, 1
		CHECKS(order>=0,"order needs to be entered before permit");
		i=readmolname(sim,line2,&ms,0);
		CHECKS(i!=0,"empty molecules cannot be entered");
		CHECKS(i!=-1,"permit format: name(state) rxn_name value");
		CHECKS(i!=-2,"mismatched or improper parentheses around molecule state");
		CHECKS(i!=-3,"cannot read molecule state value");
		CHECKS(i!=-4,"molecule name not recognized");
		CHECKS(i!=-5,"molecule name cannot be set to 'all'");
		CHECKS(i!=-6,"molecule name cannot include wildcards");
		CHECKS(ms<MSMAX,"all and bsoln are not allowed in permit for first order reactions");
		CHECKS(line2=strnword(line2,2),"permit format: name(state) rxn_name value");
		itct=sscanf(line2,"%s %i",rxnnm,&i3);
		CHECKS(itct==2,"permit format: name(state) rxn_name value");
		r=stringfind(rxnss->rname,rxnss->totrxn,rxnnm);
		CHECKS(r>=0,"in permit, reaction name not recognized");
		for(j=0;j<rxnss->nrxn[i] && rxnss->table[i][j]!=r;j++);
		CHECKS(rxnss->table[i][j]==r,"in permit, reaction was not already listed for this reactant");
		CHECKS(i3==0 || i3==1,"in permit, value needs to be 0 or 1");
		rxnss->rxn[r]->permit[ms]=i3;
		CHECKS(!strnword(line2,3),"unexpected text following permit"); }
	
	else if(!strcmp(word,"permit") && order==2) {		// permit, 2
		CHECKS(order>=0,"order needs to be entered before permit");
		i1=readmolname(sim,line2,&ms,0);
		CHECKS(i1!=0,"empty molecules not allowed");
		CHECKS(i1!=-1,"permit format: name(state) + name(state) rxn_name value");
		CHECKS(i1!=-2,"mismatched or improper parentheses around first molecule state");
		CHECKS(i1!=-3,"cannot read first molecule state value");
		CHECKS(i1!=-4,"first molecule name not recognized");
		CHECKS(i1!=-5,"first molecule state missing, or is set to 'all'");
		CHECKS(i1!=-6,"molecule name cannot include wildcards");
		CHECKS(ms<MSMAX1,"all is not allowed in permit");
		CHECKS(line2=strnword(line2,3),"permit format: name(state) + name(state) rxn_name value");
		i2=readmolname(sim,line2,&ms2,0);
		CHECKS(i2!=0,"empty molecules are not allowed");
		CHECKS(i2!=-1,"permit format: name(state) + name(state) rxn_name value");
		CHECKS(i2!=-2,"mismatched or improper parentheses around second molecule state");
		CHECKS(i2!=-3,"cannot read second molecule state value");
		CHECKS(i2!=-4,"second molecule name not recognized");
		CHECKS(i2!=-5,"second molecule state missing, or is set to 'all'");
		CHECKS(i2!=-6,"molecule name cannot include wildcards");
		CHECKS(ms2<MSMAX1,"all is not allowed in permit");
		CHECKS(line2=strnword(line2,2),"permit format: name(state) + name(state) rxn_name value");
		i=i1*maxspecies+i2;
		itct=sscanf(line2,"%s %i",rxnnm,&i3);
		CHECKS(itct==2,"permit format: name(state) + name(state) rxn_name value");
		r=stringfind(rxnss->rname,rxnss->totrxn,rxnnm);
		CHECKS(r>=0,"in permit, reaction name not recognized");
		for(j=0;j<rxnss->nrxn[i] && rxnss->table[i][j]!=r;j++);
		CHECKS(rxnss->table[i][j]==r,"in permit, reaction was not already listed for this reactant");
		CHECKS(i3==0 || i3==1,"in permit, value needs to be 0 or 1");
		rxnss->rxn[r]->permit[ms*MSMAX1+ms2]=i3;
		CHECKS(!strnword(line2,3),"unexpected text following permit"); }
	
	else if(!strcmp(word,"rate")) {								// rate
		CHECKS(order>=0,"order needs to be entered before rate");
		itct=sscanf(line2,"%s %lg",nm,&rtemp);
		CHECKS(itct==2,"format for rate: rxn_name rate");
		r=stringfind(rxnss->rname,rxnss->totrxn,nm);
		CHECKS(r>=0,"unknown reaction name in rate");
		CHECKS(rtemp>=0,"reaction rate needs to be >=0 (maybe try rate_internal)");
		rxnss->rxn[r]->rate=rtemp;
		CHECKS(!strnword(line2,3),"unexpected text following rate"); }
	
	else if(!strcmp(word,"confspread_radius")) {	// confspread_radius
		CHECKS(order>=0,"order needs to be entered before confspread_radius");
		itct=sscanf(line2,"%s %lg",nm,&rtemp);
		CHECKS(itct==2,"format for confspread_radius: rxn_name radius");
		r=stringfind(rxnss->rname,rxnss->totrxn,nm);
		CHECKS(r>=0,"unknown reaction name in confspread_radius");
		CHECKS(rxnss->rxn[r]->rparamt!=RPconfspread,"confspread_radius can only be entered once for a reaction");
		CHECKS(rtemp>=0,"confspread_radius needs to be >=0");
		rxnss->rxn[r]->bindrad2=rtemp*rtemp;
		rxnss->rxn[r]->rparamt=RPconfspread;
		CHECKS(!strnword(line2,3),"unexpected text following confspread_radius"); }
	
	else if(!strcmp(word,"rate_internal")) {			// rate_internal
		CHECKS(order>=0,"order needs to be entered before rate_internal");
		itct=sscanf(line2,"%s %lg",nm,&rtemp);
		CHECKS(itct==2,"format for rate_internal: rxn_name rate");
		r=stringfind(rxnss->rname,rxnss->totrxn,nm);
		CHECKS(r>=0,"unknown reaction name in rate_internal");
		CHECKS(rtemp>=0,"rate_internal needs to be >=0");
		if(order<2) rxnss->rxn[r]->prob=rtemp;
		else rxnss->rxn[r]->bindrad2=rtemp*rtemp;
		CHECKS(!strnword(line2,3),"unexpected text following rate_internal"); }
	
	else if(!strcmp(word,"probability")) {			// probability
		CHECKS(order>=0,"order needs to be entered before probability");
		itct=sscanf(line2,"%s %lg",nm,&rtemp);
		CHECKS(itct==2,"format for probability: rxn_name probability");
		r=stringfind(rxnss->rname,rxnss->totrxn,nm);
		CHECKS(r>=0,"unknown reaction name in probability");
		CHECKS(rtemp>=0,"probability needs to be >=0");
		CHECKS(rtemp<=1,"probability needs to be <=1");
		rxnss->rxn[r]->prob=rtemp;
		CHECKS(!strnword(line2,3),"unexpected text following probability"); }
	
	else if(!strcmp(word,"product")) {						// product
		CHECKS(order>=0,"order needs to be entered before product");
		itct=sscanf(line2,"%s",rxnnm);
		CHECKS(itct==1,"format for product: rxn_name product_list");
		r=stringfind(rxnss->rname,rxnss->totrxn,rxnnm);
		CHECKS(r>=0,"unknown reaction name in product");
		nptemp=symbolcount(line2,'+')+1;
		CHECKS(nptemp>=0,"number of products needs to be >=0");
		CHECKS(nptemp<=MAXPRODUCT,"more products are entered than Smoldyn can handle");
		CHECKS(rxnss->rxn[r]->nprod==0,"products for a reaction can only be entered once");
		for(prd=0;prd<nptemp;prd++) {
			CHECKS(line2=strnword(line2,2),"product list is incomplete");
			i=readmolname(sim,line2,&ms,0);
			CHECKS(i!=0,"empty molecules cannot be products");
			CHECKS(i!=-1,"product format: rxn_name name(state) + name(state) + ...");
			CHECKS(i!=-2,"mismatched or improper parentheses around molecule state");
			CHECKS(i!=-3,"cannot read molecule state value");
			CHECKS(i!=-4,"molecule name not recognized");
			CHECKS(i!=-5,"molecule name cannot be 'all'");
			CHECKS(i!=-6,"molecule name cannot include wildcards");
			CHECKS(ms<MSMAX1,"product state is not allowed");
			identlist[prd]=i;
			mslist[prd]=ms;
			if(prd+1<nptemp) {
				CHECKS(line2=strnword(line2,2),"incomplete product list"); }}
		CHECKS(RxnAddReaction(sim,rxnnm,order,NULL,NULL,nptemp,identlist,mslist,NULL,NULL),"failed to add products to reaction");
		CHECKS(!strnword(line2,2),"unexpected text following product"); }
	
	else if(!strcmp(word,"product_param")) {				// product_param
		CHECKS(order>=0,"order needs to be entered before product_param");
		itct=sscanf(line2,"%s",nm);
		CHECKS(itct==1,"format for product_param: rxn type [parameters]");
		r=stringfind(rxnss->rname,rxnss->totrxn,nm);
		CHECKS(r>=0,"unknown reaction name in product_param");
		rxn=rxnss->rxn[r];
		rparamt=rxn->rparamt;
		CHECKS(rparamt==RPnone,"product_param can only be entered once");
		CHECKS(line2=strnword(line2,2),"format for product_param: rxn type [parameters]");
		itct=sscanf(line2,"%s",nm);
		CHECKS(itct==1,"missing parameter type in product_param");
		rparamt=rxnstring2rp(nm);
		CHECKS(rparamt!=RPnone,"unrecognized parameter type");
		rtemp=0;
		prd=0;
		for(d=0;d<sim->dim;d++) postemp[prd]=0;
		if(rparamt==RPpgem || rparamt==RPpgemmax || rparamt==RPratio || rparamt==RPunbindrad || rparamt==RPpgem2 || rparamt==RPpgemmax2 || rparamt==RPratio2) {
			CHECKS(line2=strnword(line2,2),"missing parameter in product_param");
			itct=sscanf(line2,"%lg",&rtemp);
			CHECKS(itct==1,"error reading parameter in product_param"); }
		else if(rparamt==RPoffset || rparamt==RPfixed) {
			CHECKS(line2=strnword(line2,2),"missing parameters in product_param");
			itct=sscanf(line2,"%s",nm2);
			CHECKS(itct==1,"format for product_param: rxn type [parameters]");
				CHECKS((i=stringfind(sim->mols->spname,sim->mols->nspecies,nm2))>=0,"unknown molecule in product_param");
				for(prd=0;prd<rxn->nprod && rxn->prdident[prd]!=i;prd++);
				CHECKS(prd<rxn->nprod,"molecule in product_param is not a product of this reaction");
				CHECKS(line2=strnword(line2,2),"position vector missing for product_param");
				itct=strreadnd(line2,sim->dim,postemp,NULL);
				CHECKS(itct==sim->dim,"insufficient data for position vector for product_param");
				line2=strnword(line2,sim->dim); }
			i1=RxnSetRevparam(sim,rxn,rparamt,rtemp,prd,postemp,sim->dim);
			CHECKS(i1!=1,"reversible parameter type can only be set once");
		CHECKS(i1!=2,"reversible parameter value is out of bounds");
		CHECKS(!strnword(line2,2),"unexpected text following product_param"); }
	
	else {																				// unknown word
		CHECKS(0,"syntax error within reaction block: statement not recognized"); }

	return rxnss;
	
 failure:
	simParseError(sim,pfp);
	return NULL; }


/* loadrxn */
int loadrxn(simptr sim,ParseFilePtr *pfpptr,char *line2) {
	ParseFilePtr pfp;
	char word[STRCHAR],errstring[STRCHAR];
	int done,pfpcode,firstline2;
	rxnssptr rxnss;

	pfp=*pfpptr;
	done=0;
	rxnss=NULL;
	firstline2=line2?1:0;

	while(!done) {
		if(pfp->lctr==0)
			simLog(sim,2," Reading file: '%s'\n",pfp->fname);
		if(firstline2) {
			strcpy(word,"order");
			pfpcode=1;
			firstline2=0; }
		else
			pfpcode=Parse_ReadLine(&pfp,word,&line2,errstring);
		*pfpptr=pfp;
		CHECKS(pfpcode!=3,"%s",errstring);

		if(pfpcode==0);																// already taken care of
		else if(pfpcode==2) {													// end reading
			done=1; }
		else if(!strcmp(word,"end_reaction")) {				// end_reaction
			CHECKS(!line2,"unexpected text following end_reaction");
			return 0; }
		else if(!line2) {															// just word
			CHECKS(0,"unknown word or missing parameter"); }
		else {
			rxnss=rxnreadstring(sim,pfp,rxnss,word,line2);
			CHECK(rxnss); }}

	CHECKS(0,"end of file encountered before end_reaction statement");	// end of file

 failure:																					// failure
	if(ErrorType!=1) simParseError(sim,pfp);
	*pfpptr=pfp=NULL;
	return 1; }


/* rxnsupdateparams */
int rxnsupdateparams(simptr sim) {
	int er,order,wflag;
	char errorstr[STRCHAR];
	
	wflag=strchr(sim->flags,'w')?1:0;
	for(order=0;order<MAXORDER;order++)
		if(sim->rxnss[order] && sim->rxnss[order]->condition<=SCparams) {
			er=rxnsetrates(sim,order,errorstr);							// set rates
			if(er>=0) {
				simLog(sim,8,"Error setting rate for reaction order %i, reaction %s\n%s\n",order,sim->rxnss[order]->rname[er],errorstr);
				return 3; }}
	
	for(order=0;order<MAXORDER;order++)
		if(sim->rxnss[order] && sim->rxnss[order]->condition<=SCparams) {
			errorstr[0]='\0';
			er=rxnsetproducts(sim,order,errorstr);						// set products
			if(er>=0) {
				simLog(sim,8,"Error setting products for reaction order %i, reaction %s\n%s\n",order,sim->rxnss[order]->rname[er],errorstr);
				return 3; }
			if(!wflag && strlen(errorstr)) simLog(sim,5,"%s\n",errorstr); }

	for(order=0;order<MAXORDER;order++)									// calculate tau values
		if(sim->rxnss[order] && sim->rxnss[order]->condition<=SCparams)
			rxncalctau(sim,order);
	
	return 0; }


/* rxnsupdatelists */
int rxnsupdatelists(simptr sim,int order) {
	rxnssptr rxnss;
	int maxlist,ll,nl2o,r,i1,i2,ll1,ll2;
	rxnptr rxn;
	enum MolecState ms1,ms2;

	rxnss=sim->rxnss[order];

	if(order==0) return 0;

	if(!sim->mols || sim->mols->condition<SCparams) return 2;

	maxlist=rxnss->maxlist;								// set reaction molecule lists
	if(maxlist!=sim->mols->maxlist) {
		free(rxnss->rxnmollist);
		rxnss->rxnmollist=NULL;
		maxlist=sim->mols->maxlist;
		if(maxlist>0) {
			nl2o=intpower(maxlist,order);
			rxnss->rxnmollist=(int*) calloc(nl2o,sizeof(int));
			CHECKMEM(rxnss->rxnmollist); }
		rxnss->maxlist=maxlist; }

	if(maxlist>0) {
		nl2o=intpower(maxlist,order);
		for(ll=0;ll<nl2o;ll++) rxnss->rxnmollist[ll]=0;

		for(r=0;r<rxnss->totrxn;r++) {
			rxn=rxnss->rxn[r];
			i1=rxn->rctident[0];
			if(order==1) {
				for(ms1=(MolecState)0;ms1<MSMAX1;ms1=(MolecState)(ms1+1)) {
#ifdef OPTION_VCELL
					if(rxn->permit[ms1] && (rxn->prob>0 || rxn->rate>0 || rxn->rateValueProvider != NULL))
#else
					if(rxn->permit[ms1] && (rxn->prob>0 || rxn->rate>0))
#endif
						{
							ll1=sim->mols->listlookup[i1][ms1];
							rxnss->rxnmollist[ll1]=1; }}}

			else if(order==2) {
				i2=rxn->rctident[1];
				for(ms1=(MolecState)0;ms1<MSMAX1;ms1=(MolecState)(ms1+1))
					for(ms2=(MolecState)0;ms2<MSMAX1;ms2=(MolecState)(ms2+1)) {

#ifdef OPTION_VCELL
						if(rxn->permit[ms1*MSMAX1+ms2] && rxn->prob!=0 && (rxn->rate>0 || rxn->bindrad2>0 || rxn->rateValueProvider != NULL))
#else
						if(rxn->permit[ms1*MSMAX1+ms2] && rxn->prob!=0 && (rxn->rate>0 || rxn->bindrad2>0))
#endif
							{
								ll1=sim->mols->listlookup[i1][ms1==MSbsoln?MSsoln:ms1];
								ll2=sim->mols->listlookup[i2][ms2==MSbsoln?MSsoln:ms2];
								rxnss->rxnmollist[ll1*maxlist+ll2]=1;
								rxnss->rxnmollist[ll2*maxlist+ll1]=1; }}}}}

	return 0;
failure:
	simLog(sim,10,"Unable to allocate memory in rxnsupdatelists");
	return 1; }


/* rxnsupdate */
int rxnsupdate(simptr sim) {
	int er,order,doparams;

	for(order=0;order<MAXORDER;order++) {
		if(sim->rxnss[order] && sim->rxnss[order]->condition<=SClists) {
			er=rxnsupdatelists(sim,order);
			if(er) return er;
			rxnsetcondition(sim,order,SCparams,1); }}

	doparams=0;
	for(order=0;order<MAXORDER;order++)
		if(sim->rxnss[order] && sim->rxnss[order]->condition<SCok) doparams=1;
	if(doparams) {
		er=rxnsupdateparams(sim);
		if(er) return er;
		rxnsetcondition(sim,-1,SCok,1); }

	return 0; }



/******************************************************************************/
/************************** core simulation functions *************************/
/******************************************************************************/


/* doreact */
int doreact(simptr sim,rxnptr rxn,moleculeptr mptr1,moleculeptr mptr2,int ll1,int m1,int ll2,int m2,double *pos,panelptr rxnpnl) {
	int order,prd,d,nprod,dim,calc,dorxnlog,prd2;
	long int serno,sernolist[MAXPRODUCT];
	double dc1,dc2,x,dist;
	molssptr mols;
	moleculeptr mptr,mptrfrom;
	boxptr rxnbptr;
	double v1[DIMMAX],rxnpos[DIMMAX],m3[DIMMAX*DIMMAX];
	enum MolecState ms;
	FILE *fptr;

	mols=sim->mols;
	dim=sim->dim;
	order=rxn->rxnss->order;
	dorxnlog=0;
	fptr=NULL;

// get reaction position in rxnpos, rxnpnl, rxnbptr
	if(order==0) {																// order 0
		for(d=0;d<dim;d++) rxnpos[d]=pos[d];
		rxnbptr=pos2box(sim,rxnpos); }

	else if(order==1) {														// order 1
		for(d=0;d<dim;d++) rxnpos[d]=mptr1->pos[d];
		rxnpnl=mptr1->pnl;
		rxnbptr=mptr1->box; }

	else {																				// order 2
		dc1=mols->difc[mptr1->ident][mptr1->mstate];
		dc2=mols->difc[mptr2->ident][mptr2->mstate];
		if(dc1==0 && dc2==0) x=0.5;
		else x=dc2/(dc1+dc2);
		for(d=0;d<dim;d++) rxnpos[d]=x*mptr1->pos[d]+(1.0-x)*mptr2->pos[d];
		if(mptr1->pnl || mptr2->pnl) {
			if(mptr1->pnl && ptinpanel(rxnpos,mptr1->pnl,dim)) rxnpnl=mptr1->pnl;
			else if(mptr2->pnl && ptinpanel(rxnpos,mptr2->pnl,dim)) rxnpnl=mptr2->pnl;
			else if(mptr1->pnl)
				closestsurfacept(mptr1->pnl->srf,dim,rxnpos,NULL,&rxnpnl,mptr1->box);
			else
				closestsurfacept(mptr2->pnl->srf,dim,rxnpos,NULL,&rxnpnl,mptr2->box); }
		else rxnpnl=NULL;
		rxnbptr=(dc1<=dc2)?mptr1->box:mptr2->box; }

// determine if reaction needs to be logged
	if(rxn->logserno) {
		if(rxn->logserno->n==0) dorxnlog=1;
		else if(mptr1 && List_MemberLI(rxn->logserno,mptr1->serno)) dorxnlog=1;
		else if(mptr2 && List_MemberLI(rxn->logserno,mptr2->serno)) dorxnlog=1;
		else dorxnlog=0; }

// place products
	nprod=rxn->nprod;
	calc=0;
	dist=0;
	for(prd=0;prd<nprod;prd++) {
		mptr=getnextmol(sim->mols);								// mptr is current product
		if(!mptr) return 1;
		mptr->ident=rxn->prdident[prd];
		mptr->mstate=rxn->prdstate[prd];
		ms=mptr->mstate;
		mptr->list=sim->mols->listlookup[mptr->ident][ms];

		if(rxn->rparamt==RPconfspread) {					// confspread reaction
			if(prd<2) {
				mptrfrom=(prd==0)?mptr1:mptr2;
				mptr->box=mptrfrom->box;
				mptr->serno=mptrfrom->serno;
				for(d=0;d<dim;d++) {
					mptr->posx[d]=mptr->pos[d]=mptrfrom->pos[d];
					mptr->posoffset[d]=mptrfrom->posoffset[d]; }
				mptr->pnl=(ms==MSsoln)?NULL:mptrfrom->pnl; }
			else {
				for(d=0;d<dim;d++)
					mptr->posx[d]=mptr->pos[d]=rxnpos[d];
				if(ms==MSsoln) mptr->pnl=NULL;
				else if(mptrfrom->pnl) mptr->pnl=mptrfrom->pnl;
				else mptr->pnl=rxnpnl;
				mptr->box=rxnbptr; }}

		else if(rxn->rparamt==RPbounce) {					// bounce reaction
			if(prd==0) {
				dist=0;
				for(d=0;d<dim;d++) {
					v1[d]=mptr2->pos[d]-mptr1->pos[d];	// v1 is from mptr1 to mptr2
					dist+=v1[d]*v1[d]; }								// dist is length of v1
				if(dist==0) {
					dist=sqrt(rxn->bindrad2);
					v1[0]=dist; }
				else
					dist=sqrt(dist);
				if(rxn->rparam>=0) x=1.0/dist-(1.0/rxn->rparam);	// x is scaling factor
				else x=2.0/dist-2.0/(rxn->prdpos[0][0]+rxn->prdpos[1][0]);
				mptr->box=mptr1->box;
				mptr->pnl=mptr1->pnl;
				mptr->serno=mptr1->serno;
				for(d=0;d<dim;d++) {
					mptr->posx[d]=mptr1->pos[d];
					mptr->posoffset[d]=mptr1->posoffset[d];
					mptr->pos[d]=mptr1->pos[d]-x*rxn->prdpos[0][0]*v1[d]; }
				if(mptr->pnl) movemol2closepanel(sim,mptr); }
			else if(prd==1) {
				mptr->box=mptr2->box;
				mptr->pnl=mptr2->pnl;
				mptr->serno=mptr2->serno;
				for(d=0;d<dim;d++) {
					mptr->posx[d]=mptr2->pos[d];
					mptr->posoffset[d]=mptr2->posoffset[d];
					mptr->pos[d]=mptr2->pos[d]+x*rxn->prdpos[1][0]*v1[d]; }
				if(mptr->pnl) movemol2closepanel(sim,mptr); }
			else {
				mptr->box=rxnbptr;
				for(d=0;d<dim;d++)
					mptr->posx[d]=mptr->pos[d]=rxnpos[d];
				if(ms==MSsoln) mptr->pnl=NULL;
				else mptr->pnl=rxnpnl; }}

		else {
			mptr->box=rxnbptr;
			for(d=0;d<dim;d++) mptr->posx[d]=rxnpos[d];

			mptr->mstate=ms=rxn->prdstate[prd];

			if(rxn->prdintersurf) {
				if(rxn->prdintersurf[prd]==1) mptr->pnl=mptr1->pnl;
				else mptr->pnl=mptr2->pnl; }
			else
				mptr->pnl=rxnpnl;

			if(!rxnpnl);														// soln -> soln
			else if(ms==MSsoln) {										// surf -> front soln
				fixpt2panel(mptr->posx,mptr->pnl,dim,PFfront,sim->srfss->epsilon);
				mptr->pnl=NULL; }
			else if(ms==MSbsoln) {									// surf -> back soln
				fixpt2panel(mptr->posx,mptr->pnl,dim,PFback,sim->srfss->epsilon);
				mptr->mstate=MSsoln;
				mptr->pnl=NULL; }
			else if(ms==MSfront) {									// surf -> front surf
				fixpt2panel(mptr->posx,mptr->pnl,dim,PFfront,sim->srfss->epsilon); }
			else if(ms==MSback) {										// surf -> back surf
				fixpt2panel(mptr->posx,mptr->pnl,dim,PFback,sim->srfss->epsilon); }
			else {																	// surf -> surf: up, down
				fixpt2panel(mptr->posx,mptr->pnl,dim,PFnone,sim->srfss->epsilon); }

			for(d=0;d<dim && rxn->prdpos[prd][d]==0;d++);
			if(d!=dim) {
				if(rxn->rparamt==RPfixed) {
					for(d=0;d<dim;d++) v1[d]=rxn->prdpos[prd][d]; }
				else if(dim==1) {
					if(!calc) {m3[0]=signrand();calc=1;}
					v1[0]=m3[0]*rxn->prdpos[prd][0]; }
				else if(dim==2) {
					if(!calc) {DirCosM2D(m3,unirandCOD(0,2*PI));calc=1;}
					dotMVD(m3,rxn->prdpos[prd],v1,2,2); }
				else if(dim==3) {
					if(!calc) {DirCosMD(m3,thetarandCCD(),unirandCOD(0,2*PI),unirandCOD(0,2*PI));calc=1;}
					dotMVD(m3,rxn->prdpos[prd],v1,3,3); }
				else {
					if(!calc) {DirCosMD(m3,thetarandCCD(),unirandCOD(0,2*PI),unirandCOD(0,2*PI));calc=1;}
					dotMVD(m3,rxn->prdpos[prd],v1,3,3); }
				for(d=0;d<dim;d++)
					mptr->pos[d]=mptr->posx[d]+v1[d]; }
			else {
				for(d=0;d<dim;d++)
					mptr->pos[d]=mptr->posx[d]; }

			if(mptr->mstate!=MSsoln)
				movemol2closepanel(sim,mptr);

			if(rxn->prdserno) {												// set product serial numbers
				serno=rxn->prdserno[prd];
				if(serno==0);
				else if(serno==-1) mptr->serno=mptr1->serno;
				else if(serno==-2) mptr->serno=mptr2->serno;
				else if(serno<=-10) mptr->serno=sernolist[(int)(-9-serno)];
				else mptr->serno=serno;
				sernolist[prd]=mptr->serno; }}

		if(rxn->logserno) {													// log reaction if needed
			if(dorxnlog==0 && List_MemberLI(rxn->logserno,mptr->serno)) dorxnlog=1;
			if(dorxnlog==1) {
				fptr=scmdgetfptr(sim->cmds,rxn->logfile);
				if(!fptr) {
					simLog(sim,8,"cannot write to reaction log filename '%s'\n",rxn->logfile);
					dorxnlog=-1; }
				else {
					scmdfprintf(sim->cmds,fptr,"%g %s",sim->time,rxn->rname);
					for(d=0;d<dim;d++) scmdfprintf(sim->cmds,fptr," %g",rxnpos[d]);
					if(mptr1) scmdfprintf(sim->cmds,fptr," %li",mptr1->serno);
					if(mptr2) scmdfprintf(sim->cmds,fptr," %li",mptr2->serno);
					for(prd2=0;prd2<prd;prd2++) scmdfprintf(sim->cmds,fptr," ?");
					dorxnlog=2; }}
			if(dorxnlog==2) {
				scmdfprintf(sim->cmds,fptr," %li",mptr->serno);
				if(prd==nprod-1) scmdfprintf(sim->cmds,fptr,"\n"); }}}

	if(mptr1) molkill(sim,mptr1,ll1,m1);					// kill reactants
	if(mptr2) molkill(sim,mptr2,ll2,m2);

	return 0; }


/* zeroreact */
int zeroreact(simptr sim) {
	int i,r,nmol;
	rxnptr rxn;
	rxnssptr rxnss;
	double pos[DIMMAX];
	panelptr pnl;

	pnl=NULL;
	rxnss=sim->rxnss[0];
	if(!rxnss) return 0;
	for(r=0;r<rxnss->totrxn;r++) {
		rxn=rxnss->rxn[r];
				
#ifdef OPTION_VCELL
		if(rxn->rateValueProvider != NULL) {
			if(rxn->cmpt) {
				AbstractMesh* mesh = sim->mesh;
				double delta[3];
				mesh->getDeltaXYZ(delta);
				int n[3];
				mesh->getNumXYZ(n);
				double meshv = delta[0]*delta[1]*delta[2];
				int totalNumMesh = n[0]*n[1]*n[2];			
			
				for(i=0; i<totalNumMesh; i++) {
					int volIndex = i;
					double centerPos[3];
					mesh->getCenterCoordinates(volIndex, centerPos);
					if (posincompart(sim, centerPos, rxn->cmpt)) {
					   // go through each mesh elements to see if it is in the compartments that the reaction happens
						double rate =  evaluateVolRnxRate(sim, rxn, centerPos);
						double prob = rate * sim->dt * meshv;	
						nmol=poisrandD(prob);
						int count = 0;
						//put generated molecules in the same mesh
						double pos[3];
						while(count < nmol) {
							count++;
							randomPosInMesh(sim,centerPos, pos); //the generated position saved in pos
							//since we are using vcell mesh, we double check the pos in compartment
							if(posincompart(sim,pos,rxn->cmpt)) {
								//count++;
								if(doreact(sim,rxn,NULL,NULL,-1,-1,-1,-1,pos,pnl)) return 1; }}
						sim->eventcount[ETrxn0]+=nmol; }}}
			else if(rxn->srf) {
				int numOfSuf = sim->srfss->nsrf;
				surfaceptr * surfaces = sim->srfss->srflist;
				for(i=0; i<numOfSuf; i++) {
					if(surfaces[i] == rxn->srf) { //find the surface where the reaction happens
						//get all the panels on the surface
						int numOfTriPanels = surfaces[i]->npanel[PStri];
						panelptr* panels = surfaces[i]->panels[PStri];
						for(long j=0; j<numOfTriPanels; j++) {
							//evaluate rate 
							double ** points = panels[j]->point; //point[number][dim]
							double triCenterPos[3]; 
							Geo_TriCenter(points, triCenterPos, sim->dim);
							double rate = evaluateMemRnxRate(sim, rxn, triCenterPos, panels[j]->pname);
							//get probability
							double triPanelArea = Geo_TriArea3D(points[0], points[1], points[2]);
							double prob = rate * sim->dt * triPanelArea;
							//num of molecules generated
							nmol=poisrandD(prob);
							//uniformly and randomly place the molecule 
							int count = 0;
							//put generated molecules in the same mesh
							while(count < nmol) {
								count++;
								panelrandpos(panels[j],pos,sim->dim);//randomly generate a position for surface molecule
								////since we are using vcell mesh, we double check the pos in compartment
								//if(posincompart(sim,pos,rxn->cmpt))
								//{
									//count++; 
									if(doreact(sim,rxn,NULL,NULL,-1,-1,-1,-1,pos,panels[j])) return 1; 
								//}
							}
							sim->eventcount[ETrxn0]+=nmol; }}}}}
		else
#endif

		{
			nmol=poisrandD(rxn->prob);
			for(i=0;i<nmol;i++) {
				if(rxn->cmpt) compartrandpos(sim,pos,rxn->cmpt);
				else if(rxn->srf) pnl=surfrandpos(rxn->srf,pos,sim->dim);
				else systemrandpos(sim,pos);
				if(doreact(sim,rxn,NULL,NULL,-1,-1,-1,-1,pos,pnl)) return 1; }
			sim->eventcount[ETrxn0]+=nmol; }}
	return 0; }


/* unireact */
int unireact(simptr sim) {
	rxnssptr rxnss;
	rxnptr rxn,*rxnlist;
	moleculeptr *mlist,mptr;
	int *nrxn,**table;
	int i,j,m,nmol,ll;
	enum MolecState ms;

	rxnss=sim->rxnss[1];
	if(!rxnss) return 0;
	nrxn=rxnss->nrxn;
	table=rxnss->table;
	rxnlist=rxnss->rxn;
	for(ll=0;ll<sim->mols->nlist;ll++)
		if(rxnss->rxnmollist[ll]) {
			mlist=sim->mols->live[ll];
			nmol=sim->mols->nl[ll];
			for(m=0;m<nmol;m++) {
				mptr=mlist[m];
				i=mptr->ident;
				ms=mptr->mstate;
				for(j=0;j<nrxn[i];j++) {
					rxn=rxnlist[table[i][j]];
#ifdef OPTION_VCELL
					if(rxn->rateValueProvider != NULL)
					{
						if(rxn->srf)//surface reaction
						{
							if(mptr->pnl)
							{
								rxn -> rate = evaluateMemRnxRate(sim, rxn, mptr->pos, mptr->pnl->pname);
							}
							else
							{
								simLog(sim,10,"Unimolecular membrance reaction should have a membrane reactant.");
							}
						}
						else
						{
							rxn -> rate = evaluateVolRnxRate(sim, rxn, mptr->pos);
						}
						char erstr[STRCHAR];
						int er, r;
						r = table[i][j];
						er=rxnsetrate(sim,1,r,erstr);
					}
#endif

					if(!rxn->permit[ms]);																						// failed permit test
					else if(!coinrandD(rxn->prob));																	// failed probability test
					else if(rxn->cmpt && !posincompart(sim,mptr->pos,rxn->cmpt));		// failed compartment test
					else if(rxn->srf && (!mptr->pnl || mptr->pnl->srf!=rxn->srf));	// failed surface test
					else if(mptr->ident==0);																				// failed existance test
					else {																													// react
						if(doreact(sim,rxn,mptr,NULL,ll,m,-1,-1,NULL,NULL)) return 1;
						sim->eventcount[ETrxn1]++;
						j=nrxn[i]; }}}}
	return 0; }


/* morebireact */
int morebireact(simptr sim,rxnptr rxn,moleculeptr mptr1,moleculeptr mptr2,int ll1,int m1,int ll2,enum EventType et,double *vect) {
	moleculeptr mptrA,mptrB;
	int d,swap;
	enum MolecState ms,msA,msB;

	if(rxn->cmpt && !(posincompart(sim,mptr1->pos,rxn->cmpt) && posincompart(sim,mptr2->pos,rxn->cmpt))) return 0;
	if(rxn->srf && !((mptr1->pnl && mptr1->pnl->srf==rxn->srf ) || (mptr2->pnl && mptr2->pnl->srf==rxn->srf))) return 0;

	if(mptr1->pnl && mptr2->pnl && mptr1->pnl!=mptr2->pnl) {					// intersurface reaction
		if(rxn->rparamt==RPconfspread || rxn->rparamt==RPbounce || rxn->prdintersurf);
		else return 0; }

	if(mptr1->ident==rxn->rctident[0]) {
		mptrA=mptr1;
		mptrB=mptr2;
		swap=0; }
	else {
		mptrA=mptr2;
		mptrB=mptr1;
		swap=1; }

	msA=mptrA->mstate;
	msB=mptrB->mstate;
	if(msA==MSsoln && msB!=MSsoln)
		msA=(panelside(mptrA->pos,mptrB->pnl,sim->dim,NULL,0)==PFfront)?MSsoln:MSbsoln;
	else if(msB==MSsoln && msA!=MSsoln)
		msB=(panelside(mptrB->pos,mptrA->pnl,sim->dim,NULL,0)==PFfront)?MSsoln:MSbsoln;
	ms=(MolecState)(msA*MSMAX1+msB);

	if(rxn->permit[ms]) {
		if(et==ETrxn2wrap && rxn->rparamt!=RPconfspread) {		// if wrapping, then move faster diffusing molecule
			if(sim->mols->difc[mptr1->ident][mptr1->mstate]<sim->mols->difc[mptr2->ident][mptr2->mstate])
				for(d=0;d<sim->dim;d++) {
					mptr2->posoffset[d]-=(mptr1->pos[d]+vect[d])-mptr2->pos[d];
					mptr2->pos[d]=mptr1->pos[d]+vect[d]; }
			else
				for(d=0;d<sim->dim;d++) {
					mptr1->posoffset[d]-=(mptr2->pos[d]-vect[d])-mptr1->pos[d];
					mptr1->pos[d]=mptr2->pos[d]-vect[d]; }}
		sim->eventcount[et]++;
		if(!swap) return doreact(sim,rxn,mptrA,mptrB,ll1,m1,ll2,-1,NULL,NULL);
		else return doreact(sim,rxn,mptrA,mptrB,ll2,-1,ll1,m1,NULL,NULL); }

	return 0; }


#ifdef OPTION_VCELL
void setBiReactRateForHybrid(simptr sim,rxnptr rxn,moleculeptr mptr1,moleculeptr mptr2,int r)
{
	if(rxn->rateValueProvider != NULL)
	{
		int dimension = sim->dim;
		double pos[3]; 
		for(int dimIdx =0; dimIdx < dimension; dimIdx++)
		{
			pos[dimIdx] = (mptr1->pos[dimIdx] + mptr2->pos[dimIdx])/2;
		}
		if(rxn->srf)//surface reaction
		{
			if(mptr1->pnl)
			{
				rxn -> rate = evaluateMemRnxRate(sim, rxn, pos, mptr1->pnl->pname);
			}
			else if(mptr2->pnl)
			{
				rxn -> rate = evaluateMemRnxRate(sim, rxn, pos, mptr2->pnl->pname);
			}
			else
			{
				simLog(sim,10,"Bimolecular membrance reaction should have at least one membrane reactant.");
			}
		}
		else
		{
			rxn -> rate = evaluateVolRnxRate(sim, rxn, pos);
		}

		char erstr[STRCHAR];
		int er;
		er=rxnsetrate(sim,2,r,erstr);
	}
}
#endif


/* bireact */
int bireact(simptr sim,int neigh) {
	int dim,maxspecies,ll1,ll2,i,j,d,*nl,nmol2,b2,m1,m2,bmax,wpcode,nlist,maxlist;
	int *nrxn,**table;
	double dist2,vect[DIMMAX];
	rxnssptr rxnss;
	rxnptr rxn,*rxnlist;
	boxptr bptr;
	moleculeptr **live,*mlist2,mptr1,mptr2;

	rxnss=sim->rxnss[2];
	if(!rxnss) return 0;
	dim=sim->dim;
	live=sim->mols->live;
	maxspecies=rxnss->maxspecies;
	maxlist=rxnss->maxlist;
	nlist=sim->mols->nlist;
	nrxn=rxnss->nrxn;
	table=rxnss->table;
	rxnlist=rxnss->rxn;
	nl=sim->mols->nl;

	if(!neigh) {																		// same box
		for(ll1=0;ll1<nlist;ll1++)
			for(ll2=ll1;ll2<nlist;ll2++)
				if(rxnss->rxnmollist[ll1*maxlist+ll2])
					for(m1=0;m1<nl[ll1];m1++) {
						mptr1=live[ll1][m1];
						bptr=mptr1->box;
						mlist2=bptr->mol[ll2];
						nmol2=bptr->nmol[ll2];
						for(m2=0;m2<nmol2 && mlist2[m2]!=mptr1;m2++) {
							mptr2=mlist2[m2];
							i=mptr1->ident*maxspecies+mptr2->ident;
							for(j=0;j<nrxn[i];j++) {
								rxn=rxnlist[table[i][j]];
#ifdef OPTION_VCELL
								setBiReactRateForHybrid(sim, rxn, mptr1, mptr2, table[i][j]);
#endif
								dist2=0;
								for(d=0;d<dim;d++)
									dist2+=(mptr1->pos[d]-mptr2->pos[d])*(mptr1->pos[d]-mptr2->pos[d]);
								if(dist2<=rxn->bindrad2 && (rxn->prob==1 || randCOD()<rxn->prob) && (mptr1->mstate!=MSsoln || mptr2->mstate!=MSsoln || !rxnXsurface(sim,mptr1,mptr2)) && mptr1->ident!=0 && mptr2->ident!=0) {
									if(morebireact(sim,rxn,mptr1,mptr2,ll1,m1,ll2,ETrxn2intra,NULL)) return 1;
									if(mptr1->ident==0) {
										j=nrxn[i];
										m2=nmol2; }}}}}}

	else {																					// neighbor box
		for(ll1=0;ll1<nlist;ll1++)
			for(ll2=ll1;ll2<nlist;ll2++)
				if(rxnss->rxnmollist[ll1*maxlist+ll2])
					for(m1=0;m1<nl[ll1];m1++) {
						mptr1=live[ll1][m1];
						bptr=mptr1->box;
						bmax=(ll1!=ll2)?bptr->nneigh:bptr->midneigh;
						for(b2=0;b2<bmax;b2++) {
							mlist2=bptr->neigh[b2]->mol[ll2];
							nmol2=bptr->neigh[b2]->nmol[ll2];
							if(bptr->wpneigh && bptr->wpneigh[b2]) {	// neighbor box with wrapping
								wpcode=bptr->wpneigh[b2];
								for(m2=0;m2<nmol2;m2++) {
									mptr2=mlist2[m2];
									i=mptr1->ident*maxspecies+mptr2->ident;
									for(j=0;j<nrxn[i];j++) {
										rxn=rxnlist[table[i][j]];
#ifdef OPTION_VCELL
										setBiReactRateForHybrid(sim, rxn, mptr1, mptr2, table[i][j]);
#endif
										dist2=wallcalcdist2(sim,mptr1->pos,mptr2->pos,wpcode,vect);
										if(dist2<=rxn->bindrad2 && (rxn->prob==1 || randCOD()<rxn->prob) && mptr1->ident!=0 && mptr2->ident!=0) {
											if(morebireact(sim,rxn,mptr1,mptr2,ll1,m1,ll2,ETrxn2wrap,vect)) return 1;
											if(mptr1->ident==0) {
												j=nrxn[i];
												m2=nmol2;
												b2=bmax; }}}}}

							else													// neighbor box, no wrapping
								for(m2=0;m2<nmol2;m2++) {
									mptr2=mlist2[m2];
									i=mptr1->ident*maxspecies+mptr2->ident;
									for(j=0;j<nrxn[i];j++) {
										rxn=rxnlist[table[i][j]];
#ifdef OPTION_VCELL
										setBiReactRateForHybrid(sim, rxn, mptr1, mptr2, table[i][j]);
#endif
										dist2=0;
										for(d=0;d<dim;d++)
											dist2+=(mptr1->pos[d]-mptr2->pos[d])*(mptr1->pos[d]-mptr2->pos[d]);
										if(dist2<=rxn->bindrad2 && (rxn->prob==1 || randCOD()<rxn->prob) &&  (mptr1->mstate!=MSsoln || mptr2->mstate!=MSsoln || !rxnXsurface(sim,mptr1,mptr2)) && mptr1->ident!=0 && mptr2->ident!=0) {
											if(morebireact(sim,rxn,mptr1,mptr2,ll1,m1,ll2,ETrxn2inter,NULL)) return 1;
											if(mptr1->ident==0) {
												j=nrxn[i];
												m2=nmol2;
												b2=bmax; }}}}}}}

	return 0; }


