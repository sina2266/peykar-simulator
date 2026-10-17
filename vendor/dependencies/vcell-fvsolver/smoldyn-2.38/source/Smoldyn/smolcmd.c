/* Steven Andrews, started 10/22/2001.
 This is a library of functions for the Smoldyn program.
 See documentation called Smoldyn_doc1.pdf and Smoldyn_doc2.pdf, and the Smoldyn
 website, which is at www.smoldyn.org.
 Copyright 2003-2013 by Steven Andrews.  This work is distributed under the terms
 of the Gnu Lesser General Public License (LGPL). */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Geometry.h"
#include "opengl2.h"
#include "random2.h"
#include "Rn.h"
#include "RnSort.h"
#include "smoldyn.h"
#include "smoldynfuncs.h"
#include "string2.h"
#include "Zn.h"
#include <string>

#include "smoldynconfigure.h"


/**********************************************************/
/******************** command declarations ****************/
/**********************************************************/

#ifdef VCELL
#include <vcellcmd.h>
#endif

// simulation control
enum CMDcode cmdstop(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdpause(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdbeep(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdkeypress(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsetflag(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsetrandseed(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsetgraphics(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsetgraphic_iter(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdupdategraphics(simptr sim,cmdptr cmd,char *line2);

// file manipulation
enum CMDcode cmdoverwrite(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdincrementfile(simptr sim,cmdptr cmd,char *line2);

// conditional
enum CMDcode cmdifflag(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdifprob(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdifno(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdifless(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdifmore(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdifincmpt(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdifchange(simptr sim,cmdptr cmd,char *line2);

// system observation
enum CMDcode cmdecho(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdwarnescapee(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountheader(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcount(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountinbox(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountincmpt(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountincmpts(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountincmpt2(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountonsurf(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountspace(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolcountspecies(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmollistsize(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdlistmols(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdlistmols2(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdlistmols3(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdlistmols4(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdlistmolscmpt(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolpos(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdtrackmol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmolmoments(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsavesim(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmeansqrdisp(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmeansqrdisp2(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmeansqrdisp3(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdresidencetime(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmddiagnostics(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdexecutiontime(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdwriteVTK(simptr sim,cmdptr cmd,char *line2);

enum CMDcode cmdprintLattice(simptr sim,cmdptr cmd,char *line2);

// system manipulation
enum CMDcode cmdset(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdpointsource(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdvolumesource(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmovesurfacemol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdkillmol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdkillmolprob(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdkillmolinsphere(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdkillmolincmpt(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdkillmoloutsidesystem(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdfixmolcount(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdfixmolcountrange(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdfixmolcountonsurf(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdfixmolcountrangeonsurf(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdfixmolcountincmpt(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdfixmolcountrangeincmpt(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdequilmol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdreplacexyzmol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdreplacevolmol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdreplacecmptmol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdmodulatemol(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdreact1(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsetrateint(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdshufflemollist(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdshufflereactions(simptr sim,cmdptr cmd,char *line2);
//enum CMDcode cmdsetsurfcoeff(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsettimestep(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdporttransport(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdexcludebox(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdexcludesphere(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdincludeecoli(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdsetreactionratemolcount(simptr sim,cmdptr cmd,char *line2);
enum CMDcode cmdexpandsystem(simptr sim,cmdptr cmd,char *line2);

// internal functions
void cmdv1free(cmdptr cmd);
void cmdv1v2free(cmdptr cmd);
enum CMDcode conditionalcmdtype(simptr sim,cmdptr cmd,int nparam);
int insideecoli(double *pos,double *ofst,double rad,double length);
void putinecoli(double *pos,double *ofst,double rad,double length);
int molinpanels(simptr sim,int ll,int m,int s,char pshape);
void cmdmeansqrdispfree(cmdptr cmd);


/**********************************************************/
/********************* command processor ******************/
/**********************************************************/

enum CMDcode docommand(void *cmdfnarg,cmdptr cmd,char *line) {
	simptr sim;
	char word[STRCHAR],*line2;
	int itct;

	if(!cmdfnarg) return CMDok;
	sim=(simptr) cmdfnarg;
	if(!line) return CMDok;
	itct=sscanf(line,"%s",word);
	if(itct<=0) return CMDok;
	line2=strnword(line,2);

	// simulation control
	if(!strcmp(word,"stop")) return cmdstop(sim,cmd,line2);
	else if(!strcmp(word,"pause")) return cmdpause(sim,cmd,line2);
	else if(!strcmp(word,"beep")) return cmdbeep(sim,cmd,line2);
	else if(!strcmp(word,"keypress")) return cmdkeypress(sim,cmd,line2);
	else if(!strcmp(word,"setflag")) return cmdsetflag(sim,cmd,line2);
	else if(!strcmp(word,"setrandseed")) return cmdsetrandseed(sim,cmd,line2);
	else if(!strcmp(word,"setgraphics")) return cmdsetgraphics(sim,cmd,line2);
	else if(!strcmp(word,"setgraphic_iter")) return cmdsetgraphic_iter(sim,cmd,line2);
	else if(!strcmp(word,"updategraphics")) return cmdupdategraphics(sim,cmd,line2);

	// file manipulation
	else if(!strcmp(word,"overwrite")) return cmdoverwrite(sim,cmd,line2);
	else if(!strcmp(word,"incrementfile")) return cmdincrementfile(sim,cmd,line2);

	// conditional
	else if(!strcmp(word,"ifflag")) return cmdifflag(sim,cmd,line2);
	else if(!strcmp(word,"ifprob")) return cmdifprob(sim,cmd,line2);
	else if(!strcmp(word,"ifno")) return cmdifno(sim,cmd,line2);
	else if(!strcmp(word,"ifless")) return cmdifless(sim,cmd,line2);
	else if(!strcmp(word,"ifmore")) return cmdifmore(sim,cmd,line2);
	else if(!strcmp(word,"ifincmpt")) return cmdifincmpt(sim,cmd,line2);
	else if(!strcmp(word,"ifchange")) return cmdifchange(sim,cmd,line2);

	// system observation
	else if(!strcmp(word,"echo")) return cmdecho(sim,cmd,line2);
	else if(!strcmp(word,"warnescapee")) return cmdwarnescapee(sim,cmd,line2);
	else if(!strcmp(word,"molcountheader")) return cmdmolcountheader(sim,cmd,line2);
	else if(!strcmp(word,"molcount")) return cmdmolcount(sim,cmd,line2);
	else if(!strcmp(word,"molcountinbox")) return cmdmolcountinbox(sim,cmd,line2);
	else if(!strcmp(word,"molcountincmpt")) return cmdmolcountincmpt(sim,cmd,line2);
	else if(!strcmp(word,"molcountincmpts")) return cmdmolcountincmpts(sim,cmd,line2);
	else if(!strcmp(word,"molcountincmpt2")) return cmdmolcountincmpt2(sim,cmd,line2);
	else if(!strcmp(word,"molcountonsurf")) return cmdmolcountonsurf(sim,cmd,line2);
	else if(!strcmp(word,"molcountspace")) return cmdmolcountspace(sim,cmd,line2);
	else if(!strcmp(word,"molcountspecies")) return cmdmolcountspecies(sim,cmd,line2);
	else if(!strcmp(word,"mollistsize")) return cmdmollistsize(sim,cmd,line2);
	else if(!strcmp(word,"listmols")) return cmdlistmols(sim,cmd,line2);
	else if(!strcmp(word,"listmols2")) return cmdlistmols2(sim,cmd,line2);
	else if(!strcmp(word,"listmols3")) return cmdlistmols3(sim,cmd,line2);
	else if(!strcmp(word,"listmols4")) return cmdlistmols4(sim,cmd,line2);
	else if(!strcmp(word,"listmolscmpt")) return cmdlistmolscmpt(sim,cmd,line2);
	else if(!strcmp(word,"molpos")) return cmdmolpos(sim,cmd,line2);
	else if(!strcmp(word,"trackmol")) return cmdtrackmol(sim,cmd,line2);
	else if(!strcmp(word,"molmoments")) return cmdmolmoments(sim,cmd,line2);
	else if(!strcmp(word,"savesim")) return cmdsavesim(sim,cmd,line2);
	else if(!strcmp(word,"meansqrdisp")) return cmdmeansqrdisp(sim,cmd,line2);
	else if(!strcmp(word,"meansqrdisp2")) return cmdmeansqrdisp2(sim,cmd,line2);
	else if(!strcmp(word,"meansqrdisp3")) return cmdmeansqrdisp3(sim,cmd,line2);
	else if(!strcmp(word,"residencetime")) return cmdresidencetime(sim,cmd,line2);
	else if(!strcmp(word,"diagnostics")) return cmddiagnostics(sim,cmd,line2);
	else if(!strcmp(word,"executiontime")) return cmdexecutiontime(sim,cmd,line2);
	else if(!strcmp(word,"writeVTK")) return cmdwriteVTK(sim,cmd,line2);
	else if(!strcmp(word,"printLattice")) return cmdprintLattice(sim,cmd,line2);


	// system manipulation
	else if(!strcmp(word,"set")) return cmdset(sim,cmd,line2);
	else if(!strcmp(word,"pointsource")) return cmdpointsource(sim,cmd,line2);
	else if(!strcmp(word,"volumesource")) return cmdvolumesource(sim,cmd,line2);
	else if(!strcmp(word,"movesurfacemol")) return cmdmovesurfacemol(sim,cmd,line2);
	else if(!strcmp(word,"killmol")) return cmdkillmol(sim,cmd,line2);
	else if(!strcmp(word,"killmolprob")) return cmdkillmolprob(sim,cmd,line2);
	else if(!strcmp(word,"killmolinsphere")) return cmdkillmolinsphere(sim,cmd,line2);
	else if(!strcmp(word,"killmolincmpt")) return cmdkillmolincmpt(sim,cmd,line2);
	else if(!strcmp(word,"killmoloutsidesystem")) return cmdkillmoloutsidesystem(sim,cmd,line2);
	else if(!strcmp(word,"fixmolcount")) return cmdfixmolcount(sim,cmd,line2);
	else if(!strcmp(word,"fixmolcountrange")) return cmdfixmolcountrange(sim,cmd,line2);
	else if(!strcmp(word,"fixmolcountonsurf")) return cmdfixmolcountonsurf(sim,cmd,line2);
	else if(!strcmp(word,"fixmolcountrangeonsurf")) return cmdfixmolcountrangeonsurf(sim,cmd,line2);
	else if(!strcmp(word,"fixmolcountincmpt")) return cmdfixmolcountincmpt(sim,cmd,line2);
	else if(!strcmp(word,"fixmolcountrangeincmpt")) return cmdfixmolcountrangeincmpt(sim,cmd,line2);
	else if(!strcmp(word,"equilmol")) return cmdequilmol(sim,cmd,line2);
	else if(!strcmp(word,"replacexyzmol")) return cmdreplacexyzmol(sim,cmd,line2);
	else if(!strcmp(word,"replacevolmol")) return cmdreplacevolmol(sim,cmd,line2);
	else if(!strcmp(word,"replacecmptmol")) return cmdreplacecmptmol(sim,cmd,line2);
	else if(!strcmp(word,"modulatemol")) return cmdmodulatemol(sim,cmd,line2);
	else if(!strcmp(word,"react1")) return cmdreact1(sim,cmd,line2);
	else if(!strcmp(word,"setrateint")) return cmdsetrateint(sim,cmd,line2);
	else if(!strcmp(word,"shufflemollist")) return cmdshufflemollist(sim,cmd,line2);
	else if(!strcmp(word,"shufflereactions")) return cmdshufflereactions(sim,cmd,line2);
//	else if(!strcmp(word,"setsurfcoeff")) return cmdsetsurfcoeff(sim,cmd,line2);
	else if(!strcmp(word,"settimestep")) return cmdsettimestep(sim,cmd,line2);
	else if(!strcmp(word,"porttransport")) return cmdporttransport(sim,cmd,line2);
	else if(!strcmp(word,"excludebox")) return cmdexcludebox(sim,cmd,line2);
	else if(!strcmp(word,"excludesphere")) return cmdexcludesphere(sim,cmd,line2);
	else if(!strcmp(word,"includeecoli")) return cmdincludeecoli(sim,cmd,line2);
	else if(!strcmp(word,"setreactionratemolcount")) return cmdsetreactionratemolcount(sim,cmd,line2);
	else if(!strcmp(word,"expandsystem")) return cmdexpandsystem(sim,cmd,line2);

#ifdef VCELL
	// vcell commands
	else if(!strcmp(word,"vcellPrintProgress")) return cmdVCellPrintProgress(sim,cmd,line2);
	else if(!strcmp(word,"vcellWriteOutput")) return cmdVCellWriteOutput(sim,cmd,line2);
	else if(!strcmp(word,"vcellDataProcess")) return cmdVCellDataProcess(sim,cmd,line2);
#endif

	SCMDCHECK(0,"command not recognized");
	return CMDwarn; }


/**********************************************************/
/****************** simulation control ********************/
/**********************************************************/

enum CMDcode cmdstop(simptr sim,cmdptr cmd,char *line2) {
	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	return CMDstop; }


enum CMDcode cmdpause(simptr sim,cmdptr cmd,char *line2) {
	char c;
	int tflag;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	if(!sim->graphss || sim->graphss->graphics==0) {
		fprintf(stderr,"Simulation paused at time %g.  Press enter to continue.",sim->time);
		scanf("%c",&c);
		return CMDok; }
	tflag=strchr(sim->flags,'t')?1:0;
	SCMDCHECK(sim->graphss && sim->graphss->graphics!=0 && !tflag,"pause doesn't work without graphics");
	gl2State(1);
	return CMDpause; }


enum CMDcode cmdbeep(simptr sim,cmdptr cmd,char *line2) {
	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	fprintf(stderr,"\7");
	return CMDok; }


enum CMDcode cmdkeypress(simptr sim,cmdptr cmd,char *line2) {
	char c;
	int itct,tflag;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%c",&c);
	SCMDCHECK(itct==1,"cannot read character");
	tflag=strchr(sim->flags,'t')?1:0;
	SCMDCHECK(sim->graphss && sim->graphss->graphics!=0 && !tflag,"keypress doesn't work without graphics");
	gl2SetKeyPush((unsigned char) c);
	return CMDok; }


enum CMDcode cmdsetflag(simptr sim,cmdptr cmd,char *line2) {
	double f1;
	int itct;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%lf",&f1);
	SCMDCHECK(itct==1,"cannot read flag value");
	scmdsetflag(sim->cmds,f1);
	return CMDok; }


enum CMDcode cmdsetrandseed(simptr sim,cmdptr cmd,char *line2) {
	int itct;
	long int seed;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%li",&seed);
	SCMDCHECK(itct==1,"cannot read seed");
	if(seed<0) randomize((long int) time(NULL));
	else randomize((long int) seed);
	return CMDok; }


enum CMDcode cmdsetgraphics(simptr sim,cmdptr cmd,char *line2) {
	int itct;
	char str[STRCHAR];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	if(!sim->graphss || sim->graphss->graphics==0) return CMDok;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s",str);
	SCMDCHECK(itct==1,"cannot read graphics type");
	if(!strcmp(str,"opengl")) sim->graphss->graphics=1;
	else if(!strcmp(str,"opengl_good")) sim->graphss->graphics=2;
	else SCMDCHECK(0,"unrecognized graphics type");
	return CMDok; }


enum CMDcode cmdsetgraphic_iter(simptr sim,cmdptr cmd,char *line2) {
	int itct,iter;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	if(!sim->graphss || sim->graphss->graphics==0) return CMDok;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%i",&iter);
	SCMDCHECK(itct==1,"cannot read graphics iterations");
	SCMDCHECK(iter>0,"graphics iterations must be >0");
	sim->graphss->graphicit=iter;
	return CMDok; }


enum CMDcode cmdupdategraphics(simptr sim,cmdptr cmd,char *line2) {
	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	if(!sim->graphss || sim->graphss->graphics==0) return CMDok;
	smolPostRedisplay();
	return CMDok; }


/**********************************************************/
/****************** file manipulation ********************/
/**********************************************************/


enum CMDcode cmdoverwrite(simptr sim,cmdptr cmd,char *line2) {
	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(scmdoverwrite(sim->cmds,line2),"failed to open file");
	return CMDok; }


enum CMDcode cmdincrementfile(simptr sim,cmdptr cmd,char *line2) {
	if(line2 && !strcmp(line2,"cmdtype")) return CMDcontrol;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(scmdincfile(sim->cmds,line2),"failed to increment file");
	return CMDok; }



/**********************************************************/
/********************** conditional ***********************/
/**********************************************************/


enum CMDcode cmdifflag(simptr sim,cmdptr cmd,char *line2) {
	int itct;
	char ch;
	double f1,flag;
	
	if(line2 && !strcmp(line2,"cmdtype")) return conditionalcmdtype(sim,cmd,2);
	SCMDCHECK(line2,"missing arguments");
	itct=sscanf(line2,"%c %lf",&ch,&f1);
	SCMDCHECK(itct==2,"cannot read comparison symbol or flag value");
	SCMDCHECK(ch=='<' || ch=='=' || ch=='>',"comparison symbol has to be <, =, or >");
	flag=scmdreadflag(sim->cmds);
	if((ch=='<' && flag<f1) || (ch=='=' && flag==f1) || (ch=='>' && flag>f1))
		return docommand(sim,cmd,strnword(line2,3));
	return CMDok; }


enum CMDcode cmdifprob(simptr sim,cmdptr cmd,char *line2) {
	int itct;
	double f1;

	if(line2 && !strcmp(line2,"cmdtype")) return conditionalcmdtype(sim,cmd,1);
	SCMDCHECK(line2,"missing arguments");
	itct=sscanf(line2,"%lf",&f1);
	SCMDCHECK(itct==1,"cannot read probability value");
	SCMDCHECK(f1>=0 && f1<=1,"probability value needs to be between 0 and 1");
	if(randCOD()<f1)
		return docommand(sim,cmd,strnword(line2,2));
	return CMDok; }


enum CMDcode cmdifno(simptr sim,cmdptr cmd,char *line2) {
	int er,count,*index;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) {
		return conditionalcmdtype(sim,cmd,1); }

	er=molstring2index1(sim,line2,&ms,&index);
	SCMDCHECK(er!=-1,"species is missing or cannot be read");
	SCMDCHECK(er!=-2,"mismatched or improper parentheses around molecule state");
	SCMDCHECK(er!=-3,"cannot read molecule state value");
	SCMDCHECK(er!=-4,"molecule name not recognized");
	SCMDCHECK(er!=-7,"error allocating memory");
	count=molcount(sim,er,index,ms,NULL,1);
	if(count) return CMDok;
	return docommand(sim,cmd,strnword(line2,2)); }


enum CMDcode cmdifless(simptr sim,cmdptr cmd,char *line2) {
	int itct,er,*index,count,min;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) {
		return conditionalcmdtype(sim,cmd,2); }

	er=molstring2index1(sim,line2,&ms,&index);
	SCMDCHECK(er!=-1,"species is missing or cannot be read");
	SCMDCHECK(er!=-2,"mismatched or improper parentheses around molecule state");
	SCMDCHECK(er!=-3,"cannot read molecule state value");
	SCMDCHECK(er!=-4,"molecule name not recognized");
	SCMDCHECK(er!=-7,"error allocating memory");
	SCMDCHECK(line2=strnword(line2,2),"missing value argument");
	itct=sscanf(line2,"%i",&min);
	SCMDCHECK(itct==1,"cannot read value argument");
	count=molcount(sim,er,index,ms,NULL,min);
	if(count<min) return docommand(sim,cmd,strnword(line2,2));
	return CMDok; }


enum CMDcode cmdifmore(simptr sim,cmdptr cmd,char *line2) {
	int itct,er,*index,count,min;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) {
		return conditionalcmdtype(sim,cmd,2); }

	er=molstring2index1(sim,line2,&ms,&index);
	SCMDCHECK(er!=-1,"species is missing or cannot be read");
	SCMDCHECK(er!=-2,"mismatched or improper parentheses around molecule state");
	SCMDCHECK(er!=-3,"cannot read molecule state value");
	SCMDCHECK(er!=-4,"molecule name not recognized");
	SCMDCHECK(er!=-7,"error allocating memory");
	SCMDCHECK(line2=strnword(line2,2),"missing value argument");
	itct=sscanf(line2,"%i",&min);
	SCMDCHECK(itct==1,"cannot read value argument");
	count=molcount(sim,er,index,ms,NULL,min+1);
	if(count>min) return docommand(sim,cmd,strnword(line2,2));
	return CMDok; }


enum CMDcode cmdifincmpt(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,count,min,c,ll,m;
	enum MolecState ms;
	char cname[STRCHAR];
	compartssptr cmptss;
	compartptr cmpt;
	char ch;
	moleculeptr mptr;

	if(line2 && !strcmp(line2,"cmdtype")) return conditionalcmdtype(sim,cmd,4);

	cmptss=sim->cmptss;
	SCMDCHECK(cmptss,"no compartments defined");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(line2,"missing argument");
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<=0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	SCMDCHECK(line2=strnword(line2,2),"missing value argument");
	itct=sscanf(line2,"%c %i %s",&ch,&min,cname);
	SCMDCHECK(itct==3,"cannot read symbol, value, and/or compartment arguments");
	SCMDCHECK(ch=='<' || ch=='=' || ch=='>',"comparison symbol has to be <, =, or >");
	c=stringfind(cmptss->cnames,cmptss->ncmpt,cname);
	SCMDCHECK(c>=0,"compartment name not recognized");
	cmpt=cmptss->cmptlist[c];
	line2=strnword(line2,4);

	count=0;
	if(i>0 && ms!=MSall) {
		ll=sim->mols->listlookup[i][ms];
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident==i && mptr->mstate==ms && posincompart(sim,mptr->pos,cmpt))
				count++; }}
	else {
		for(ll=0;ll<sim->mols->nlist;ll++)
			if(sim->mols->listtype[ll]==MLTsystem)
				for(m=0;m<sim->mols->nl[ll];m++) {
					mptr=sim->mols->live[ll][m];
					if(i==-5 || mptr->ident==i)
						if(ms==MSall || mptr->mstate==ms)
							if(posincompart(sim,mptr->pos,cmpt))
								count++; }}

	if((ch=='<' && count<min) || (ch=='=' && count==min) || (ch=='>' && count>min))
		return docommand(sim,cmd,line2);
	return CMDok; }


enum CMDcode cmdifchange(simptr sim,cmdptr cmd,char *line2) {
	int itct,er,*index,count,num,diff;
	enum MolecState ms;
  char change;
  
	if(line2 && !strcmp(line2,"cmdtype")) {
		return conditionalcmdtype(sim,cmd,2); }

	er=molstring2index1(sim,line2,&ms,&index);
	SCMDCHECK(er!=-1,"species is missing or cannot be read");
	SCMDCHECK(er!=-2,"mismatched or improper parentheses around molecule state");
	SCMDCHECK(er!=-3,"cannot read molecule state value");
	SCMDCHECK(er!=-4,"molecule name not recognized");
	SCMDCHECK(er!=-7,"error allocating memory");
	SCMDCHECK(line2=strnword(line2,2),"missing value argument");
	itct=sscanf(line2,"%c %i",&change,&num);
	SCMDCHECK(itct==2,"cannot read change or number arguments");
  SCMDCHECK(line2=strnword(line2,3),"missing conditional command");

  if(cmd->i1==0) {
    cmd->i1=1;
    cmd->i2=molcount(sim,er,index,ms,NULL,-1); }
  else {
    count=molcount(sim,er,index,ms,NULL,-1);
    diff=count-cmd->i2;
    cmd->i2=count;
    if((change=='>' && diff>num) || (change=='<' && diff<num) || (change=='=' && diff==num) || (change=='!' && diff!=num))
      return docommand(sim,cmd,line2); }
	return CMDok; }


/**********************************************************/
/***************** observation commands *******************/
/**********************************************************/


enum CMDcode cmdwarnescapee(simptr sim,cmdptr cmd,char *line2) {
	int i,m,escape,dim,nmol,ll;
	enum MolecState ms;
	FILE *fptr;
	moleculeptr *mlist,mptr;
	double *pos,*posx,*via;
	wallptr *wlist;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	dim=sim->dim;
	wlist=sim->wlist;
	for(ll=0;ll<sim->mols->nlist;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident>0 && i<0 && (ms==MSall || mptr->mstate==ms)) || (mptr->ident==i && (ms==MSall || mptr->mstate==ms))) {
				pos=mptr->pos;
				escape=!posinsystem(sim,pos);
				if(escape) {
					posx=mptr->posx;
					escape=!posinsystem(sim,posx);
					if(!escape) {
						via=mptr->via;
						if(dim==1) scmdfprintf(cmd->cmds,fptr,"New escapee: %g #%li %g to %g via %g\n",sim->time,mptr->serno,posx[0],pos[0],via[0]);
						else if(dim==2) scmdfprintf(cmd->cmds,fptr,"New escapee: %g #%li (%g,%g) to (%g,%g) via (%g,%g)\n",sim->time,mptr->serno,posx[0],posx[1],pos[0],pos[1],via[0],via[1]);
						else scmdfprintf(cmd->cmds,fptr,"New escapee: %g #%li (%g,%g,%g) to (%g,%g,%g) via (%g,%g,%g)\n",sim->time,mptr->serno,posx[0],posx[1],posx[2],pos[0],pos[1],pos[2],via[0],via[1],via[2]); }}}}}
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdecho(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	char *termqt,str[STRCHAR];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	line2=strnword(line2,2);
	SCMDCHECK(line2=strchr(line2,'"'),"no starting quote on string");
	strncpy(str,line2+1,STRCHAR-1);
	str[STRCHAR-1]='\0';
	SCMDCHECK(termqt=strchr(str,'"'),"no terminal quote on string");
	*termqt='\0';
	strbslash2escseq(str);
	scmdfprintf(cmd->cmds,fptr,"%s",str);
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountheader(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	int i;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	SCMDCHECK(sim->mols,"molecules are undefined");
	scmdfprintf(cmd->cmds,fptr,"time");
	for(i=1;i<sim->mols->nspecies;i++) scmdfprintf(cmd->cmds,fptr," %s",sim->mols->spname[i]);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcount(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	int ll,m,*ct,i,nspecies,*ctlat,ilat;
	latticeptr lat;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(cmd->i1!=-1,"error on setup");					// failed before, don't try again
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	SCMDCHECK(sim->mols,"molecules are undefined");

	nspecies=sim->mols->nspecies;
	if(cmd->i1!=nspecies) {														// allocate counter if required
		cmdv1free(cmd);
		cmd->i1=nspecies;
		cmd->freefn=&cmdv1v2free;
		cmd->v1=calloc(nspecies,sizeof(int));
		if(!cmd->v1) {cmd->i1=-1;return CMDwarn;} }

	ct=(int*)cmd->v1;
	for(i=0;i<nspecies;i++) ct[i]=0;
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			i=sim->mols->live[ll][m]->ident;
			if(i>0) ct[i]++; }

	if(sim->latticess) {
    if(cmd->i2!=nspecies) {
      free(cmd->v2);
      cmd->i2=nspecies;
      cmd->v2=calloc(nspecies,sizeof(int));
      if(!cmd->v2) {cmd->i2=-1;return CMDwarn;}}

    ctlat=(int*)cmd->v2;
		for(ilat=0;ilat<sim->latticess->nlattice;++ilat) {
			lat=sim->latticess->latticelist[ilat];
			for(i=1;i<nspecies;i++) ctlat[i]=0;
			if(lat->type==LATTICEnsv) {
				NSV_CALL(nsv_molcount(lat->nsv,ctlat)); }
			else if(lat->type==LATTICEpde) {
				//not implemented
			}
			for(i=1;i<nspecies;i++) {
				ct[i]+=ctlat[i]; }}}

	scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
	for(i=1;i<nspecies;i++) scmdfprintf(cmd->cmds,fptr," %i",ct[i]);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountinbox(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	int ll,m,*ct,d,dim,itct,i,nspecies;
	double low[3],high[3];
	moleculeptr mptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(cmd->i1!=-1,"error on setup");					// failed before, don't try again
	SCMDCHECK(sim->mols,"molecules are undefined");
	dim=sim->dim;
	for(d=0;d<dim;d++) {
		SCMDCHECK(line2,"missing argument");
		itct=sscanf(line2,"%lg %lg",&low[d],&high[d]);
		SCMDCHECK(itct==2,"read failure");
		line2=strnword(line2,3); }
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	nspecies=sim->mols->nspecies;
	if(cmd->i1!=nspecies) {														// allocate counter if required
		cmdv1free(cmd);
		cmd->i1=nspecies;
		cmd->freefn=&cmdv1free;
		cmd->v1=calloc(nspecies,sizeof(int));
		if(!cmd->v1) {cmd->i1=-1;return CMDwarn;} }
	
	ct=(int*)cmd->v1;
	for(i=0;i<nspecies;i++) ct[i]=0;
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			for(d=0;d<dim;d++)
				if(mptr->pos[d]<low[d] || mptr->pos[d]>high[d]) d=dim+1;
			if(d==dim && mptr->ident>0) ct[mptr->ident]++; }
	scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
	for(i=1;i<nspecies;i++) scmdfprintf(cmd->cmds,fptr," %i",ct[i]);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountincmpt(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	char nm[STRCHAR];
	compartptr cmpt;
	compartssptr cmptss;
	int ll,m,*ct,itct,c,i,nspecies;
	moleculeptr mptr;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(cmd->i1!=-1,"error on setup");					// failed before, don't try again
	cmptss=sim->cmptss;
	SCMDCHECK(cmptss,"no compartments defined");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s",nm);
	SCMDCHECK(itct==1,"cannot read argument");
	c=stringfind(cmptss->cnames,cmptss->ncmpt,nm);
	SCMDCHECK(c>=0,"compartment name not recognized");
	cmpt=cmptss->cmptlist[c];
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	
	nspecies=sim->mols->nspecies;
	if(cmd->i1!=nspecies) {														// allocate counter if required
		cmdv1free(cmd);
		cmd->i1=nspecies;
		cmd->freefn=&cmdv1free;
		cmd->v1=calloc(nspecies,sizeof(int));
		if(!cmd->v1) {cmd->i1=-1;return CMDwarn;} }
	
	ct=(int*)cmd->v1;
	for(i=0;i<nspecies;i++) ct[i]=0;
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident>0 && posincompart(sim,mptr->pos,cmpt) && mptr->mstate==MSsoln) ct[mptr->ident]++; }
	scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
	for(i=1;i<nspecies;i++) scmdfprintf(cmd->cmds,fptr," %i",ct[i]);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountincmpts(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	char nm[STRCHAR];
	compartptr cmptlist[16];
	compartssptr cmptss;
	int ll,m,*ct,itct,c,i,ic,ncmpt,nspecies;
	moleculeptr mptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(cmd->i1!=-1,"error on setup");					// failed before, don't try again
	cmptss=sim->cmptss;
	SCMDCHECK(cmptss,"no compartments defined");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(line2,"missing argument");
	ncmpt=wordcount(line2)-1;
	SCMDCHECK(ncmpt>=1,"no compartment or no output file listed");
	for(ic=0;ic<ncmpt;ic++) {
		itct=sscanf(line2,"%s",nm);
		SCMDCHECK(itct==1,"cannot read compartment name");
		c=stringfind(cmptss->cnames,cmptss->ncmpt,nm);
		SCMDCHECK(c>=0,"compartment name not recognized");
		cmptlist[ic]=cmptss->cmptlist[c];
		line2=strnword(line2,2);
		SCMDCHECK(line2,"missing argument"); }
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	nspecies=sim->mols->nspecies;
	if(cmd->i1!=nspecies) {														// allocate counter if required
		cmdv1free(cmd);
		cmd->i1=nspecies;
		cmd->freefn=&cmdv1free;
		cmd->v1=calloc(nspecies*ncmpt,sizeof(int));
		if(!cmd->v1) {cmd->i1=-1;return CMDwarn;} }

	ct=(int*)cmd->v1;
	for(i=0;i<nspecies*ncmpt;i++) ct[i]=0;
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident>0 && mptr->mstate==MSsoln)
				for(ic=0;ic<ncmpt;ic++)
					if(posincompart(sim,mptr->pos,cmptlist[ic])) ct[ic*nspecies+mptr->ident]++; }

	scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
	for(i=1;i<nspecies*ncmpt;i++) 
		if(i%nspecies!=0) scmdfprintf(cmd->cmds,fptr," %i",ct[i]);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountincmpt2(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	char nm[STRCHAR],state[STRCHAR];
	compartptr cmpt;
	compartssptr cmptss;
	int ll,m,*ct,itct,c,i,nspecies;
	moleculeptr mptr;
	enum MolecState ms;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(cmd->i1!=-1,"error on setup");					// failed before, don't try again
	cmptss=sim->cmptss;
	SCMDCHECK(cmptss,"no compartments defined");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s %s",nm,state);
	SCMDCHECK(itct==2,"cannot read arguments");
	c=stringfind(cmptss->cnames,cmptss->ncmpt,nm);
	SCMDCHECK(c>=0,"compartment name not recognized");
	ms=molstring2ms(state);
	SCMDCHECK(ms!=MSnone,"molecule state not recognized");
	SCMDCHECK(ms!=MSbsoln,"bsoln molecule state not permitted");
	cmpt=cmptss->cmptlist[c];
	line2=strnword(line2,3);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	nspecies=sim->mols->nspecies;
	if(cmd->i1!=nspecies) {														// allocate counter if required
		cmdv1free(cmd);
		cmd->i1=nspecies;
		cmd->freefn=&cmdv1free;
		cmd->v1=calloc(nspecies,sizeof(int));
		if(!cmd->v1) {cmd->i1=-1;return CMDwarn;} }

	ct=(int*)cmd->v1;
	for(i=0;i<nspecies;i++) ct[i]=0;
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident>0 && posincompart(sim,mptr->pos,cmpt))
				if(ms==MSall || mptr->mstate==ms) ct[mptr->ident]++; }
	scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
	for(i=1;i<nspecies;i++) scmdfprintf(cmd->cmds,fptr," %i",ct[i]);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountonsurf(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	char nm[STRCHAR];
	surfaceptr srf;
	surfacessptr srfss;
	int ll,m,*ct,itct,found,i,nspecies;
	moleculeptr mptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(cmd->i1!=-1,"error on setup");					// failed before, don't try again
	srfss=sim->srfss;
	SCMDCHECK(srfss,"no surfaces defined");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s",nm);
	SCMDCHECK(itct==1,"cannot read argument");
	found=srfss->snametosrf->contains(srfss->snametosrf, nm);
	SCMDCHECK(found,"surface name '%s' not recognized",nm);
	srf=(surfaceptr)srfss->snametosrf->getFrom(srfss->snametosrf, nm);
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	nspecies=sim->mols->nspecies;
	if(cmd->i1!=nspecies) {														// allocate counter if required
		cmdv1free(cmd);
		cmd->i1=nspecies;
		cmd->freefn=&cmdv1free;
		cmd->v1=calloc(nspecies,sizeof(int));
		if(!cmd->v1) {cmd->i1=-1;return CMDwarn;} }

	ct=(int*)cmd->v1;
	for(i=0;i<nspecies;i++) ct[i]=0;
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident>0 && mptr->mstate!=MSsoln && mptr->pnl->srf==srf) ct[mptr->ident]++; }
	scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
	for(i=1;i<nspecies;i++) scmdfprintf(cmd->cmds,fptr," %i",ct[i]);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountspace(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	int dim,i,itct,axis,nbin,ax2,d,*ct,bin,ll,lllo,llhi,nmol,m,average,*ctlat,ilat;
	enum MolecState ms;
	double low[DIMMAX],high[DIMMAX],scale;
	char axisstr[STRCHAR];
	moleculeptr *mlist,mptr;
	latticeptr lat;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(cmd->i1!=-1,"error on setup");					// failed before, don't try again

	dim=sim->dim;
	SCMDCHECK(line2,"missing arguments");
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing arguments");
	itct=sscanf(line2,"%s",axisstr);
	SCMDCHECK(itct==1,"cannot read axis value");
	if(!strcmp(axisstr,"0") || !strcmp(axisstr,"x")) axis=0;
	else if(!strcmp(axisstr,"1") || !strcmp(axisstr,"y")) axis=1;
	else if(!strcmp(axisstr,"2") || !strcmp(axisstr,"z")) axis=2;
	else axis=3;
	SCMDCHECK(axis>=0 && axis<dim,"illegal axis value");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing arguments");
	itct=sscanf(line2,"%lf %lf %i",&low[axis],&high[axis],&nbin);
	SCMDCHECK(itct==3,"cannot read arguments: low high bins");
	SCMDCHECK(low[axis]<high[axis],"low value needs to be less than high value");
	SCMDCHECK(nbin>0,"bins value needs to be > 0");
	line2=strnword(line2,4);
	ax2=0;
	for(d=0;d<dim-1;d++) {
		if(ax2==axis) ax2++;
		SCMDCHECK(line2,"missing position arguments");
		itct=sscanf(line2,"%lf %lf",&low[ax2],&high[ax2]);
		SCMDCHECK(itct==2,"cannot read (or insufficient) position arguments");
		SCMDCHECK(low[ax2]<=high[ax2],"low value needs to be less than or equal to high value");
		line2=strnword(line2,3);
		ax2++; }
	SCMDCHECK(line2,"missing arguments");
	itct=sscanf(line2,"%i",&average);
	SCMDCHECK(itct==1,"cannot read average number");
	SCMDCHECK(average>=0,"illegal average value");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	if(cmd->i1!=nbin) {														// allocate counter if required
		cmdv1free(cmd);
		cmd->i1=nbin;
		cmd->freefn=&cmdv1v2free;
		cmd->v1=calloc(nbin,sizeof(int));
		if(!cmd->v1) {cmd->i1=-1;return CMDwarn;} }

	ct=(int*)cmd->v1;
	if(average<=1 || cmd->invoke%average==1)
		for(bin=0;bin<nbin;bin++) ct[bin]=0;
	scale=(double)nbin/(high[axis]-low[axis]);

	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident>0 && i<0 && (ms==MSall || mptr->mstate==ms)) || (mptr->ident==i && (ms==MSall || mptr->mstate==ms))) {
				for(d=0;d<dim;d++)
					if(mptr->pos[d]<=low[d] || mptr->pos[d]>=high[d]) d=dim+1;
				if(d==dim) {
					bin=(int)floor(scale*(mptr->pos[axis]-low[axis]));
					if(bin==nbin) bin--;
					ct[bin]++; }}}}

	if(sim->latticess) {
    if(cmd->i2!=nbin) {
      free(cmd->v2);
      cmd->i2=nbin;
      cmd->v2=calloc(nbin,sizeof(int));
      if(!cmd->v2) {cmd->i2=-1;return CMDwarn;} }

    ctlat=(int*)cmd->v2;
		for(ilat=0;ilat<sim->latticess->nlattice;++ilat) {
			lat=sim->latticess->latticelist[ilat];
			if(lat->type==LATTICEnsv) {
				NSV_CALL(nsv_molcountspace(lat->nsv,i,low,high,dim,nbin,axis,ctlat)); }
			else if(lat->type==LATTICEpde) {
				//not implemented
			}
			for(bin=0;bin<nbin;++bin) {
				ct[bin]+=ctlat[bin]; }}}

	if(average<=1) {
		scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
		for(bin=0;bin<nbin;bin++) scmdfprintf(cmd->cmds,fptr," %i",ct[bin]);
		scmdfprintf(cmd->cmds,fptr,"\n"); }
	else if(cmd->invoke%average==0) {
		scmdfprintf(cmd->cmds,fptr,"%g",sim->time);
		for(bin=0;bin<nbin;bin++) scmdfprintf(cmd->cmds,fptr," %g",(double)(ct[bin])/(double)average);
		scmdfprintf(cmd->cmds,fptr,"\n"); }
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolcountspecies(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	int er,*index,count;
	enum MolecState ms;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	er=molstring2index1(sim,line2,&ms,&index);
	SCMDCHECK(er!=-1,"species is missing or cannot be read");
	SCMDCHECK(er!=-2,"mismatched or improper parentheses around molecule state");
	SCMDCHECK(er!=-3,"cannot read molecule state value");
	SCMDCHECK(er!=-4,"molecule name not recognized");
	SCMDCHECK(er!=-7,"error allocating memory");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	
	count=molcount(sim,er,index,ms,NULL,-1);
	scmdfprintf(cmd->cmds,fptr,"%g %i\n",sim->time,count);
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmollistsize(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	int ll,itct;
	char listname[STRCHAR];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	itct=sscanf(line2,"%s",listname);
	SCMDCHECK(itct==1,"cannot read molecule list name");
	SCMDCHECK(sim->mols && sim->mols->nlist>0,"no molecule lists defined");
	ll=stringfind(sim->mols->listname,sim->mols->nlist,listname);
	SCMDCHECK(ll>=0,"molecule list name not recognized");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	scmdfprintf(cmd->cmds,fptr,"%g %i\n",sim->time,sim->mols->nl[ll]);
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdlistmols(simptr sim,cmdptr cmd,char *line2) {
	int m,ll,d;
	static char string[STRCHAR];
	moleculeptr mptr;
	FILE *fptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(sim->mols,"molecules are undefined");
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident>0) {
				scmdfprintf(cmd->cmds,fptr,"%s(%s) ",sim->mols->spname[mptr->ident],molms2string(mptr->mstate,string));
				for(d=0;d<sim->dim;d++)
					scmdfprintf(cmd->cmds,fptr,"%g%s",mptr->pos[d],d<sim->dim-1?" ":"\n"); }}
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdlistmols2(simptr sim,cmdptr cmd,char *line2) {
	int m,ll,invk,d;
	moleculeptr mptr;
	FILE *fptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	invk=cmd?cmd->invoke:0;
	for(ll=0;ll<sim->mols->nlist;ll++)
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident>0) {
				scmdfprintf(cmd->cmds,fptr,"%i %i %i ",invk,mptr->ident,mptr->mstate);
				for(d=0;d<sim->dim;d++)
					scmdfprintf(cmd->cmds,fptr,"%g%s",mptr->pos[d],d<sim->dim-1?" ":"\n"); }}
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdlistmols3(simptr sim,cmdptr cmd,char *line2) {
	int i,m,ll,dim,invk,lllo,llhi,nmol,d;
	moleculeptr *mlist,mptr;
	FILE *fptr;
	enum MolecState ms;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	invk=cmd?cmd->invoke:0;
	dim=sim->dim;
	
	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident>0 && i<0 && (ms==MSall || mptr->mstate==ms)) || (mptr->ident==i && (ms==MSall || mptr->mstate==ms))) {
				scmdfprintf(cmd->cmds,fptr,"%i %i %i ",invk,mptr->ident,mptr->mstate);
				for(d=0;d<sim->dim;d++)
					scmdfprintf(cmd->cmds,fptr,"%g%s",mptr->pos[d],d<sim->dim-1?" ":"\n"); }}}
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdlistmols4(simptr sim,cmdptr cmd,char *line2) {
	int i,m,ll,dim,invk,lllo,llhi,nmol,d;
	moleculeptr *mlist,mptr;
	FILE *fptr;
	enum MolecState ms;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	invk=cmd?cmd->invoke:0;
	dim=sim->dim;
	
	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident>0 && i<0 && (ms==MSall || mptr->mstate==ms)) || (mptr->ident==i && (ms==MSall || mptr->mstate==ms))) {
				scmdfprintf(cmd->cmds,fptr,"%i %i %i ",invk,mptr->ident,mptr->mstate);
				for(d=0;d<sim->dim;d++)
					scmdfprintf(cmd->cmds,fptr,"%g%s",mptr->pos[d]+mptr->posoffset[d],d<sim->dim-1?" ":"\n"); }}}
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdlistmolscmpt(simptr sim,cmdptr cmd,char *line2) {
	int i,c,m,ll,dim,invk,lllo,llhi,nmol,itct,d;
	moleculeptr *mlist,mptr;
	FILE *fptr;
	enum MolecState ms;
	char cname[STRCHAR];
	compartssptr cmptss;
	compartptr cmpt;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing compartment name");
	itct=sscanf(line2,"%s",cname);
	SCMDCHECK(itct==1,"cannot read compartment name");
	cmptss=sim->cmptss;
	SCMDCHECK(cmptss,"no compartments defined");
	c=stringfind(cmptss->cnames,cmptss->ncmpt,cname);
	SCMDCHECK(c>=0,"compartment name not recognized");
	cmpt=cmptss->cmptlist[c];
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	invk=cmd?cmd->invoke:0;
	dim=sim->dim;

	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident>0 && i<0 && (ms==MSall || mptr->mstate==ms)) || (mptr->ident==i && (ms==MSall || mptr->mstate==ms))) {
				if(posincompart(sim,mptr->pos,cmpt)) {
					scmdfprintf(cmd->cmds,fptr,"%i %i %i ",invk,mptr->ident,mptr->mstate);
					for(d=0;d<sim->dim;d++)
						scmdfprintf(cmd->cmds,fptr,"%g%s",mptr->pos[d],d<sim->dim-1?" ":"\n"); }}}}
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolpos(simptr sim,cmdptr cmd,char *line2) {
	int i,d,m,ll,dim,lllo,llhi,nmol;
	moleculeptr *mlist,mptr;
	FILE *fptr;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	dim=sim->dim;

	scmdfprintf(cmd->cmds,fptr,"%g ",sim->time);
	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident>0 && i<0 && (ms==MSall || mptr->mstate==ms)) || (mptr->ident==i && (ms==MSall || mptr->mstate==ms))) {
				for(d=0;d<dim;d++)
					scmdfprintf(cmd->cmds,fptr,"%g ",mptr->pos[d]); }}}
		scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdtrackmol(simptr sim,cmdptr cmd,char *line2) {
	int itct,ll,m,d,c;
	long int serno;
	FILE *fptr;
	molssptr mols;
	moleculeptr mptr;
	char string[STRCHAR];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	itct=sscanf(line2,"%li",&serno);
	SCMDCHECK(itct==1,"cannot read molecule serial number");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	if(sim->mols) {
		mols=sim->mols;
		for(ll=0;ll<mols->nlist;ll++)
			for(m=0;m<mols->nl[ll];m++)
				if(mols->live[ll][m]->serno==serno) {
					mptr=mols->live[ll][m];
					scmdfprintf(cmd->cmds,fptr,"%g %s %s",sim->time,mols->spname[mptr->ident],molms2string(mptr->mstate,string));
					for(d=0;d<sim->dim;d++)
						scmdfprintf(cmd->cmds,fptr," %g",mptr->pos[d]);
					if(sim->cmptss)
						for(c=0;c<sim->cmptss->ncmpt;c++) {
							if(posincompart(sim,mptr->pos,sim->cmptss->cmptlist[c]))
								scmdfprintf(cmd->cmds,fptr," in");
							else
								scmdfprintf(cmd->cmds,fptr," out"); }
					scmdfprintf(cmd->cmds,fptr,"\n");
					m=mols->nl[ll];
					ll=mols->nlist; }}
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmolmoments(simptr sim,cmdptr cmd,char *line2) {
	int i,ll,m,ctr,dim,d,d2,nmol;
	double v1[DIMMAX],m1[DIMMAX*DIMMAX];
	FILE *fptr;
	moleculeptr *mlist,mptr;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>=0,"cannot read molecule and/or state name; 'all' is not permitted");
	if(ms==MSall) ms=MSsoln;
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	dim=sim->dim;

	ll=sim->mols->listlookup[i][ms];
	mlist=sim->mols->live[ll];
	nmol=sim->mols->nl[ll];
	ctr=0;
	for(d=0;d<dim;d++) v1[d]=0;
	for(d=0;d<dim*dim;d++) m1[d]=0;
	for(m=0;m<nmol;m++) {
		mptr=mlist[m];
		if(mptr->ident==i && mptr->mstate==ms) {
			ctr++;
			for(d=0;d<dim;d++) v1[d]+=mlist[m]->pos[d]; }}
	for(d=0;d<dim;d++) v1[d]/=1.0*ctr;
	for(m=0;m<nmol;m++) {
		mptr=mlist[m];
		if(mptr->ident==i && mptr->mstate==ms) {
			for(d=0;d<dim;d++)
				for(d2=0;d2<dim;d2++)
					m1[d*dim+d2]+=(mlist[m]->pos[d]-v1[d])*(mlist[m]->pos[d2]-v1[d2]); }}
	scmdfprintf(cmd->cmds,fptr,"%g %i",sim->time,ctr);
	for(d=0;d<dim;d++) scmdfprintf(cmd->cmds,fptr," %g",v1[d]);
	for(d=0;d<dim;d++)
		for(d2=0;d2<dim;d2++)
			scmdfprintf(cmd->cmds,fptr," %g",m1[d*dim+d2]/ctr);
	scmdfprintf(cmd->cmds,fptr,"\n");
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdsavesim(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
	if(line2) {
		strcutwhite(line2,2); }

	scmdfprintf(cmd->cmds,fptr,"# Configuration file automatically created by Smoldyn\n\n");
	writesim(sim,fptr);
	writegraphss(sim,fptr);
	writemols(sim,fptr);
	writewalls(sim,fptr);
	writesurfaces(sim,fptr);
	writecomparts(sim,fptr);
	writereactions(sim,fptr);
	writelattices(sim,fptr);
	scmdwritecommands(sim->cmds,fptr,line2);
	writemolecules(sim,fptr);
	scmdfprintf(cmd->cmds,fptr,"\nend_file\n");
	scmdflush(fptr);
	return CMDok; }


void cmdmeansqrdispfree(cmdptr cmd) {
	int j;

	if(cmd->v2 && cmd->i1)
		for(j=0;j<cmd->i1;j++)
			free(((double**)(cmd->v2))[j]);
	if(cmd->v2) free(cmd->v2);
	if(cmd->v1) free(cmd->v1);
	return; }


enum CMDcode cmdmeansqrdisp(simptr sim,cmdptr cmd,char *line2) {
	static char dimstr[STRCHAR];
	int i,j,d,itct,ll,dim,ctr,m,msddim,nmol;
	FILE *fptr;
	moleculeptr *mlist;
	double r2,sum,sum4,diff,*pos,**v2;
	long int *v1;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>=0,"cannot read molecule and/or state name; 'all' is not permitted");
	if(ms==MSall) ms=MSsoln;
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing dimension information");
	itct=sscanf(line2,"%s",dimstr);
	SCMDCHECK(itct==1,"cannot read dimension information");
	msddim=1;
	if(!strcmp(dimstr,"all")) msddim=-1;
	else if(!strcmp(dimstr,"0") || !strcmp(dimstr,"x")) msddim=0;
	else if(!strcmp(dimstr,"1") || !strcmp(dimstr,"y")) msddim=1;
	else if(!strcmp(dimstr,"2") || !strcmp(dimstr,"z")) msddim=2;
	else msddim=3;
	SCMDCHECK(msddim>=0 && msddim<sim->dim,"invalid dimension value");
	line2=strnword(line2,2);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	SCMDCHECK(cmd->i2!=2,"error on setup");					// failed before, don't try again

	ll=sim->mols->listlookup[i][ms];
	dim=sim->dim;
	mlist=sim->mols->live[ll];
	nmol=sim->mols->nl[ll];

	if(!cmd->i2) {										// test for first run
		cmd->i2=1;											// if first run, set up data structures
		ctr=0;
		for(m=0;m<nmol;m++)
			if(mlist[m]->ident==i && mlist[m]->mstate==ms) ctr++;
		cmd->i1=ctr;										// size of arrays
		SCMDCHECK(ctr>0,"no molecules to track");
		cmd->freefn=&cmdmeansqrdispfree;
		cmd->v1=calloc(ctr,sizeof(long int));	// v1 is serial numbers
		if(!cmd->v1) {cmd->i2=2;return CMDwarn;}
		for(j=0;j<ctr;j++) ((long int*)cmd->v1)[j]=0;
		cmd->v2=calloc(ctr,sizeof(double**));	// v2 is positions
		if(!cmd->v2) {cmd->i2=2;return CMDwarn;}
		for(j=0;j<ctr;j++) ((double**)cmd->v2)[j]=NULL;
		for(j=0;j<ctr;j++) {
			((double**)cmd->v2)[j]=(double*)calloc(dim,sizeof(double));
			if(!((double**)cmd->v2)[j]) {cmd->i2=2;return CMDwarn;}
			for(d=0;d<dim;d++) ((double**)cmd->v2)[j][d]=0; }
		ctr=0;
		for(m=0;m<nmol;m++) {
			if(mlist[m]->ident==i && mlist[m]->mstate==ms) {
				((long int*)cmd->v1)[ctr]=mlist[m]->serno;
				for(d=0;d<dim;d++)
					((double**)cmd->v2)[ctr][d]=mlist[m]->posoffset[d]+mlist[m]->pos[d];
				ctr++; }}
		sortVliv((long int*)cmd->v1,(void**)cmd->v2,cmd->i1); }

	ctr=0;														// start of code that is run every invocation
	sum=0;
	sum4=0;
	v1=(long int*)cmd->v1;
	v2=(double**)cmd->v2;
	for(m=0;m<nmol;m++)
		if(mlist[m]->ident==i && mlist[m]->mstate==ms) {
			j=locateVli(v1,mlist[m]->serno,cmd->i1);
			if(j>=0) {
				pos=mlist[m]->pos;
				ctr++;
				if(msddim<0) {
					r2=0;
					for(d=0;d<dim;d++) {
						diff=mlist[m]->posoffset[d]+pos[d]-v2[j][d];
						r2+=diff*diff; }
					sum+=r2;
					sum4+=r2*r2; }
				else {
					diff=mlist[m]->posoffset[msddim]+pos[msddim]-v2[j][msddim];
					sum+=diff*diff;
					sum4+=diff*diff*diff*diff; }}}
	scmdfprintf(cmd->cmds,fptr,"%g %g %g\n",sim->time,sum/ctr,sum4/ctr);

	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmeansqrdisp2(simptr sim,cmdptr cmd,char *line2) {
	static char dimstr[STRCHAR];
	int i,j,d,itct,ll,dim,ctr,m,msddim,nmol,maxmoment,maxmol,mom;
	FILE *fptr;
	moleculeptr *mlist;
	static double sum[17];
	double r2,diff,**v2,*dblptr;
	long int *v1;
	enum MolecState ms;
	char startchar,reportchar;
  
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>=0,"cannot read molecule and/or state name; 'all' is not permitted");
	if(ms==MSall) ms=MSsoln;
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing dimension information");
	itct=sscanf(line2,"%s",dimstr);
	SCMDCHECK(itct==1,"cannot read dimension information");
	if(!strcmp(dimstr,"all")) msddim=-1;
	else if(!strcmp(dimstr,"0") || !strcmp(dimstr,"x")) msddim=0;
	else if(!strcmp(dimstr,"1") || !strcmp(dimstr,"y")) msddim=1;
	else if(!strcmp(dimstr,"2") || !strcmp(dimstr,"z")) msddim=2;
	else msddim=3;
	SCMDCHECK(msddim>=0 && msddim<sim->dim,"invalid dimension value");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"insufficient arguments");
	itct=sscanf(line2,"%c %c %i %i",&startchar,&reportchar,&maxmol,&maxmoment);
	SCMDCHECK(itct==4,"cannot read start, report, max_mol, or max_moment information");
	SCMDCHECK(maxmol>0,"max_mol has to be at least 1");
	SCMDCHECK(maxmoment>0,"maxmoment has to be at least 1");
	SCMDCHECK(maxmoment<=16,"max_moment cannot exceed 16");
	line2=strnword(line2,5);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
  
	SCMDCHECK(cmd->i2!=2,"error on setup");					// failed before, don't try again
  
	ll=sim->mols->listlookup[i][ms];
	dim=sim->dim;
	mlist=sim->mols->live[ll];
	nmol=sim->mols->nl[ll];
  
	if(!cmd->i2) {										// test for first run
		cmd->i2=1;											// if first run, set up data structures
		cmd->i1=maxmol;
		cmd->i3=0;
		cmd->freefn=&cmdmeansqrdispfree;
		cmd->v1=calloc(maxmol,sizeof(long int));	// v1 is serial numbers
		if(!cmd->v1) {cmd->i2=2;return CMDwarn;}
		v1=(long int*)cmd->v1;
		for(j=0;j<maxmol;j++) v1[j]=0;
		cmd->v2=calloc(maxmol,sizeof(double**));	// v2 is positions
		if(!cmd->v2) {cmd->i2=2;return CMDwarn;}
		v2=(double**)cmd->v2;
		for(j=0;j<maxmol;j++) v2[j]=NULL;
		for(j=0;j<maxmol;j++) {
			v2[j]=(double*)calloc(2*dim+1,sizeof(double));
			if(!v2[j]) {cmd->i2=2;return CMDwarn;}
			for(d=0;d<2*dim+1;d++) v2[j][d]=0; }
		ctr=0;
		for(m=0;m<nmol;m++) {
			if(mlist[m]->ident==i && mlist[m]->mstate==ms) {
				SCMDCHECK(ctr<maxmol,"insufficient allocated space");
				v1[ctr]=mlist[m]->serno;
				if(startchar=='c') v2[ctr][0]=0;
				else v2[ctr][0]=2.0;
				for(d=0;d<dim;d++)
					v2[ctr][1+d]=v2[ctr][dim+1+d]=mlist[m]->posoffset[d]+mlist[m]->pos[d];
				ctr++; }}
		cmd->i3=ctr;
		if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3); }
  
	v1=(long int*)cmd->v1;						// start of code that is run every invocation
	v2=(double**)cmd->v2;
  
	for(m=0;m<nmol;m++) {							// update tracking information for all tracked molecules
		if(mlist[m]->ident==i && mlist[m]->mstate==ms) {
			j=locateVli(v1,mlist[m]->serno,cmd->i3);
			if(j>=0) {										// molecule was found
				v2[j][0]+=1.0;
				if(v2[j][0]==3.0) {					// molecule is being tracked and exists, so record current positions
					for(d=0;d<dim;d++)
						v2[j][dim+1+d]=mlist[m]->posoffset[d]+mlist[m]->pos[d]; }}
			else if(startchar!='i') {			// molecule was not found but should be tracked
				if(cmd->i3==cmd->i1) {SCMDCHECK(0,"not enough allocated space");}
				j=cmd->i3++;					// find empty spot
				v1[j]=mlist[m]->serno;
				v2[j][0]=3.0;
				for(d=0;d<dim;d++)
					v2[j][1+d]=v2[j][dim+1+d]=mlist[m]->posoffset[d]+mlist[m]->pos[d]; }}}
	if(startchar!='i')								// resort lists if appropriate
		if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3);
  
	for(mom=0;mom<=maxmoment;mom++)
		sum[mom]=0;
	for(j=0;j<cmd->i3;j++) {					// find moments of all reported results
		if((reportchar=='e' && v2[j][0]==3.0) || (reportchar=='r' && v2[j][0]==2.0)) { // molecule should be recorded
			if(msddim<0) {
				r2=0;
				for(d=0;d<dim;d++) {
					diff=v2[j][dim+1+d]-v2[j][1+d];
					r2+=diff*diff; }
				r2=sqrt(r2);
				for(mom=0;mom<=maxmoment;mom++)
					sum[mom]+=pow(r2,mom); }
			else {
				diff=v2[j][dim+1+msddim]-v2[j][1+msddim];
				for(mom=0;mom<=maxmoment;mom++)
					sum[mom]+=pow(diff,mom); }}}
  
	if(sum[0]>0) {
		scmdfprintf(cmd->cmds,fptr,"%g %g",sim->time,sum[0]);					// display results
		for(mom=1;mom<=maxmoment;mom++) {
			scmdfprintf(cmd->cmds,fptr," %g",sum[mom]/sum[0]); }
		scmdfprintf(cmd->cmds,fptr,"\n"); }
  
	for(j=0;j<cmd->i3;j++) {							// stop tracking expired molecules
		if(v2[j][0]==0 || v2[j][0]==2.0) {
			v1[j]=v1[cmd->i3-1];
			v1[cmd->i3-1]=0;
			dblptr=v2[j];
			v2[j]=v2[cmd->i3-1];
			v2[cmd->i3-1]=dblptr;
			v2[cmd->i3-1][0]=0;
			j--;
			cmd->i3--; }
		else
			v2[j][0]-=1.0; }
	if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3);
  
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdmeansqrdisp3(simptr sim,cmdptr cmd,char *line2) {
	static char dimstr[STRCHAR];
	int i,j,d,itct,ll,dim,ctr,m,msddim,nmol,maxmol;
	FILE *fptr;
	moleculeptr *mlist;
	double sum,wgt;
	double r2,diff,**v2,*dblptr,change;
	long int *v1;
	enum MolecState ms;
	char startchar,reportchar;
  
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>=0,"cannot read molecule and/or state name; 'all' is not permitted for species");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing dimension information");
	itct=sscanf(line2,"%s",dimstr);
	SCMDCHECK(itct==1,"cannot read dimension information");
	if(!strcmp(dimstr,"all")) msddim=-1;
	else if(!strcmp(dimstr,"0") || !strcmp(dimstr,"x")) msddim=0;
	else if(!strcmp(dimstr,"1") || !strcmp(dimstr,"y")) msddim=1;
	else if(!strcmp(dimstr,"2") || !strcmp(dimstr,"z")) msddim=2;
	else msddim=3;
	SCMDCHECK(msddim>=0 && msddim<sim->dim,"invalid dimension value");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"insufficient arguments");
	itct=sscanf(line2,"%c %c %i %lg",&startchar,&reportchar,&maxmol,&change);
	SCMDCHECK(itct==4,"cannot read start, report, max_mol, or change information");
	SCMDCHECK(maxmol>0,"max_mol has to be at least 1");
	line2=strnword(line2,5);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
  line2=strnword(line2,2);
  SCMDCHECK(change<=0 || line2,"missing task to be accomplished if change is small");
  
	SCMDCHECK(cmd->i2!=2,"error on setup");					// failed before, don't try again
  
	if(ms==MSall)
    ll=sim->mols->listlookup[i][MSsoln];
  else
    ll=sim->mols->listlookup[i][ms];
	dim=sim->dim;
	mlist=sim->mols->live[ll];
	nmol=sim->mols->nl[ll];
  
	if(!cmd->i2) {										// test for first run
		cmd->i2=1;											// if first run, set up data structures
		cmd->i1=maxmol;
		cmd->i3=0;
    cmd->f1=-1;
		cmd->freefn=&cmdmeansqrdispfree;
		cmd->v1=calloc(maxmol,sizeof(long int));	// v1 is serial numbers
		if(!cmd->v1) {cmd->i2=2;return CMDwarn;}
		v1=(long int*)cmd->v1;
		for(j=0;j<maxmol;j++) v1[j]=0;
		cmd->v2=calloc(maxmol,sizeof(double**));	// v2 is positions
		if(!cmd->v2) {cmd->i2=2;return CMDwarn;}
		v2=(double**)cmd->v2;
		for(j=0;j<maxmol;j++) v2[j]=NULL;
		for(j=0;j<maxmol;j++) {
			v2[j]=(double*)calloc(2*dim+2,sizeof(double));
			if(!v2[j]) {cmd->i2=2;return CMDwarn;}
			for(d=0;d<2*dim+2;d++) v2[j][d]=0; }
		ctr=0;
		for(m=0;m<nmol;m++) {                    // record initial data
			if(mlist[m]->ident==i && (ms==MSall || mlist[m]->mstate==ms)) {
				SCMDCHECK(ctr<maxmol,"insufficient allocated space");
				v1[ctr]=mlist[m]->serno;
				if(startchar=='c') v2[ctr][0]=0;
				else v2[ctr][0]=2.0;
				for(d=0;d<dim;d++)
					v2[ctr][1+d]=v2[ctr][dim+1+d]=mlist[m]->posoffset[d]+mlist[m]->pos[d];
        v2[ctr][2*dim+1]=sim->time;
				ctr++; }}
		cmd->i3=ctr;
		if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3); }
  
	v1=(long int*)cmd->v1;						// start of code that is run every invocation
	v2=(double**)cmd->v2;
  
	for(m=0;m<nmol;m++) {							// update tracking information for all tracked molecules
		if(mlist[m]->ident==i && (ms==MSall || mlist[m]->mstate==ms)) {
			j=locateVli(v1,mlist[m]->serno,cmd->i3);
			if(j>=0) {										// molecule was found
				v2[j][0]+=1.0;
				if(v2[j][0]==3.0) {					// molecule is being tracked and exists, so record current positions
					for(d=0;d<dim;d++)
						v2[j][dim+1+d]=mlist[m]->posoffset[d]+mlist[m]->pos[d]; }}
			else if(startchar!='i') {			// molecule was not found but should be tracked
				if(cmd->i3==cmd->i1) {SCMDCHECK(0,"not enough allocated space");}
				j=cmd->i3++;					// find empty spot
				v1[j]=mlist[m]->serno;
				v2[j][0]=3.0;
				for(d=0;d<dim;d++)
					v2[j][1+d]=v2[j][dim+1+d]=mlist[m]->posoffset[d]+mlist[m]->pos[d];
        v2[j][2*dim+1]=sim->time; }}}
	if(startchar!='i')								// resort lists if appropriate
		if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3);
  
  sum=0;
  ctr=0;
  wgt=0;
	for(j=0;j<cmd->i3;j++) {					// find effective diffusion coefficients of all reported results
		if((reportchar=='e' && v2[j][0]==3.0) || (reportchar=='r' && v2[j][0]==2.0)) { // molecule should be recorded
      ctr++;
			if(msddim<0) {
				r2=0;
				for(d=0;d<dim;d++) {
					diff=v2[j][dim+1+d]-v2[j][1+d];
					r2+=diff*diff; }
        sum+=r2;
        wgt+=sim->time-v2[j][2*dim+1]; }
			else {
				diff=v2[j][dim+1+msddim]-v2[j][1+msddim];
        r2=diff*diff;
        sum+=r2;
        wgt+=sim->time-v2[j][2*dim+1]; }}}
  if(msddim<0) sum/=(2.0*dim);
  else sum/=2.0;

  scmdfprintf(cmd->cmds,fptr,"%g %i %g\n",sim->time,ctr,sum/wgt);					// display results
  
	for(j=0;j<cmd->i3;j++) {							// stop tracking expired molecules
		if(v2[j][0]==0 || v2[j][0]==2.0) {
			v1[j]=v1[cmd->i3-1];
			v1[cmd->i3-1]=0;
			dblptr=v2[j];
			v2[j]=v2[cmd->i3-1];
			v2[cmd->i3-1]=dblptr;
			v2[cmd->i3-1][0]=0;
			j--;
			cmd->i3--; }
		else
			v2[j][0]-=1.0; }
	if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3);
  
  if(change>0 && ctr>0 && cmd->f1>0 && fabs((sum/ctr-cmd->f1)/cmd->f1)<change)
    return docommand(sim,cmd,line2);
  cmd->f1=sum/ctr;
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdresidencetime(simptr sim,cmdptr cmd,char *line2) {
	int i,j,d,itct,ll,ctr,m,nmol,maxmol,summaryout,listout;
	FILE *fptr;
	moleculeptr *mlist;
	double sum;
	double **v2,*dblptr;
	long int *v1;
	enum MolecState ms;
	char startchar,reportchar;
  
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>=0,"cannot read molecule and/or state name; 'all' is not permitted");
	if(ms==MSall) ms=MSsoln;
	line2=strnword(line2,2);
	SCMDCHECK(line2,"insufficient arguments");
	itct=sscanf(line2,"%c %c %i %i %i",&startchar,&reportchar,&summaryout,&listout,&maxmol);
	SCMDCHECK(itct==5,"cannot read start, report, summary_out, list_out, or max_mol information");
	SCMDCHECK(maxmol>0,"max_mol has to be at least 1");
	line2=strnword(line2,6);
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");
  
	SCMDCHECK(cmd->i2!=2,"error on setup");					// failed before, don't try again
  
	ll=sim->mols->listlookup[i][ms];
	mlist=sim->mols->live[ll];
	nmol=sim->mols->nl[ll];
  
	if(!cmd->i2) {										// test for first run
		cmd->i2=1;											// if first run, set up data structures
		cmd->i1=maxmol;
		cmd->i3=0;
		cmd->freefn=&cmdmeansqrdispfree;
		cmd->v1=calloc(maxmol,sizeof(long int));	// v1 is serial numbers
		if(!cmd->v1) {cmd->i2=2;return CMDwarn;}
		v1=(long int*)cmd->v1;
		for(j=0;j<maxmol;j++) v1[j]=0;
		cmd->v2=calloc(maxmol,sizeof(double**));	// v2 is creation times
		if(!cmd->v2) {cmd->i2=2;return CMDwarn;}
		v2=(double**)cmd->v2;
		for(j=0;j<maxmol;j++) v2[j]=NULL;
		for(j=0;j<maxmol;j++) {
			v2[j]=(double*)calloc(2,sizeof(double));
			if(!v2[j]) {cmd->i2=2;return CMDwarn;}
			for(d=0;d<2;d++) v2[j][d]=0; }
		ctr=0;
		for(m=0;m<nmol;m++) {                    // record initial data
			if(mlist[m]->ident==i && mlist[m]->mstate==ms) {
				SCMDCHECK(ctr<maxmol,"insufficient allocated space");
				v1[ctr]=mlist[m]->serno;
				if(startchar=='c') v2[ctr][0]=0;
				else v2[ctr][0]=2.0;
        v2[ctr][1]=sim->time;
				ctr++; }}
		cmd->i3=ctr;
		if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3); }
  
	v1=(long int*)cmd->v1;						// start of code that is run every invocation
	v2=(double**)cmd->v2;
  
	for(m=0;m<nmol;m++) {							// update tracking information for all tracked molecules
		if(mlist[m]->ident==i && mlist[m]->mstate==ms) {
			j=locateVli(v1,mlist[m]->serno,cmd->i3);
			if(j>=0) {										// molecule was found
				v2[j][0]+=1.0; }
			else if(startchar!='i') {			// molecule was not found but should be tracked
				if(cmd->i3==cmd->i1) {SCMDCHECK(0,"not enough allocated space");}
				j=cmd->i3++;                // find empty spot
				v1[j]=mlist[m]->serno;
				v2[j][0]=3.0;
        v2[j][1]=sim->time; }}}
	if(startchar!='i')								// resort lists if appropriate
		if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3);
  
  sum=0;
  ctr=0;
	for(j=0;j<cmd->i3;j++) {					// find effective diffusion coefficients of all reported results
		if((reportchar=='e' && v2[j][0]==3.0) || (reportchar=='r' && v2[j][0]==2.0)) { // molecule should be recorded
      ctr++;
      sum+=sim->time-v2[j][1];
      if(listout>0 && cmd->invoke>0 && cmd->invoke%listout==0)
        scmdfprintf(cmd->cmds,fptr,"%li %g\n",v1[j],sim->time-v2[j][1]); }}
  
  if(summaryout>0 && cmd->invoke>0 && cmd->invoke%summaryout==0)
    scmdfprintf(cmd->cmds,fptr,"%g %i %g\n",sim->time,ctr,sum/ctr);					// display results
  
	for(j=0;j<cmd->i3;j++) {							// stop tracking expired molecules
		if(v2[j][0]==0 || v2[j][0]==2.0) {
			v1[j]=v1[cmd->i3-1];
			v1[cmd->i3-1]=0;
			dblptr=v2[j];
			v2[j]=v2[cmd->i3-1];
			v2[cmd->i3-1]=dblptr;
			v2[cmd->i3-1][0]=0;
			j--;
			cmd->i3--; }
		else
			v2[j][0]-=1.0; }
	if(cmd->i3>0) sortVliv(v1,(void**)cmd->v2,cmd->i3);
  
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmddiagnostics(simptr sim,cmdptr cmd,char *line2) {
	int itct,order;
	static char nm[STRCHAR];
	enum SmolStruct ss;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s",nm);
	SCMDCHECK(itct==1,"read failure");
	ss=simstring2ss(nm);
	SCMDCHECK(ss!=SSnone,"diagnostic type not recognized");
	
	if(ss==SSsim || ss==SSall) simoutput(sim);
	if(ss==SSwall || ss==SSall) walloutput(sim);
	if(ss==SSmolec || ss==SSall) molssoutput(sim);
	if(ss==SSsurf || ss==SSall) surfaceoutput(sim);
	if(ss==SScmd || ss==SSall) scmdoutput(sim->cmds);
	if(ss==SSbox || ss==SSall) boxssoutput(sim);
	if(ss==SSrxn || ss==SSall)
		for(order=0;order<MAXORDER;order++) rxnoutput(sim,order);
	if(ss==SScmpt || ss==SSall) compartoutput(sim);
	if(ss==SSport || ss==SSall) portoutput(sim);
	if(ss==SScheck || ss==SSall) checksimparams(sim);
	return CMDok; }


enum CMDcode cmdexecutiontime(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	scmdfprintf(cmd->cmds,fptr,"%g %g\n",sim->time,sim->elapsedtime+difftime(time(NULL),sim->clockstt));

	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdprintLattice(simptr sim,cmdptr cmd,char *line2) {
	FILE *fptr;
	latticeptr lattice;
	char buffer[STRCHAR*2];
	int n,i;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
	fptr=scmdgetfptr(sim->cmds,line2);
	SCMDCHECK(fptr,"file name not recognized");

	n=sim->latticess->nlattice;
	for(i=0;i<n;++i) {
		lattice=sim->latticess->latticelist[i];
		scmdfprintf(cmd->cmds,fptr,"Lattice %d: %s:\n",i,lattice->latticename);
		NSV_CALL(nsv_print(lattice->nsv,buffer));
		scmdfprintf(cmd->cmds,fptr,"%s",buffer); }
	scmdflush(fptr);
	return CMDok; }


enum CMDcode cmdwriteVTK(simptr sim,cmdptr cmd,char *line2) {

#ifndef OPTION_VTK
	simLog(NULL,11,"ERROR: VTK option not set. Recompile with OPTION_VTK = ON\n");
#else

	static char nm[STRCHAR];
	molssptr mols;
	moleculeptr mptr;
	int ll,m,dim,itct;
	vtkUnstructuredGrid* grid;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDobserve;
  SCMDCHECK(line2,"file name not given");
	itct=sscanf(line2,"%s",nm);
	SCMDCHECK(itct==1,"read failure");
  
//	printf("Writing out VTK files....\n");
	grid = vtkCreateMolGrid();
  
	dim=sim->dim;
	mols=sim->mols;
	if(!mols) return CMDwarn;
  
	for(ll=0;ll<sim->mols->nlist;ll++)
		if(sim->mols->listtype[ll]==MLTsystem) {
			for(m=0;m<mols->nl[ll];m++) {
				mptr=mols->live[ll][m];
				if(mptr->ident>0)
					vtkAddPoint(grid,mptr->pos[0],mptr->pos[1],mptr->pos[2],mptr->serno,mptr->ident);}}
  
	vtkWriteGrid(nm,"Molecules",cmd->invoke,grid);
	vtkDeleteGrid(grid);
  
#ifdef OPTION_NSV
	static char nm2[STRCHAR];
	latticeptr lattice;

	if (sim->latticess) {
		for (ll = 0; ll < sim->latticess->nlattice; ++ll) {
			lattice = sim->latticess->latticelist[ll];
			sprintf(nm2,"Lattice%02d_",ll);
			switch(lattice->type) {
        case LATTICEnsv:
          if (lattice->nsv) vtkWriteGrid(nm,nm2,cmd->invoke,nsv_get_grid(lattice->nsv));
          break;
        case LATTICEpde:
          //not implemented
          break;
        case LATTICEnone:
        break; }}}
#endif
#endif

	return CMDok; }


/**********************************************************/
/****************** system manipulation ********************/
/**********************************************************/


enum CMDcode cmdset(simptr sim,cmdptr cmd,char *line2) {
	int er,itct;
	char word[STRCHAR];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s",word);
	SCMDCHECK(itct==1,"missing statement");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing statement text");
	er=simreadstring(sim,NULL,word,line2);
	SCMDCHECK(!er,"%s",ErrorString);
	return CMDok; }


enum CMDcode cmdpointsource(simptr sim,cmdptr cmd,char *line2) {
	int itct,num,i;
	static char nm[STRCHAR];
	double pos[DIMMAX];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(sim->mols,"molecules are undefined");
	itct=sscanf(line2,"%s %i",nm,&num);
	SCMDCHECK(itct==2,"read failure");
	SCMDCHECK(num>=0,"number cannot be negative");
	i=stringfind(sim->mols->spname,sim->mols->nspecies,nm);
	SCMDCHECK(i>=1,"name not recognized");
	line2=strnword(line2,3);
	SCMDCHECK(line2,"missing location");
	itct=strreadnd(line2,sim->dim,pos,NULL);
	SCMDCHECK(itct==sim->dim,"insufficient location dimensions");
	SCMDCHECK(addmol(sim,num,i,pos,pos,1)==0,"not enough available molecules");
	return CMDok; }


enum CMDcode cmdvolumesource(simptr sim,cmdptr cmd,char *line2) {
	int itct,num,i,d;
	static char nm[STRCHAR];
	double poslo[DIMMAX],poshi[DIMMAX];
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(sim->mols,"molecules are undefined");
	itct=sscanf(line2,"%s %i",nm,&num);
	SCMDCHECK(itct==2,"read failure");
	SCMDCHECK(num>=0,"number cannot be negative");
	i=stringfind(sim->mols->spname,sim->mols->nspecies,nm);
	SCMDCHECK(i>=1,"name not recognized");
	line2=strnword(line2,3);
	SCMDCHECK(line2,"missing location");
	for(d=0;d<sim->dim;d++) {
		SCMDCHECK(line2,"missing argument");
		itct=sscanf(line2,"%lg %lg",&poslo[d],&poshi[d]);
		SCMDCHECK(itct==2,"read failure");
		line2=strnword(line2,3); }
	SCMDCHECK(addmol(sim,num,i,poslo,poshi,1)==0,"not enough available molecules");
	return CMDok; }


enum CMDcode cmdmovesurfacemol(simptr sim,cmdptr cmd,char *line2) {
	int itct,i1,p1,p2,ll,lllo,llhi,m,d,nmol;
	static char nm[STRCHAR],nm2[STRCHAR];
	double prob;
	enum MolecState ms1,ms2;
	enum PanelShape ps1,ps2;
	moleculeptr mptr,*mlist;
	double pos[DIMMAX];
	surfaceptr srf,srf2;
	panelptr pnl2;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing arguments");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(sim->srfss,"surfaces are undefined");
	itct=sscanf(line2,"%s %lg",nm,&prob);
	SCMDCHECK(itct==2,"failed to read molecule name or probability");
	i1=readmolname(sim,nm,&ms1,0);
	SCMDCHECK(i1>=0 || i1==-5,"failed to read molecule name");
	SCMDCHECK(ms1==MSfront || ms1==MSback || ms1==MSup || ms1==MSdown || ms1==MSall,"failed to read molecule state or illegal state");
	SCMDCHECK(prob>=0 && prob<=1,"probability out of bounds");
	line2=strnword(line2,3);
	SCMDCHECK(line2,"missing originating surface:panel");
	itct=sscanf(line2,"%s %s",nm,nm2);
	SCMDCHECK(itct==2,"failed to read surfaces and panels");
	srf=identifypanelindexinsurfacebyname(sim,nm,&ps1,&p1);
	SCMDCHECK(srf,"failed to read surface1");
	SCMDCHECK(p1>=0 || p1==-5,"failed to read panel1");
	srf2=identifypanelindexinsurfacebyname(sim,nm2,&ps2,&p2);
	SCMDCHECK(srf2,"failed to read surface2");
	SCMDCHECK(p2>=0 || p2==-5,"failed to read panel2");
	line2=strnword(line2,3);
	if(line2) {
		itct=sscanf(line2,"%s",nm);
		SCMDCHECK(itct==1,"failed to read final state");
		ms2=molstring2ms(nm);
		SCMDCHECK(ms2!=MSnone,"failed to read final state");
		line2=strnword(line2,2); }
	else ms2=MSnone;

	if(p2==-5) pnl2=NULL;
	else pnl2=srf2->panels[ps2][p2];

	if(i1<0 || ms1==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i1][ms1]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident>0 && i1<0 && (ms1==MSall || mptr->mstate==ms1)) || (mptr->ident==i1 && (ms1==MSall || mptr->mstate==ms1)))
				if(mptr->pnl && mptr->pnl->srf==srf && (p1==-5 || mptr->pnl==srf->panels[ps1][p1]))
					if(randCOD()<prob) {
						if(p2==-5) pnl2=surfrandpos(srf2,pos,sim->dim);
						else panelrandpos(pnl2,pos,sim->dim);
						for(d=0;d<sim->dim;d++) {
							mptr->posoffset[d]=mptr->pos[d]-pos[d];
							mptr->posx[d]=mptr->pos[d]=pos[d]; }
						molchangeident(sim,mptr,ll,m,mptr->ident,ms2==MSnone?mptr->mstate:ms2,pnl2); }}}
	return CMDok; }


enum CMDcode cmdkillmol(simptr sim,cmdptr cmd,char *line2) {
	int i,ll,m,lllo,llhi,nmol;
	moleculeptr *mlist,mptr;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=mlist[m];
			if((i<0 && ms==MSall) || (i<0 && mptr->mstate==ms) || (mptr->ident==i && ms==MSall) || (mptr->ident==i && mptr->mstate==ms))
				molkill(sim,mptr,ll,m); }}
	return CMDok; }


enum CMDcode cmdkillmolprob(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,ll,m,nmol,lllo,llhi;
	moleculeptr *mlist,mptr;
	double prob;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing probability value");
	itct=sscanf(line2,"%lg",&prob);
	SCMDCHECK(itct==1,"killmolprob format: name[(state)] probability");
	SCMDCHECK(prob>=0 && prob<=1,"probability needs to be between 0 and 1");
	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if(coinrandD(prob) && ((i<0 && ms==MSall) || (i<0 && mptr->mstate==ms) || (mptr->ident==i && ms==MSall) || (mptr->ident==i && mptr->mstate==ms)))
				molkill(sim,mptr,ll,m); }}
	return CMDok; }


enum CMDcode cmdkillmolinsphere(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,s,ll,m,lllo,llhi,nmol;
	static char nm[STRCHAR];
	moleculeptr *mlist,mptr;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	if(!sim->srfss) return CMDok;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing surface name");
	itct=sscanf(line2,"%s",nm);
	SCMDCHECK(itct==1,"cannot read surface name");
	if(!strcmp(nm,"all")) s=-1;
	else {
		s=stringfind(sim->srfss->snames,sim->srfss->nsrf,nm);
		SCMDCHECK(s>=0,"surface not recognized"); }

	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((i<0 || mptr->ident==i) && (ms==MSall || mptr->mstate==ms))
				if(molinpanels(sim,ll,m,s,'s')) molkill(sim,mptr,ll,m); }}
	return CMDok; }


enum CMDcode cmdkillmolincmpt(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,ll,m,c;
	static char cname[STRCHAR];
	moleculeptr mptr;
	enum MolecState ms;
	compartssptr cmptss;
	compartptr cmpt;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;

	cmptss=sim->cmptss;
	SCMDCHECK(cmptss,"no compartments defined");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(line2,"missing argument");
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<=0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	SCMDCHECK(line2=strnword(line2,2),"missing value argument");
	itct=sscanf(line2,"%s",cname);
	SCMDCHECK(itct==1,"cannot read compartment name");
	c=stringfind(cmptss->cnames,cmptss->ncmpt,cname);
	SCMDCHECK(c>=0,"compartment name not recognized");
	cmpt=cmptss->cmptlist[c];

	if(i>0 && ms!=MSall) {
		ll=sim->mols->listlookup[i][ms];
		for(m=0;m<sim->mols->nl[ll];m++) {
			mptr=sim->mols->live[ll][m];
			if(mptr->ident==i && mptr->mstate==ms && posincompart(sim,mptr->pos,cmpt))
				molkill(sim,mptr,ll,m); }}
	else {
		for(ll=0;ll<sim->mols->nlist;ll++)
			if(sim->mols->listtype[ll]==MLTsystem)
				for(m=0;m<sim->mols->nl[ll];m++) {
					mptr=sim->mols->live[ll][m];
					if(i==-5 || mptr->ident==i)
						if(ms==MSall || mptr->mstate==ms)
							if(posincompart(sim,mptr->pos,cmpt))
								molkill(sim,mptr,ll,m); }}

	return CMDok; }


enum CMDcode cmdkillmoloutsidesystem(simptr sim,cmdptr cmd,char *line2) {
	int i,ll,m,lllo,llhi,nmol;
	moleculeptr *mlist,mptr;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	if(!sim->srfss) return CMDok;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	if(i<0 || ms==MSall) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((i<0 && ms==MSall) || (i<0 && mptr->mstate==ms) || (mptr->ident==i && ms==MSall) || (mptr->ident==i && mptr->mstate==ms))
				if(!posinsystem(sim,mptr->pos)) molkill(sim,mptr,ll,m); }}
	return CMDok; }


enum CMDcode cmdfixmolcount(simptr sim,cmdptr cmd,char *line2) {
	int itct,num,i,ll,m,ct,numl;
	static char nm[STRCHAR];
	double pos1[DIMMAX],pos2[DIMMAX];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(sim->mols,"molecules are undefined");
	itct=sscanf(line2,"%s %i",nm,&num);
	SCMDCHECK(itct==2,"read failure");
	SCMDCHECK(num>=0,"number cannot be negative");
	i=stringfind(sim->mols->spname,sim->mols->nspecies,nm);
	SCMDCHECK(i>=1,"name not recognized");

	ll=sim->mols->listlookup[i][MSsoln];
	numl=sim->mols->nl[ll];
	ct=0;
	for(m=0;m<numl;m++)
		if(sim->mols->live[ll][m]->ident==i) ct++;

	if(ct==num);
	else if(ct<num) {
		systemcorners(sim,pos1,pos2);
		SCMDCHECK(addmol(sim,num-ct,i,pos1,pos2,1)==0,"not enough available molecules"); }
	else {
		num=ct-num;
		for(;num>0;num--) {
			m=intrand(numl);
			while(sim->mols->live[ll][m]->ident!=i) m=(m==numl-1)?0:m+1;
			molkill(sim,sim->mols->live[ll][m],ll,m); }}

	return CMDok; }


enum CMDcode cmdfixmolcountrange(simptr sim,cmdptr cmd,char *line2) {
	int itct,lownum,highnum,i,ll,m,ct,numl;
	static char nm[STRCHAR];
	double pos1[DIMMAX],pos2[DIMMAX];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(sim->mols,"molecules are undefined");
	itct=sscanf(line2,"%s %i %i",nm,&lownum,&highnum);
	SCMDCHECK(itct==3,"read failure");
	SCMDCHECK(lownum>=0 && highnum>=0 && highnum>=lownum,"molecule numbers are out of bounds");
	i=stringfind(sim->mols->spname,sim->mols->nspecies,nm);
	SCMDCHECK(i>=1,"species name not recognized");

	ll=sim->mols->listlookup[i][MSsoln];
	numl=sim->mols->nl[ll];
	ct=0;
	for(m=0;m<numl;m++)
		if(sim->mols->live[ll][m]->ident==i) ct++;

	if(ct>=lownum && ct<=highnum);
	else if(ct<lownum) {
		systemcorners(sim,pos1,pos2);
		SCMDCHECK(addmol(sim,lownum-ct,i,pos1,pos2,1)==0,"not enough available molecules"); }
	else {
		highnum=ct-highnum;
		for(;highnum>0;highnum--) {
			m=intrand(numl);
			while(sim->mols->live[ll][m]->ident!=i) m=(m==numl-1)?0:m+1;
			molkill(sim,sim->mols->live[ll][m],ll,m); }}

	return CMDok; }


enum CMDcode cmdfixmolcountonsurf(simptr sim,cmdptr cmd,char *line2) {
	int itct,num,i,ll,m,ct,numl,s;
	static char nm[STRCHAR];
	enum MolecState ms;
	surfaceptr sptr;
	moleculeptr mptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>0,"failed to read molecule name or state");
	SCMDCHECK(ms!=MSsoln && ms!=MSbsoln,"molecule state needs to be surface-bound");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"fixmolcountonsurf format: species(state) number surface");
	itct=sscanf(line2,"%i %s",&num,nm);
	SCMDCHECK(itct==2,"read failure");
	SCMDCHECK(num>=0,"number cannot be negative");
	SCMDCHECK(sim->srfss,"no surfaces defined");
	s=stringfind(sim->srfss->snames,sim->srfss->nsrf,nm);
	SCMDCHECK(s>=0,"surface not recognized");
	sptr=sim->srfss->srflist[s];

	ll=sim->mols->listlookup[i][ms];
	numl=sim->mols->nl[ll];
	ct=0;
	for(m=0;m<numl;m++) {
		mptr=sim->mols->live[ll][m];
		if(mptr->ident==i && mptr->mstate==ms && mptr->pnl->srf==sptr) ct++; }

	if(ct==num);
	else if(ct<num) {
		SCMDCHECK(addsurfmol(sim,num-ct,i,ms,NULL,NULL,s,PSall,NULL)==0,"not enough available molecules"); }
	else {
		num=ct-num;
		for(;num>0;num--) {
			m=intrand(numl);
			mptr=sim->mols->live[ll][m];
			while(!(mptr->ident==i && mptr->mstate==ms && mptr->pnl->srf==sptr)) {
				m=(m==numl-1)?0:m+1;
				mptr=sim->mols->live[ll][m]; }
			molkill(sim,mptr,ll,m); }}

	return CMDok; }


enum CMDcode cmdfixmolcountrangeonsurf(simptr sim,cmdptr cmd,char *line2) {
	int itct,lownum,highnum,i,ll,m,ct,numl,s;
	static char nm[STRCHAR];
	enum MolecState ms;
	surfaceptr sptr;
	moleculeptr mptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>0,"failed to read molecule name or state");
	SCMDCHECK(ms!=MSsoln && ms!=MSbsoln,"molecule state needs to be surface-bound");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"fixmolcountrangeonsurf format: species(state) low_number high_number surface");
	itct=sscanf(line2,"%i %i %s",&lownum,&highnum,nm);
	SCMDCHECK(itct==3,"read failure");
	SCMDCHECK(lownum>=0 && highnum>=0 && highnum>=lownum,"molecule numbers are out of bounds");
	SCMDCHECK(sim->srfss,"no surfaces defined");
	s=stringfind(sim->srfss->snames,sim->srfss->nsrf,nm);
	SCMDCHECK(s>=0,"surface not recognized");
	sptr=sim->srfss->srflist[s];

	ll=sim->mols->listlookup[i][ms];
	numl=sim->mols->nl[ll];
	ct=0;
	for(m=0;m<numl;m++) {
		mptr=sim->mols->live[ll][m];
		if(mptr->ident==i && mptr->mstate==ms && mptr->pnl->srf==sptr) ct++; }

	if(ct>=lownum && ct<=highnum);
	else if(ct<lownum) {
		SCMDCHECK(addsurfmol(sim,lownum-ct,i,ms,NULL,NULL,s,PSall,NULL)==0,"not enough available molecules"); }
	else {
		highnum=ct-highnum;
		for(;highnum>0;highnum--) {
			m=intrand(numl);
			mptr=sim->mols->live[ll][m];
			while(!(mptr->ident==i && mptr->mstate==ms && mptr->pnl->srf==sptr)) {
				m=(m==numl-1)?0:m+1;
				mptr=sim->mols->live[ll][m]; }
			molkill(sim,mptr,ll,m); }}

	return CMDok; }


enum CMDcode cmdfixmolcountincmpt(simptr sim,cmdptr cmd,char *line2) {
	int itct,num,i,ll,m,ct,numl,c;
	static char nm[STRCHAR];
	moleculeptr mptr;
	compartptr cmpt;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(sim->cmptss,"compartments are undefined");
	itct=sscanf(line2,"%s %i",nm,&num);
	SCMDCHECK(itct==2,"read failure");
	SCMDCHECK(num>=0,"number cannot be negative");
	i=stringfind(sim->mols->spname,sim->mols->nspecies,nm);
	SCMDCHECK(i>=1,"molecule name not recognized");
	line2=strnword(line2,3);
	SCMDCHECK(line2,"compartment name missing");
	itct=sscanf(line2,"%s",nm);
	c=stringfind(sim->cmptss->cnames,sim->cmptss->ncmpt,nm);
	SCMDCHECK(c>=0,"compartment not recognized");
	cmpt=sim->cmptss->cmptlist[c];

	ll=sim->mols->listlookup[i][MSsoln];
	numl=sim->mols->nl[ll];
	ct=0;
	for(m=0;m<numl;m++) {
		mptr=sim->mols->live[ll][m];
		if(mptr->ident==i && mptr->mstate==MSsoln && posincompart(sim,mptr->pos,cmpt)) ct++; }

	if(ct==num);
	else if(ct<num) {
		SCMDCHECK(addcompartmol(sim,num-ct,i,cmpt)==0,"not enough available molecules"); }
	else {
		num=ct-num;
		for(;num>0;num--) {
			m=intrand(numl);
			mptr=sim->mols->live[ll][m];
			while(!(mptr->ident==i && mptr->mstate==MSsoln && posincompart(sim,mptr->pos,cmpt))) {
				m=(m==numl-1)?0:m+1;
				mptr=sim->mols->live[ll][m]; }
			molkill(sim,mptr,ll,m); }}

	return CMDok; }


enum CMDcode cmdfixmolcountrangeincmpt(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,ll,m,ct,numl,c,lownum,highnum;
	static char nm[STRCHAR];
	moleculeptr mptr;
	compartptr cmpt;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(sim->mols,"molecules are undefined");
	SCMDCHECK(sim->cmptss,"compartments are undefined");
	itct=sscanf(line2,"%s %i %i",nm,&lownum,&highnum);
	SCMDCHECK(itct==3,"read failure");
	i=stringfind(sim->mols->spname,sim->mols->nspecies,nm);
	SCMDCHECK(i>=1,"molecule name not recognized");
	line2=strnword(line2,4);
	SCMDCHECK(line2,"compartment name missing");
	itct=sscanf(line2,"%s",nm);
	c=stringfind(sim->cmptss->cnames,sim->cmptss->ncmpt,nm);
	SCMDCHECK(c>=0,"compartment not recognized");
	cmpt=sim->cmptss->cmptlist[c];

	ll=sim->mols->listlookup[i][MSsoln];
	numl=sim->mols->nl[ll];
	ct=0;
	for(m=0;m<numl;m++) {
		mptr=sim->mols->live[ll][m];
		if(mptr->ident==i && mptr->mstate==MSsoln && posincompart(sim,mptr->pos,cmpt)) ct++; }

	if(ct>=lownum && ct<=highnum);
	else if(ct<lownum) {
		SCMDCHECK(addcompartmol(sim,lownum-ct,i,cmpt)==0,"not enough available molecules"); }
	else {
		highnum=ct-highnum;
		for(;highnum>0;highnum--) {
			m=intrand(numl);
			mptr=sim->mols->live[ll][m];
			while(!(mptr->ident==i && mptr->mstate==MSsoln && posincompart(sim,mptr->pos,cmpt))) {
				m=(m==numl-1)?0:m+1;
				mptr=sim->mols->live[ll][m]; }
			molkill(sim,mptr,ll,m); }}

	return CMDok; }


enum CMDcode cmdequilmol(simptr sim,cmdptr cmd,char *line2) {
	int itct,i1,i2,m,ll1,ll2,ll,nmol;
	moleculeptr *mlist,mptr;
	double prob;
	enum MolecState ms1,ms2;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i1=readmolname(sim,line2,&ms1,0);
	SCMDCHECK(i1>0,"cannot read first molecule and/or state name; 'all' is not permitted");
	if(ms1==MSall) ms1=MSsoln;
	line2=strnword(line2,2);
	i2=readmolname(sim,line2,&ms2,0);
	SCMDCHECK(i2>0,"cannot read second molecule and/or state name; 'all' is not permitted");
	if(ms2==MSall) ms2=MSsoln;
	SCMDCHECK((ms1==MSsoln && ms2==MSsoln) || (ms1!=MSsoln && ms2!=MSsoln),"cannot equilibrate between solution and surface-bound");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing probability argument");
	itct=sscanf(line2,"%lg",&prob);
	SCMDCHECK(itct==1,"failed to read probability");
	SCMDCHECK(prob>=0 && prob<=1,"probability is out of bounds");

	ll1=sim->mols->listlookup[i1][ms1];
	ll2=sim->mols->listlookup[i2][ms2];
	for(ll=ll1;ll!=-1;ll=(ll==ll2?-1:ll2)) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident==i1 && mptr->mstate==ms1) || (mptr->ident==i2 && mptr->mstate==ms2)) {
				if(coinrandD(prob))
					molchangeident(sim,mptr,ll,m,i2,ms2,mptr->pnl);
				else
					molchangeident(sim,mptr,ll,m,i1,ms1,mptr->pnl); }}}
	return CMDok; }


enum CMDcode cmdreplacexyzmol(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,m,d,ll;
	moleculeptr *mlist;
	boxptr bptr;
	double pos[DIMMAX];
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(i>=0,"cannot read molecule and/or state name; 'all' is not permitted");
	SCMDCHECK(ms!=MSall,"molecule state may not be 'all'");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing position information");
	itct=strreadnd(line2,sim->dim,pos,NULL);
	SCMDCHECK(itct==sim->dim,"insufficient dimensions entered");
	bptr=pos2box(sim,pos);
	ll=sim->mols->listlookup[i][ms];
	mlist=bptr->mol[ll];
	for(m=0;m<bptr->nmol[ll];m++) {
		for(d=0;d<sim->dim;d++)
			if(mlist[m]->pos[d]!=pos[d]) d=sim->dim+1;
		if(d==sim->dim) {
			molchangeident(sim,mlist[m],ll,-1,i,ms,mlist[m]->pnl);
			m=bptr->nmol[ll]+1; }}
	return CMDok; }


enum CMDcode cmdreplacevolmol(simptr sim,cmdptr cmd,char *line2) {
	int m,itct,dim,d,b,b1,b2,i1,i2,ll;
	double *pos,poslo[DIMMAX],poshi[DIMMAX],frac;
	boxptr bptr1,bptr2,bptr;
	boxssptr boxs;
	moleculeptr *mlist;
	enum MolecState ms1,ms2;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i1=readmolname(sim,line2,&ms1,0);
	SCMDCHECK(i1>0,"cannot read first molecule and/or state name; 'all' is not permitted");
	SCMDCHECK(ms1!=MSall,"first state may not be 'all'");
	line2=strnword(line2,2);
	i2=readmolname(sim,line2,&ms2,0);
	SCMDCHECK(i2>=0,"cannot read second molecule and/or state name; 'all' is not permitted");
	SCMDCHECK(ms2!=MSall,"second state may not be 'all'");
	SCMDCHECK((ms1==MSsoln && ms2==MSsoln) || (ms1!=MSsoln && ms2!=MSsoln),"cannot equilibrate between solution and surface-bound");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing fraction information");
	itct=sscanf(line2,"%lg",&frac);
	SCMDCHECK(itct==1,"cannot read fraction");
	SCMDCHECK(frac>=0 && frac<=1,"fraction out of bounds");
	line2=strnword(line2,2);
	dim=sim->dim;
	boxs=sim->boxs;
	for(d=0;d<dim;d++) {
		SCMDCHECK(line2,"missing argument");
		itct=sscanf(line2,"%lg %lg",&poslo[d],&poshi[d]);
		SCMDCHECK(itct==2,"read failure");
		line2=strnword(line2,3); }

	bptr1=pos2box(sim,poslo);
	bptr2=pos2box(sim,poshi);
	b1=indx2addZV(bptr1->indx,boxs->side,dim);
	b2=indx2addZV(bptr2->indx,boxs->side,dim);
	ll=sim->mols->listlookup[i1][ms1];
	for(b=b1;b<=b2;b=nextaddZV(b,bptr1->indx,bptr2->indx,boxs->side,dim)) {
		bptr=boxs->blist[b];
		mlist=bptr->mol[ll];
		for(m=0;m<bptr->nmol[ll];m++) {
			if(mlist[m]->ident==i1 && mlist[m]->mstate==ms1) {
				pos=mlist[m]->pos;
				for(d=0;d<dim;d++) if(pos[d]<poslo[d] || pos[d]>poshi[d]) d=dim+1;
				if(d==dim && coinrandD(frac)) {
					molchangeident(sim,mlist[m],ll,-1,i2,ms2,mlist[m]->pnl); }}}}
	return CMDok; }


enum CMDcode cmdreplacecmptmol(simptr sim,cmdptr cmd,char *line2) {
	int m,itct,i1,i2,ll,c,numl;
	char nm[STRCHAR];
	double frac;
	enum MolecState ms1,ms2;
	compartptr cmpt;
	moleculeptr mptr;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i1=readmolname(sim,line2,&ms1,0);
	SCMDCHECK(i1>0,"cannot read first molecule and/or state name; 'all' is not permitted");
	SCMDCHECK(ms1!=MSall,"first state may not be 'all'");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing second species name");
	i2=readmolname(sim,line2,&ms2,0);
	SCMDCHECK(i2>=0,"cannot read second molecule and/or state name; 'all' is not permitted");
	SCMDCHECK(ms2!=MSall,"second state may not be 'all'");
	SCMDCHECK((ms1==MSsoln && ms2==MSsoln) || (ms1!=MSsoln && ms2!=MSsoln),"cannot equilibrate between solution and surface-bound");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing fraction information");
	itct=sscanf(line2,"%lg",&frac);
	SCMDCHECK(itct==1,"cannot read fraction");
	SCMDCHECK(frac>=0 && frac<=1,"fraction out of bounds");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"compartment name missing");
	itct=sscanf(line2,"%s",nm);
	c=stringfind(sim->cmptss->cnames,sim->cmptss->ncmpt,nm);
	SCMDCHECK(c>=0,"compartment not recognized");
	cmpt=sim->cmptss->cmptlist[c];

	ll=sim->mols->listlookup[i1][ms1];
	numl=sim->mols->nl[ll];
	for(m=0;m<numl;m++) {
		mptr=sim->mols->live[ll][m];
		if(mptr->ident==i1 && mptr->mstate==ms1 && posincompart(sim,mptr->pos,cmpt) && coinrandD(frac)) 
			molchangeident(sim,mptr,ll,m,i2,ms2,mptr->pnl); }
	return CMDok; }


enum CMDcode cmdmodulatemol(simptr sim,cmdptr cmd,char *line2) {
	int itct,i1,i2,m,ll1,ll2,nmol,ll;
	moleculeptr *mlist,mptr;
	double freq,shift,prob;
	enum MolecState ms1,ms2;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i1=readmolname(sim,line2,&ms1,0);
	SCMDCHECK(i1>0,"cannot read first molecule and/or state name; 'all' is not permitted");
	SCMDCHECK(ms1!=MSall,"first state may not be 'all'");
	line2=strnword(line2,2);
	i2=readmolname(sim,line2,&ms2,0);
	SCMDCHECK(i2>=0,"cannot read second molecule and/or state name; 'all' is not permitted");
	SCMDCHECK(ms1!=MSall,"second state may not be 'all'");
	SCMDCHECK((ms1==MSsoln && ms2==MSsoln) || (ms1!=MSsoln && ms2!=MSsoln),"cannot equilibrate between solution and surface-bound");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"missing frequency and shift");
	itct=sscanf(line2,"%lg %lg",&freq,&shift);
	SCMDCHECK(itct==2,"failure reading frequency or shift");

	ll1=sim->mols->listlookup[i1][ms1];
	ll2=sim->mols->listlookup[i2][ms2];
	prob=0.5*(1.0-cos(freq*sim->time+shift));
	for(ll=ll1;ll!=-1;ll=(ll==ll2?-1:ll2)) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((mptr->ident==i1 && mptr->mstate==ms1) || (mptr->ident==i2 && mptr->mstate==ms2)) {
				if(coinrandD(prob))
					molchangeident(sim,mptr,ll,m,i2,ms2,mptr->pnl);
				else
					molchangeident(sim,mptr,ll,m,i1,ms1,mptr->pnl); }}}
	return CMDok; }


enum CMDcode cmdreact1(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,ll,m,r,nmol,lllo,llhi;
	static char rnm[STRCHAR];
	moleculeptr *mlist,mptr;
	enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	i=readmolname(sim,line2,&ms,0);
	SCMDCHECK(!(i<=0 && i>-5),"cannot read molecule and/or state name");
	SCMDCHECK(i!=-6,"wildcard characters not permitted in species name");
	line2=strnword(line2,2);
	SCMDCHECK(line2,"reaction name is missing");
	itct=sscanf(line2,"%s",rnm);
	SCMDCHECK(itct==1,"cannot read reaction name");
	SCMDCHECK(sim->rxnss[1],"no first order reactions defined");
	r=stringfind(sim->rxnss[1]->rname,sim->rxnss[1]->totrxn,rnm);
	SCMDCHECK(r>=0,"reaction not recognized");

	if(i<0) {lllo=0;llhi=sim->mols->nlist;}
	else llhi=1+(lllo=sim->mols->listlookup[i][ms]);
	for(ll=lllo;ll<llhi;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++) {
			mptr=mlist[m];
			if((i<0 && ms==MSall) || (i<0 && mptr->mstate==ms) || (mptr->ident==i && ms==MSall) || (mptr->ident==i && mptr->mstate==ms))
				if(doreact(sim,sim->rxnss[1]->rxn[r],mptr,NULL,ll,m,-1,-1,NULL,NULL)) return CMDwarn; }}
	return CMDok; }


enum CMDcode cmdsetrateint(simptr sim,cmdptr cmd,char *line2) {
	int itct,r,order;
	static char rnm[STRCHAR];
	double rateint;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s %lg",rnm,&rateint);
	SCMDCHECK(itct==2,"read failure");
	r=-1;
	if(sim->rxnss[0]) r=stringfind(sim->rxnss[0]->rname,sim->rxnss[0]->totrxn,rnm);
	if(r>=0) order=0;
	else {
		if(sim->rxnss[1]) r=stringfind(sim->rxnss[1]->rname,sim->rxnss[1]->totrxn,rnm);
		if(r>=0) order=1;
		else {
			if(sim->rxnss[2]) r=stringfind(sim->rxnss[2]->rname,sim->rxnss[2]->totrxn,rnm);
			if(r>=0) order=2;
			else SCMDCHECK(0,"reaction name not recognized"); }}
	SCMDCHECK(rateint>=0,"internal rate cannot be negative");
	if(order<2) RxnSetValue(sim,"prob",sim->rxnss[order]->rxn[r],rateint);
	else RxnSetValue(sim,"bindrad",sim->rxnss[order]->rxn[r],rateint);
	return CMDok; }


enum CMDcode cmdshufflemollist(simptr sim,cmdptr cmd,char *line2) {
	int itct,ll,lllo,llhi;
	char nm[STRCHAR];

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s",nm);
	SCMDCHECK(itct==1,"read failure");
	SCMDCHECK(sim->mols && sim->mols->nlist>0,"no molecule lists");
	if(!strcmp(nm,"all")) ll=-1;
	else {
		ll=stringfind(sim->mols->listname,sim->mols->nlist,nm);
		SCMDCHECK(ll>=0,"list name not recognized"); }
	lllo=(ll==-1)?0:ll;
	llhi=(ll==-1)?sim->mols->nlist:ll+1;
	for(ll=lllo;ll<llhi;ll++)
		randshuffletableV((void**) sim->mols->live[ll],sim->mols->nl[ll]);
	return CMDok; }


enum CMDcode cmdshufflereactions(simptr sim,cmdptr cmd,char *line2) {
	int itct,i,j,i1,i2,i1lo,i1hi,i2lo,i2hi,isym;
	char nm1[STRCHAR],nm2[STRCHAR];
	enum MolecState ms1,ms2;
	rxnssptr rxnss;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s %s",nm1,nm2);
	SCMDCHECK(itct==2,"missing argument");
	i1=readmolname(sim,nm1,&ms1,0);
	SCMDCHECK(i1>=0 || i1==-5,"first species name not recognized");
	i2=readmolname(sim,nm2,&ms2,0);
	SCMDCHECK(i2>=0 || i2==-5,"second species name not recognized");
	rxnss=sim->rxnss[2];
	if(!rxnss) return CMDok;

	i1lo=(i1<0)?0:i1;
	i1hi=(i1<0)?rxnss->maxspecies:i1+1;
	i2lo=(i2<0)?0:i2;
	i2hi=(i2<0)?rxnss->maxspecies:i2+1;

	for(i1=i1lo;i1<i1hi;i1++)
		for(i2=i2lo;i2<i2hi;i2++) {
			i=i1*rxnss->maxspecies+i2;
			if(rxnss->nrxn[i]) {
				randshuffletableI(rxnss->table[i],rxnss->nrxn[i]);
				isym=i2*rxnss->maxspecies+i1;
				for(j=0;j<rxnss->nrxn[i];j++)
					rxnss->table[isym][j]=rxnss->table[i][j]; }}

	return CMDok; }

	


/*
enum CMDcode cmdsetsurfcoeff(simptr sim,cmdptr cmd,char *line2) {
	int itct,s;
	double rate;
	enum MolecState ms1, ms2; 
	char nm[STRCHAR],molname[STRCHAR], ms1string[STRCHAR],ms2string[STRCHAR];
	surfaceptr srf;
	surfacessptr srfss;
	int i;
	
	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(cmd->i1!=2,"error on setup");					// failed before, don't try again
	srfss=sim->srfss;
	SCMDCHECK(srfss,"no surfaces defined");
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s %s %s %s %lg",nm,molname,ms1string,ms2string,&rate);	
	SCMDCHECK(itct==5,"read failure");
	s=stringfind(srfss->snames,srfss->nsrf,nm);	
	SCMDCHECK(s>=0,"surface name not recognized");
	srf=srfss->srflist[s];	
	i=stringfind(sim->mols->spname,sim->mols->nspecies,molname);
	SCMDCHECK(i>=0,"molecule name not recognized");
	ms1=molstring2ms(ms1string);
	ms2=molstring2ms(ms2string);
	SCMDCHECK(ms1!=MSall,"molecule state may not be 'all'");
	SCMDCHECK(ms2!=MSall,"molecule state may not be 'all'");	
	SCMDCHECK(ms1!=MSnone,"molecule state may not be 'none'");
	SCMDCHECK(ms2!=MSnone,"molecule state may not be 'none'");
	SCMDCHECK(rate>=0,"rate may not be negative");
	srf->srfrate[i][ms1][ms2]=rate;
	surfupdateparams(sim);
return CMDok; }
*/


enum CMDcode cmdsettimestep(simptr sim,cmdptr cmd,char *line2) {
	int itct,er;
	double dt;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%lg",&dt);
	SCMDCHECK(itct==1,"read failure");
	SCMDCHECK(dt>0,"time step must be >0");

	er=simsettime(sim,dt,3);
	SCMDCHECK(er==0,"error while setting the simulation time step");
	return CMDok; }


enum CMDcode cmdporttransport(simptr sim,cmdptr cmd,char *line2) {
	int itct,prt1,prt2;
	char nm1[STRCHAR],nm2[STRCHAR];
	portptr port1,port2;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	SCMDCHECK(sim->portss,"no port superstructure is defined");
	itct=sscanf(line2,"%s %s",nm1,nm2);
	SCMDCHECK(itct==2,"porttransport format: port1 port2");
	prt1=stringfind(sim->portss->portnames,sim->portss->nport,nm1);
	SCMDCHECK(prt1>=0,"name of port1 is not recognized");
	prt2=stringfind(sim->portss->portnames,sim->portss->nport,nm2);
	SCMDCHECK(prt2>=0,"name of port2 is not recognized");
	port1=sim->portss->portlist[prt1];
	port2=sim->portss->portlist[prt2];
	porttransport(sim,port1,sim,port2);
	return CMDok; }


enum CMDcode cmdexcludebox(simptr sim,cmdptr cmd,char *line2) {
	int m,itct,dim,d,b,b1,b2;
	double *pos,poslo[DIMMAX],poshi[DIMMAX];
	boxptr bptr1,bptr2,bptr;
	boxssptr boxs;
	moleculeptr *mlist;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	dim=sim->dim;
	boxs=sim->boxs;
	for(d=0;d<dim;d++) {
		SCMDCHECK(line2,"missing argument");
		itct=sscanf(line2,"%lg %lg",&poslo[d],&poshi[d]);
		SCMDCHECK(itct==2,"read failure");
		line2=strnword(line2,3); }

	bptr1=pos2box(sim,poslo);
	bptr2=pos2box(sim,poshi);
	b1=indx2addZV(bptr1->indx,boxs->side,dim);
	b2=indx2addZV(bptr2->indx,boxs->side,dim);
	for(b=b1;b<=b2;b=nextaddZV(b,bptr1->indx,bptr2->indx,boxs->side,dim)) {
		bptr=boxs->blist[b];
		mlist=bptr->mol[0];
		for(m=0;m<bptr->nmol[0];m++) {
			pos=mlist[m]->pos;
			for(d=0;d<dim;d++) if(pos[d]<poslo[d] || pos[d]>poshi[d]) d=dim+1;
			if(d==dim) {
				pos=mlist[m]->posx;
				for(d=0;d<dim;d++) if(pos[d]<poslo[d] || pos[d]>poshi[d]) d=dim+1;
				if(d>dim) copyVD(mlist[m]->posx,mlist[m]->pos,dim); }}}
	return CMDok; }


enum CMDcode cmdexcludesphere(simptr sim,cmdptr cmd,char *line2) {
	int m,itct,dim,d,b,b1,b2;
	double *pos,poslo[DIMMAX],poshi[DIMMAX],poscent[DIMMAX],rad,dist;
	boxptr bptr1,bptr2,bptr;
	boxssptr boxs;
	moleculeptr *mlist;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	dim=sim->dim;
	boxs=sim->boxs;
	for(d=0;d<dim;d++) {
		SCMDCHECK(line2,"missing center argument");
		itct=sscanf(line2,"%lg",&poscent[d]);
		SCMDCHECK(itct==1,"failure reading center");
		line2=strnword(line2,2); }
	SCMDCHECK(line2,"missing radius");
	itct=sscanf(line2,"%lg",&rad);
	SCMDCHECK(itct==1,"failure reading radius");

	dist=rad*sqrt((double)dim);
	for(d=0;d<dim;d++) {
		poslo[d]=poscent[d]-dist;
		poshi[d]=poscent[d]+dist; }
	rad*=rad;
	bptr1=pos2box(sim,poslo);
	bptr2=pos2box(sim,poshi);
	b1=indx2addZV(bptr1->indx,boxs->side,dim);
	b2=indx2addZV(bptr2->indx,boxs->side,dim);
	for(b=b1;b<=b2;b=nextaddZV(b,bptr1->indx,bptr2->indx,boxs->side,dim)) {
		bptr=boxs->blist[b];
		mlist=bptr->mol[0];
		for(m=0;m<bptr->nmol[0];m++) {
			pos=mlist[m]->pos;
			for(dist=0,d=0;d<dim;d++) if((dist+=(pos[d]-poscent[d])*(pos[d]-poscent[d]))>rad) d=dim+1;
			if(d==dim) {
				pos=mlist[m]->posx;
				for(dist=0,d=0;d<dim;d++) if((dist+=(pos[d]-poscent[d])*(pos[d]-poscent[d]))>rad) d=dim+1;
				if(d>dim) copyVD(mlist[m]->posx,mlist[m]->pos,dim); }}}
	return CMDok; }


enum CMDcode cmdincludeecoli(simptr sim,cmdptr cmd,char *line2) {
	int m,ll,nmol;
	moleculeptr *mlist;
	double rad,length,pos[DIMMAX];
	wallptr *wlist;
  
	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(sim->dim==3,"system is not 3 dimensional");
	wlist=sim->wlist;
	rad=0.5*(wlist[3]->pos-wlist[2]->pos);
	length=wlist[1]->pos-wlist[0]->pos;
	pos[0]=wlist[0]->pos;
	pos[1]=0.5*(wlist[2]->pos+wlist[3]->pos);
	pos[2]=0.5*(wlist[4]->pos+wlist[5]->pos);
	for(ll=0;ll<sim->mols->nlist;ll++) {
		mlist=sim->mols->live[ll];
		nmol=sim->mols->nl[ll];
		for(m=0;m<nmol;m++)
			if(!insideecoli(mlist[m]->pos,pos,rad,length)) {
				if(insideecoli(mlist[m]->posx,pos,rad,length)) copyVD(mlist[m]->posx,mlist[m]->pos,3);
				else putinecoli(mlist[m]->pos,pos,rad,length); }}
	return CMDok; }


enum CMDcode cmdsetreactionratemolcount(simptr sim,cmdptr cmd,char *line2) {
	int itct,order,r,count,er,*index;
  char nm1[STRCHAR];
  rxnptr rxn;
  double rate,coeff;
  enum MolecState ms;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	SCMDCHECK(line2,"missing argument");
	itct=sscanf(line2,"%s %lg",nm1,&coeff);
  SCMDCHECK(itct==2,"missing reaction name or coefficient c0");
  r=readrxnname(sim,nm1,&order,&rxn);
  SCMDCHECK(r>=0,"unrecognized reaction name");
  line2=strnword(line2,3);
  rate=coeff;
  while(line2) {
    itct=sscanf(line2,"%lg %s",&coeff,nm1);
    SCMDCHECK(itct==2,"missing coefficient and/or species parameters");
		er=molstring2index1(sim,line2,&ms,&index);
		SCMDCHECK(er!=-1,"species is missing or cannot be read");
		SCMDCHECK(er!=-2,"mismatched or improper parentheses around molecule state");
		SCMDCHECK(er!=-3,"cannot read molecule state value");
		SCMDCHECK(er!=-4,"molecule name not recognized");
		SCMDCHECK(er!=-7,"error allocating memory");
    line2=strnword(line2,3);
    count=molcount(sim,er,index,ms,NULL,-1);
    rate+=coeff*count; }
  if(rate<0) rate=0;
  er=RxnSetValue(sim,"rate",rxn,rate);
  SCMDCHECK(er==0 || er==3,"error setting reaction rate");
  return CMDok; }


enum CMDcode cmdexpandsystem(simptr sim,cmdptr cmd,char *line2) {
	int itct,d,dim,ll,m,s,p,c,k,i,em;
	double expand[DIMMAX],center[DIMMAX],zero[DIMMAX];
	moleculeptr mptr;
	surfaceptr srf;
	enum PanelShape ps;
	enum PanelFace face;
	panelptr pnl;
	compartptr cmpt;

	if(line2 && !strcmp(line2,"cmdtype")) return CMDmanipulate;
	dim=sim->dim;
	SCMDCHECK(line2,"missing arguments");
	itct=strreadnd(line2,dim,expand,NULL);
  SCMDCHECK(itct==dim,"cannot read or wrong number of expansion values");
	systemcenter(sim,center);

	for(ll=0;ll<sim->mols->nlist;ll++)
		if(sim->mols->listtype[ll]==MLTsystem)
			for(m=0;m<sim->mols->nl[ll];m++) {
				mptr=sim->mols->live[ll][m];
				for(d=0;d<dim;d++) {
					mptr->pos[d]=center[d]+(mptr->pos[d]-center[d])*expand[d];
					mptr->posx[d]=center[d]+(mptr->posx[d]-center[d])*expand[d]; }}

	if(sim->srfss) {
		zero[0]=zero[1]=zero[2]=0;
		for(s=0;s<sim->srfss->nsrf;s++) {
			srf=sim->srfss->srflist[s];
			for(ps=(PanelShape)0;ps<(PanelShape)PSMAX;ps=(PanelShape)(ps+1))
				for(p=0;p<srf->npanel[ps];p++) {
					pnl=srf->panels[ps][p];
					surftransformpanel(pnl,sim->dim,zero,center,expand); }
			if(srf->nemitter && srf->nemitter[PFfront] && srf->nemitter[PFback] && sim->mols)
				for(face=(PanelFace)0;face<(PanelFace)2;face=(PanelFace)(face+1))
					for(i=1;i<sim->mols->nspecies;i++)
						for(em=0;em<srf->nemitter[face][i];em++)
							for(d=0;d<dim;d++)
								srf->emitterpos[face][i][em][d]=center[d]+(srf->emitterpos[face][i][em][d]-center[d])*expand[d]; }}

	if(sim->cmptss) {
		for(c=0;c<sim->cmptss->ncmpt;c++) {
			cmpt=sim->cmptss->cmptlist[c];
			for(k=0;k<cmpt->npts;k++)
				for(d=0;d<dim;d++)
					cmpt->points[k][d]=center[d]+(cmpt->points[k][d]-center[d])*expand[d]; }
		compartsetcondition(sim->cmptss,SCparams,0); }

	return CMDok; }



/**********************************************************/
/******************* internal routines ********************/
/**********************************************************/


void cmdv1free(cmdptr cmd) {
	free(cmd->v1);
	return; }


void cmdv1v2free(cmdptr cmd) {
	free(cmd->v1);
	free(cmd->v2);
	return; }


enum CMDcode conditionalcmdtype(simptr sim,cmdptr cmd,int nparam) {
	char string[STRCHAR],*strptr;
	enum CMDcode ans;

	if(!cmd->str) return CMDnone;
	strptr=strnword(cmd->str,nparam+2);
	if(!strptr) return CMDnone;
	strcpy(string,strptr);
	strptr=cmd->str;
	cmd->str=string;
	ans=scmdcmdtype(sim->cmds,cmd);
	cmd->str=strptr;
	return ans; }


int insideecoli(double *pos,double *ofst,double rad,double length) {
	double dist;

	dist=(pos[1]-ofst[1])*(pos[1]-ofst[1])+(pos[2]-ofst[2])*(pos[2]-ofst[2]);
	if(pos[0]-ofst[0]<rad) dist+=(pos[0]-ofst[0]-rad)*(pos[0]-ofst[0]-rad);
	else if(pos[0]-ofst[0]>length-rad) dist+=(pos[0]-ofst[0]-length+rad)*(pos[0]-ofst[0]-length+rad);
	return dist<rad*rad; }


void putinecoli(double *pos,double *ofst,double rad,double length) {
	double dist;

	dist=(pos[1]-ofst[1])*(pos[1]-ofst[1])+(pos[2]-ofst[2])*(pos[2]-ofst[2]);
	if(pos[0]-ofst[0]<rad) {
		dist+=(pos[0]-ofst[0]-rad)*(pos[0]-ofst[0]-rad);
		dist=sqrt(rad*rad/dist);
		pos[0]=ofst[0]+rad+dist*(pos[0]-ofst[0]-rad); }
	else if(pos[0]-ofst[0]>length-rad) {
		dist+=(pos[0]-ofst[0]-length+rad)*(pos[0]-ofst[0]-length+rad);
		dist=sqrt(rad*rad/dist);
		pos[0]=ofst[0]+length-rad+dist*(pos[0]-ofst[0]-length+rad); }
	else
		dist=sqrt(rad*rad/dist);
	pos[1]=ofst[1]+dist*(pos[1]-ofst[1]);
	pos[2]=ofst[2]+dist*(pos[2]-ofst[2]);
	return; }


int molinpanels(simptr sim,int ll,int m,int s,char pshape) {
	int p,ps,dim,npnl;
	double *pos;
	panelptr *pnls,pnl;

	if(s<0) {
		for(s=0;s<sim->srfss->nsrf;s++)
			if(molinpanels(sim,ll,m,s,pshape)) return 1;
		return 0; }

	dim=sim->dim;
	if(pshape=='s') ps=2;
	else return 0;
	pnls=sim->srfss->srflist[s]->panels[ps];
	npnl=sim->srfss->srflist[s]->npanel[ps];
	pos=sim->mols->live[ll][m]->pos;
	if(pshape=='s') {
		for(p=0;p<npnl;p++) {
			pnl=pnls[p];
			if(Geo_PtInSphere(pos,pnl->point[0],pnl->point[1][0],dim)) return 1; }}
	return 0; }




