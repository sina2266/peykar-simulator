/* File rxnparam.h, written by Steven Andrews, 2003.
This code is in the public domain.  It is not copyrighted and may not be
copyrighted.

This is a header file for rxnparam.c.  See rxnparam_doc.doc or rxn_param_doc.pdf
for documentation. */

#ifndef __rxnparam_h
#define __rxnparam_h

/***  LOOK-UP FUNCTIONS FOR REACTION RATES AND BINDING AND UNBINDING RADII  ***/

double numrxnrate(double step,double a,double b);
double actrxnrate(double step,double a);
double bindingradius(double rate,double dt,double difc,double b,int rel);
double unbindingradius(double pgem,double dt,double difc,double a);

/************    FUNCTIONS FOR INVESTIGATING AN ABSORBING SPHERE    ***********/

double rdfabsorb(double *r,double *rdf,int n);
void rdfdiffuse(double *r,double *rdfa,double *rdfd,int n,double step);
void rdfreverserxn(double *r,double *rdf,int n,double step,double b,double flux);
double rdfsteadystate(double *r,double *rdfa,double *rdfd,int n,double step,double b,double eps);
void rdfmaketable();

#endif
