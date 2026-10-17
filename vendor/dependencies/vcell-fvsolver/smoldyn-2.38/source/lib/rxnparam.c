/* File rxnparam.c, written by Steve Andrews, 2003.
This code is in the public domain.  It is not copyrighted and may not be
copyrighted.

This file calculates reaction rates for a stochastic spatial numerical method,
as described in the research paper "Stochastic Simulation of Chemical Reactions
with Spatial Resolution and Single Molecule Detail" written by Steven S. Andrews
and Dennis Bray, which was published in Physical Biology in 2004.  The code
here is written entirely in ANSII C.

History: Started 3/02, converted to a separate file 5/31/03.
  Several updates made and documentation written 4/08.   */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "rxnparam.h"

#define PI 3.14159265358979323846
#define SQRT2PI 2.50662827462

double rxnparam_erfccD(double x);


/******************************************************************************/
/***  LOOK-UP FUNCTIONS FOR REACTION RATES AND BINDING AND UNBINDING RADII  ***/
/******************************************************************************/

/* numrxnrate calculates the bimolecular reaction rate for the simulation
algorithm, based on the rms step length in step, the binding radius in a, and
the unbinding radius in b.  It uses cubic polynomial interpolation on previously
computed data, with interpolation both on the reduced step length (step/a) and
on the reduced unbinding radius (b/a).  Enter a negative value of b for
irreversible reactions.  If the input parameters result in reduced values that
are off the edge of the tabulated data, analytical results are used where
possible.  If there are no analytical results, the data are extrapolated.
Variable components: s is step, i is irreversible, r is reversible with b>a, b
is reversible with b<a, y is a rate.  The returned value is the reaction rate
times the time step delta_t.  In other words, this rate constant is per time
step, not per time unit; it has units of length cubed.  */
double numrxnrate(double step,double a,double b) {
	static const double yi[]={				// 600 pts, eps=1e-5, up&down, error<2.4%
		4.188584,4.188381,4.187971,4.187141,4.185479,4.182243,4.1761835,4.1652925,
		4.146168,4.112737,4.054173,3.95406,3.7899875,3.536844,3.178271,2.7239775,
		2.2178055,1.7220855,1.2875175,0.936562,0.668167,0.470021,0.327102,0.225858,
		0.1551475,0.1061375,0.072355,0.049183,0.0333535,0.022576,0.015257};
	static const double yr[]={				// 500 pts, eps=1e-5, up&down, error<2.2%
		4.188790,4.188790,4.188790,4.188788,4.188785,4.188775,4.188747,4.188665,
		4.188438,4.187836,4.186351,4.182961,4.175522,4.158666,4.119598,4.036667,
		3.883389,3.641076,3.316941,2.945750,2.566064,2.203901,1.872874,1.578674,
		1.322500,1.103003,0.916821,0.759962,0.628537,0.518952,0.428136,
		4.188790,4.188790,4.188789,4.188786,4.188778,4.188756,4.188692,4.188509,
		4.188004,4.186693,4.183529,4.176437,4.160926,4.125854,4.047256,3.889954,
		3.621782,3.237397,2.773578,2.289205,1.831668,1.427492,1.087417,0.811891,
		0.595737,0.430950,0.308026,0.217994,0.152981,0.106569,0.073768,
		4.188790,4.188789,4.188788,4.188783,4.188768,4.188727,4.188608,4.188272,
		4.187364,4.185053,4.179623,4.167651,4.141470,4.082687,3.956817,3.722364,
		3.355690,2.877999,2.353690,1.850847,1.411394,1.050895,0.768036,0.553077,
		0.393496,0.277134,0.193535,0.134203,0.092515,0.063465,0.043358,
		4.188790,4.188789,4.188786,4.188777,4.188753,4.188682,4.188480,4.187919,
		4.186431,4.182757,4.174363,4.156118,4.116203,4.028391,3.851515,3.546916,
		3.110183,2.587884,2.055578,1.574898,1.174608,0.858327,0.617223,0.438159,
		0.307824,0.214440,0.148358,0.102061,0.069885,0.047671,0.032414,
		4.188789,4.188788,4.188783,4.188769,4.188729,4.188614,4.188288,4.187399,
		4.185108,4.179638,4.167499,4.141379,4.084571,3.964576,3.739474,3.381771,
		2.906433,2.372992,1.854444,1.401333,1.032632,0.746509,0.531766,0.374442,
		0.261247,0.180929,0.124557,0.085332,0.058229,0.039604,0.026865,
		4.188789,4.188786,4.188778,4.188756,4.188692,4.188510,4.188002,4.186650,
		4.183288,4.175566,4.158864,4.123229,4.047257,3.896024,3.632545,3.242239,
		2.750962,2.220229,1.717239,1.285681,0.939696,0.674540,0.477621,0.334612,
		0.232460,0.160415,0.110103,0.075242,0.051236,0.034789,0.023565,
		4.188788,4.188784,4.188772,4.188737,4.188637,4.188354,4.187585,4.185607,
		4.180895,4.170490,4.148460,4.102035,4.006907,3.829897,3.541151,3.133335,
		2.635739,2.109597,1.619284,1.204298,0.875185,0.625190,0.440882,0.307818,
		0.213235,0.146800,0.100560,0.068608,0.046655,0.031645,0.021415,
		4.188787,4.188780,4.188761,4.188707,4.188552,4.188124,4.186995,4.184222,
		4.177931,4.164527,4.136619,4.079198,3.967945,3.772932,3.468711,3.050092,
		2.548726,2.026932,1.547039,1.144954,0.828592,0.589866,0.414774,0.288895,
		0.199728,0.137273,0.093903,0.063994,0.043478,0.029467,0.019931,
		4.188785,4.188774,4.188745,4.188661,4.188426,4.187795,4.186203,4.182494,
		4.174471,4.157859,4.123939,4.056910,3.934028,3.727335,3.412145,2.985387,
		2.481620,1.963935,1.492519,1.100533,0.793976,0.563797,0.395615,0.275071,
		0.189896,0.130359,0.089086,0.060661,0.041187,0.027899,0.018863,
		4.188782,4.188766,4.188720,4.188592,4.188244,4.187347,4.185205,4.180486,
		4.170691,4.150875,4.111574,4.037574,3.906673,3.691217,3.367270,2.934386,
		2.429282,1.915212,1.450660,1.066615,0.767733,0.544143,0.381225,0.264724,
		0.182558,0.125209,0.085504,0.058187,0.039490,0.026741,0.018074,
		4.188777,4.188752,4.188683,4.188492,4.187993,4.186777,4.184049,4.178342,
		4.166861,4.144167,4.100756,4.021817,3.884852,3.662169,3.331386,2.893934,
		2.388069,1.877097,1.418051,1.040351,0.747549,0.529083,0.370239,0.256844,
		0.176983,0.121304,0.082791,0.056318,0.038211,0.025871,0.017486,
		4.188770,4.188732,4.188628,4.188352,4.187671,4.186115,4.182830,4.176234,
		4.163261,4.138284,4.092055,4.009206,3.867171,3.638731,3.302580,2.861651,
		2.355385,1.846979,1.392364,1.019841,0.731849,0.517409,0.361743,0.250766,
		0.172684,0.118295,0.080706,0.054883,0.037230,0.025204,0.017035,
		4.188759,4.188702,4.188551,4.188171,4.187294,4.185421,4.181657,4.174292,
		4.160089,4.133434,4.084980,3.998932,3.852801,3.619743,3.279339,2.835771,
		2.329266,1.822929,1.372041,1.003734,0.719559,0.508296,0.355130,0.246037,
		0.169346,0.115966,0.079096,0.053783,0.036482,0.024699,0.016694,
		4.188742,4.188659,4.188449,4.187958,4.186900,4.184765,4.180606,4.172596,
		4.157440,4.129551,4.079183,3.990499,3.841031,3.604270,3.260519,2.814871,
		2.308170,1.803653,1.355971,0.991032,0.709899,0.501150,0.349948,0.242337,
		0.166741,0.114156,0.077855,0.052940,0.035912,0.024316,0.016437,
		4.188719,4.188603,4.188330,4.187736,4.186534,4.184200,4.179711,4.171172,
		4.155254,4.126372,4.074457,3.983648,3.831433,3.591609,3.245083,2.797662,
		2.290945,1.788288,1.343237,0.981003,0.702295,0.495533,0.345879,0.239437,
		0.164709,0.112756,0.076902,0.052295,0.035478,0.024023,0.016239,
		4.188688,4.188536,4.188204,4.187532,4.186227,4.183725,4.178948,4.169962,
		4.153461,4.123737,4.070508,3.977891,3.823388,3.580958,3.232035,2.783281,
		2.277011,1.776032,1.333144,0.973094,0.696303,0.491107,0.342676,0.237173,
		0.163143,0.111689,0.076180,0.051808,0.035151,0.023802,0.016089};
	const double yb[]={							// 100 pts, eps=1e-5, down only
		4.188790,4.188791,4.188791,4.188793,4.188798,4.188814,4.188859,4.188986,
		4.189328,4.190189,4.192213,4.196874,4.208210,4.236983,4.307724,4.473512,
		4.852754,5.723574,7.838600,13.880005,40.362527,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,
		4.188790,4.188791,4.188791,4.188793,4.188798,4.188813,4.188858,4.188982,
		4.189319,4.190165,4.192156,4.196738,4.207879,4.236133,4.305531,4.467899,
		4.838139,5.683061,7.710538,13.356740,36.482501,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,
		4.188790,4.188791,4.188791,4.188793,4.188798,4.188812,4.188854,4.188973,
		4.189292,4.190095,4.191984,4.196332,4.206887,4.233594,4.298997,4.451212,
		4.795122,5.566006,7.352764,11.994360,28.085852,261.834153,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,
		4.188790,4.188791,4.188791,4.188792,4.188797,4.188810,4.188848,4.188956,
		4.189246,4.189978,4.191699,4.195657,4.205243,4.229397,4.288214,4.424093,
		4.726265,5.384525,6.831926,10.241985,19.932261,69.581522,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,
		4.188790,4.188791,4.188791,4.188792,4.188796,4.188807,4.188840,4.188933,
		4.189183,4.189814,4.191300,4.194716,4.202957,4.223593,4.273478,4.387310,
		4.635278,5.155725,6.228070,8.494834,13.840039,30.707995,217.685066,-1,
		-1,-1,-1,-1,-1,-1,-1,
		4.188790,4.188791,4.188791,4.188792,4.188795,4.188803,4.188829,4.188903,
		4.189101,4.189604,4.190789,4.193514,4.200048,4.216255,4.254956,4.342131,
		4.526936,4.897901,5.610205,6.966144,9.704274,16.182908,37.778925,387.192994,
		-1,-1,-1,-1,-1,-1,-1,
		4.188790,4.188790,4.188791,4.188791,4.188793,4.188799,4.188817,4.188866,
		4.189002,4.189348,4.190168,4.192054,4.196535,4.207468,4.233117,4.289818,
		4.406055,4.627980,5.025468,5.719284,6.973164,9.454673,15.165237,32.669841,
		175.505441,-1,-1,-1,-1,-1,-1,
		4.188790,4.188790,4.188790,4.188791,4.188791,4.188794,4.188801,4.188823,
		4.188885,4.189047,4.189439,4.190344,4.192444,4.197336,4.208277,4.231803,
		4.277545,4.359611,4.499274,4.738728,5.168746,5.973516,7.554064,10.974866,
		20.015747,59.729701,-1,-1,-1,-1,-1,
		4.188790,4.188790,4.188790,4.188790,4.188789,4.188788,4.188784,4.188774,
		4.188751,4.188701,4.188602,4.188390,4.187803,4.185972,4.180908,4.169592,
		4.145768,4.102670,4.041148,3.980771,3.961901,4.039620,4.292076,4.858676,
		6.044956,8.672140,15.704344,48.437802,-1,-1,-1,
		4.188790,4.188790,4.188790,4.188789,4.188787,4.188781,4.188764,4.188718,
		4.188599,4.188311,4.187661,4.186201,4.182644,4.173501,4.151495,4.104610,
		4.014334,3.863341,3.650600,3.398722,3.141206,2.906174,2.713737,2.580237,
		2.524103,2.574058,2.785372,3.278275,4.350631,6.897568,14.924070,
		4.188790,4.188790,4.188790,4.188788,4.188784,4.188773,4.188742,4.188656,
		4.188430,4.187878,4.186617,4.183786,4.177000,4.160053,4.120473,4.038061,
		3.886243,3.645077,3.322103,2.951900,2.573084,2.212134,1.883264,1.592189,
		1.339766,1.124235,0.942582,0.791354,0.667141,0.566827,0.487862};
	const double slo=3,sinc=-0.2;								// logs of values; shi=-3
	const double blor=0,bincr=0.2;								// logs of values; bhir=3
	const double blob=0,bincb=0.1;								// actual values; bhib=1
	const int snum=31,bnumr=16,bnumb=11;
	double x[4],y[4],z[4];
	int sindx,bindx,i,j;
	double ans;

	if(step<0||a<0) return -1;
	if(step==0&&b>=0&&b<1) return -1;
	if(step==0) return 0;
	step=log(step/a);
	b/=a;

	sindx=(int)((step-slo)/sinc);
	for(i=0;i<4;i++) x[i]=slo+(sindx-1+i)*sinc;
	z[0]=(step-x[1])*(step-x[2])*(step-x[3])/(-6.0*sinc*sinc*sinc);
	z[1]=(step-x[0])*(step-x[2])*(step-x[3])/(2.0*sinc*sinc*sinc);
	z[2]=(step-x[0])*(step-x[1])*(step-x[3])/(-2.0*sinc*sinc*sinc);
	z[3]=(step-x[0])*(step-x[1])*(step-x[2])/(6.0*sinc*sinc*sinc);

	if(b<0)
		for(ans=i=0;i<4;i++) {
			if(sindx-1+i>=snum) ans+=z[i]*2.0*PI*exp(2.0*x[i]);
			else if(sindx-1+i<0) ans+=z[i]*4.0*PI/3.0;
			else ans+=z[i]*yi[sindx-1+i]; }
	else if(b<1.0) {
		bindx=(int)((b-blob)/bincb);
		if(bindx<1) bindx=1;
		else if(bindx+2>=bnumb) bindx=bnumb-3;
		while(sindx+3>=snum||(sindx>0&&yb[(bindx-1)*snum+sindx+3]<0)) sindx--;
		for(i=0;i<4;i++) x[i]=slo+(sindx-1+i)*sinc;
		z[0]=(step-x[1])*(step-x[2])*(step-x[3])/(-6.0*sinc*sinc*sinc);
		z[1]=(step-x[0])*(step-x[2])*(step-x[3])/(2.0*sinc*sinc*sinc);
		z[2]=(step-x[0])*(step-x[1])*(step-x[3])/(-2.0*sinc*sinc*sinc);
		z[3]=(step-x[0])*(step-x[1])*(step-x[2])/(6.0*sinc*sinc*sinc);
		for(j=0;j<4;j++)
			for(y[j]=i=0;i<4;i++) {
				if(sindx-1+i>=snum) y[j]+=z[i]*yb[(bindx-1+j)*snum];
				else if(sindx-1+i<0) y[j]+=z[i]*4.0*PI/3.0;
				else y[j]+=z[i]*yb[(bindx-1+j)*snum+sindx-1+i]; }
		for(j=0;j<4;j++) x[j]=blob+(bindx-1+j)*bincb;
		z[0]=(b-x[1])*(b-x[2])*(b-x[3])/(-6.0*bincb*bincb*bincb);
		z[1]=(b-x[0])*(b-x[2])*(b-x[3])/(2.0*bincb*bincb*bincb);
		z[2]=(b-x[0])*(b-x[1])*(b-x[3])/(-2.0*bincb*bincb*bincb);
		z[3]=(b-x[0])*(b-x[1])*(b-x[2])/(6.0*bincb*bincb*bincb);
		ans=z[0]*y[0]+z[1]*y[1]+z[2]*y[2]+z[3]*y[3]; }
	else {
		b=log(b);
		bindx=(int)((b-blor)/bincr);
		if(bindx<1) bindx=1;
		else if(bindx+2>=bnumr) bindx=bnumr-3;
		for(j=0;j<4;j++)
			for(y[j]=i=0;i<4;i++) {
				if(sindx-1+i>=snum&&b==0) y[j]+=z[i]*2.0*PI*exp(x[i])*(1.0+exp(x[i]));
				else if(sindx-1+i>=snum) y[j]+=z[i]*2.0*PI*exp(2.0*x[i])*exp(b)/(exp(b)-1.0);
				else if(sindx-1+i<0) y[j]+=z[i]*4.0*PI/3.0;
				else y[j]+=z[i]*yr[(bindx-1+j)*snum+sindx-1+i]; }
		for(j=0;j<4;j++) x[j]=blor+(bindx-1+j)*bincr;
		z[0]=(b-x[1])*(b-x[2])*(b-x[3])/(-6.0*bincr*bincr*bincr);
		z[1]=(b-x[0])*(b-x[2])*(b-x[3])/(2.0*bincr*bincr*bincr);
		z[2]=(b-x[0])*(b-x[1])*(b-x[3])/(-2.0*bincr*bincr*bincr);
		z[3]=(b-x[0])*(b-x[1])*(b-x[2])/(6.0*bincr*bincr*bincr);
		ans=z[0]*y[0]+z[1]*y[1]+z[2]*y[2]+z[3]*y[3]; }
	return ans*a*a*a; }



/* actrxnrate calculates the effective activation limited reaction rate for the
simulation, which is the reaction rate if the radial correlation function is 1
for all r>a.  The returned value needs to be divided by delta_t.  The equation
is ka=4*pi/3*[erfc(sqrt(2)/s)+s*sqrt((2/pi))]+2*sqrt(2*pi)/3*s*(s^2-1)*[exp(-2/s^2)-1].
It was calculated analytically and verified numerically. */
double actrxnrate(double step,double a) {
	double ka;

	if(step<0||a<=0) return -1;
	if(step==0) return 0;
	step/=a;
	ka=4.0*PI/3.0*(rxnparam_erfccD(sqrt(2.0)/step)+step*sqrt(2.0/PI));
	ka+=2.0*SQRT2PI/3.0*step*(step*step-1.0)*(exp(-2.0/step/step)-1.0);
	return ka*a*a*a; }


/* bindingradius returns the binding radius that corresponds to some given
information.  rate is the actual rate constant (not reduced), dt is the time
step, and difc is the mutual diffusion constant (sum of reactant diffusion
constants).  If b is -1, the reaction is assumed to be irreversible; if b>=0 and
rel=0, then the b value is used as the unbinding radius; and if b>=0 and rel=1,
then the b value is used as the ratio of the unbinding to binding radius, b/a.
This algorithm executes a simple search from numrxnrate, based on the fact that
reaction rates monotonically increase with increasing a, for all the b value
possibilities.  The return value is usually the binding radius.  However, a
value of -1 signifies illegal input parameters.
Modified 2/22/08 to allow for dt==0. */
double bindingradius(double rate,double dt,double difc,double b,int rel) {
	double a,lo,dif,step;
	int n;

	if(rate<0||dt<0||difc<=0) return -1;
	if(rate==0) return 0;
	if(dt==0) {
		if(b<0) return rate/(4*PI*difc);
		if(rel&&b>1) return rate*(1-1/b)/(4*PI*difc);
		if(rel&&b<=1) return -1;
		if(b>0) return rate/(4*PI*difc+rate/b);
		return -1; }
	step=sqrt(2.0*difc*dt);
	lo=0;
	a=step;
	while(numrxnrate(step,a,rel?b*a:b)<rate*dt) {
		lo=a;
		a*=2.0; }
	dif=a-lo;
	for(n=0;n<15;n++) {
		dif*=0.5;
		a=lo+dif;
		if(numrxnrate(step,a,rel?b*a:b)<rate*dt) lo=a; }
	a=lo+0.5*dif;
	return a; }


/* unbindingradius returns the unbinding radius that corresponds to the geminate
reaction probability in pgem, the time step in dt, the mutual diffusion constant
in difc, and the binding radius in a.  Illegal inputs result in a return value
of -2.  If the geminate binding probability can be made as high as that
requested, the corresponding unbinding radius is returned.  Otherwise, the
negative of the maximum achievable pgem value is returned.
Modified 2/25/08 to allow for dt==0.  */
double unbindingradius(double pgem,double dt,double difc,double a) {
	double b,lo,dif,step,ki,kmax;
	int n;

	if(pgem<=0||pgem>=1||difc<=0||a<=0||dt<0) return -2;
	if(dt==0) return a/pgem;
	step=sqrt(2.0*difc*dt);
	ki=numrxnrate(step,a,-1);
	kmax=numrxnrate(step,a,0);
	if(1.0-ki/kmax<pgem) return ki/kmax-1.0;
	lo=0;
	b=a;
	while(1.0-ki/numrxnrate(step,a,b)>pgem) {
		lo=b;
		b*=2.0; }
	dif=b-lo;
	for(n=0;n<15;n++) {
		dif*=0.5;
		b=lo+dif;
		if(1.0-ki/numrxnrate(step,a,b)>pgem) lo=b; }
	b=lo+0.5*dif;
	return b; }


/******************************************************************************/
/*************    UTILITY FUNCTION, COPIED FROM MY MATH2.H FILE    ************/
/******************************************************************************/


double rxnparam_erfccD(double x) {
	double t,z,ans;

	z=fabs(x);
	t=1.0/(1.0+0.5*z);
	ans=t*exp(-z*z-1.26551223+t*(1.00002368+t*(0.37409196+t*(0.09678418+t*(-0.18628806+t*(0.27886807+t*(-1.13520398+t*(1.48851587+t*(-0.82215223+t*0.17087277)))))))));
	return x>=0.0?ans:2.0-ans; }


/******************************************************************************/
/************    FUNCTIONS FOR INVESTIGATING AN ABSORBING SPHERE    ***********/
/******************************************************************************/

/* rdfabsorb integrates the radial diffusion function (rdf) for 0<=r<=1, sets
those values to 0, and returns the integral.  r is a vector of radii; r[0] may
equal zero but that is not required; if not, then it is assumed that the rdf has
zero slope at the origin.
   Integration uses a spherical version of the trapezoid rule: at positions r0
and r1, the function f has values f0 and f1, leading to the linear interpolation
f=[(r-r0)f1+(r1-r)f0]/(r1-r0).  Its integral is A=Integral[4*pi*r^2*f(r)*dr]
A=4*pi/(r1-r0)*[(f1-f0)/4*(r1^4-r0^4)+(r1f0-r0f1)/3*(r1^3-r0^3)]
A=pi*(f1-f0)(r1+r0)(r1^2+r0^2)+4*pi/3*(r1f0-r0f1)(r1^2+r1r0+r0^2)
The left end of the integral assumes zero slope for the rdf.  The right end does
not terminate exactly at 1, but includes the upper left triangle of the final
trapezoid.  That way, if there are two absorptions in a row, the second one will
return an integral of 0, and area is properly conserved.  The problem is that it
does not terminate exactly at 1.  Furthermore, the correct relative location of
1 between two r[j] points depends on the function.  The best solution is to use
an unevenly spaced r[j] vector, with a very narrow separation about 1 and no
r[j] equal to 1.   */
double rdfabsorb(double *r,double *rdf,int n) {
	int j;
	double sum,r0,r1,f0,f1;

	r0=r1=0;
	f1=rdf[0];
	sum=0;
	for(j=(r[0]==0?1:0);r1<1&&j<n;j++) {
		r0=r1;
		f0=f1;
		r1=r[j];
		f1=rdf[j];
		sum+=PI*(f1-f0)*(r1+r0)*(r1*r1+r0*r0)+4.0*PI/3.0*(r1*f0-r0*f1)*(r1*r1+r1*r0+r0*r0); }
	f0=0;
	sum-=PI*(f1-f0)*(r1+r0)*(r1*r1+r0*r0)+4.0*PI/3.0*(r1*f0-r0*f1)*(r1*r1+r1*r0+r0*r0);
	for(j-=2;j>=0;j--) rdf[j]=0;
	return sum; }


/* rdfdiffuse integrates the radial distribution function with the Green's
function for radially symmetric diffusion to implement diffusion over a fixed
time step.  r is a vector of radii, rdfa is the input rdf, rdfd is the output
rdf, n is the number of points, and step is the rms step length, equal to
(2Dt)^1/2.  r[0] may equal 0 but it is not required.  It is assumed that rdfa
has zero slope at r=0.  The boundary condition on the large r side is that the
function tends to 1 with a functional form 1+a2/r, for large r.  This is
accomplished by fitting the 10% largest r portion of the rdf with the function
1+a2/r.  After the integral over the tabulated data is complete, the rdf is
integrated on to infinity using the previous fit information and an analytical
result for that integral.  The numerical portion of the integral is carried out
exactly like the one in absorb but with a different integrand, which is
c(r)=Integrate[4*pi*r'^2*rdfa(r')*grn(r,r')dr'].  grn(r,r') is the Green's function, equal
to grn(r,r')=1/(4*pi*r*r')[G_step(r-r')-G_step(r+r')] and G_s(x) is a normalized
Gaussian with mean 0 and standard deviation s. */
void rdfdiffuse(double *r,double *rdfa,double *rdfd,int n,double step) {
	int i,j;
	double grn,sum,f0,f1,rr,r0,r1,alpha,beta,a2,erfcdif,erfcsum;

	alpha=beta=0;
	for(i=(int)(0.9*n);i<n;i++) {
		alpha+=1.0/r[i]/r[i];
		beta+=(rdfa[i]-1.0)/r[i]; }
	a2=beta/alpha/step;

	grn=0;
	if(r[i=0]==0) {
		rr=r1=f1=sum=0;
		for(j=1;j<n;j++) {
			r0=r1;
			f0=f1;
			r1=r[j]/step;
			grn=exp(-r1*r1/2.0)/(2.0*PI*SQRT2PI);
			f1=(rdfa[j]-rdfa[0])*grn;
			sum+=PI*(f1-f0)*(r1+r0)*(r1*r1+r0*r0)+4.0*PI/3.0*(r1*f0-r0*f1)*(r1*r1+r1*r0+r0*r0); }
		sum+=(1.0-rdfa[0])*(4.0*PI*r1*grn+rxnparam_erfccD(r1/sqrt(2.0)));
		rdfd[i++]=sum+rdfa[0]; }
	for(;i<n;i++) {
		rr=r[i]/step;
		r1=0;
		grn=exp(-rr*rr/2.0)/(2.0*PI*SQRT2PI);
		f1=(rdfa[0]-rdfa[i])*grn;
		sum=0;
		for(j=(r[0]==0?1:0);j<n;j++) {
			r0=r1;
			f0=f1;
			r1=r[j]/step;
			grn=1.0/rr/r1*(exp(-(rr-r1)*(rr-r1)/2.0)-exp(-(rr+r1)*(rr+r1)/2.0))/(4.0*PI*SQRT2PI);
			f1=(rdfa[j]-rdfa[i])*grn;
			sum+=PI*(f1-f0)*(r1+r0)*(r1*r1+r0*r0)+4.0*PI/3.0*(r1*f0-r0*f1)*(r1*r1+r1*r0+r0*r0); }
		erfcdif=rxnparam_erfccD((r1-rr)/sqrt(2.0));
		erfcsum=rxnparam_erfccD((r1+rr)/sqrt(2.0));
		sum+=(1.0-rdfa[i])*(4.0*PI*r1*grn+1.0/2.0*(erfcdif+erfcsum))+a2/2.0/rr*(erfcdif-erfcsum);
		rdfd[i]=sum+rdfa[i]; }
	return; }


/* Analysis of the reverse reaction involves adding a delta function to the rdf
and then convolving with the Green's function.  However, this leads to
numerical errors, although it is trivial analytically.  The function
rdfreverserxn is the analytic solution.  It adds the diffusion Green's function
to the rdf, based on a delta function at b, and after one diffusion step.  r is
a list of radii, rdf is the rdf, step is the rms step length, b is the delta
function point (which does not have to be equal or unequal to a r[j] value), and
flux is the area of the delta function. */
void rdfreverserxn(double *r,double *rdf,int n,double step,double b,double flux) {
	int i;
	double rr,k;

	k=1.0/(4.0*PI*SQRT2PI*step*step*step);
	if(b==0) {
		for(i=0;i<n;i++) {
			rr=r[i]/step;
			rdf[i]+=flux*k*2.0*exp(-rr*rr/2.0); }}
	else {
		b/=step;
		if(r[i=0]==0) rdf[i++]+=flux*k*2.0*exp(-b*b/2.0);
		for(;i<n;i++) {
			rr=r[i]/step;
			rdf[i]+=flux*k/rr/b*(exp(-(rr-b)*(rr-b)/2.0)-exp(-(rr+b)*(rr+b)/2.0)); }}
	return; }


/* rdfsteadystate calculates the radial distribution function (rdf) for alternating
absorption and diffusion steps, for either irreversible or reversible reactions.
r is a vector of radii, rdfa is input as a trial rdf and output as the result
after absorption, rdfd is ignored on input but is output as the rdf after
diffusion, n is the number of elements in the vectors, step is the rms step
length, and b is either <0 if the reaction is irreversible or is the unbinding
radius if the reaction is reversible.  It executes until the fractional
difference between successive steps is less than eps, but at least 30 times and
no more than maxit times.  It can also exit if it iterates more than maxit times
before converging or if the flux exceeds maxflux; if either of these happens,
the function returns -1. */
double rdfsteadystate(double *r,double *rdfa,double *rdfd,int n,double step,double b,double eps) {
	const int maxit=100000;
	const double maxflux=1e7;
	int i,it;
	double flux,fluxold;

	rdfdiffuse(r,rdfa,rdfd,n,step);
	flux=fluxold=rdfabsorb(r,rdfd,n);
	for(it=0;it<30||(it<maxit&&flux<maxflux&&fabs((flux-fluxold)/(fluxold+1e-20))>eps);it++) {
		fluxold=flux;
		rdfdiffuse(r,rdfa,rdfd,n,step);
		if(b>=0) rdfreverserxn(r,rdfd,n,step,b,flux);
		for(i=0;i<n;i++) rdfa[i]=rdfd[i];
		flux=rdfabsorb(r,rdfa,n); }
	if(it>=maxit||flux>=maxflux) flux=-1;
	return flux; }


/* rdfmaketable is used to create data tables of reaction rates, including those
used above in numrxnrate.  All input is requested from the user using the
standard input and all output is sent to standard output.
Runtime is about 1 minute with mode i, 200 pts, eps=1e-4 */
void rdfmaketable() {
	double slo=exp(-3.0),shi=exp(3.0),sinc=exp(0.2);		// step size low, high, increment
	const double blor=exp(0.0),bhir=exp(3.0),bincr=exp(0.2);	// b value low, high, increment for b>a
	const double blob=0,bhib=1.0,bincb=0.1;					// b value low, high, increment for b<a
	double *r,*rdfa,*rdfd,dr,s,b,flux,eps;
	int i,n,done;
	char mode,dir,dirs[256];

	printf("Function for calculating radial diffusion functions (rdf) and reactive\n");
	printf("fluxes for alternating reaction and diffusion steps.  This module\n");
	printf("can operate in several modes.  Enter (i) for irreversible reactions\n");
	printf("(r) for reversible reactions with the unbinding radius larger than\n");
	printf("the binding radius, or (b) for other reversible reactions.  Enter this\n");
	printf("mode in upper case for machine readable output.\n");
	printf("Operation mode: ");
	scanf("%c",&mode);
	printf("Enter the number of radial points in the rdf (e.g. 200): ");
	scanf("%i",&n);
	if(n<10) {
		printf("Value is too low.  Function stopped.\n");return; }
	printf("Enter level of precision (e.g. 1e-4): ");
	scanf("%lf",&eps);
	if(eps<=0) {
		printf("Impossible precision.  Function stopped.\n");return; }
	printf("Enter u for increasing step lengths, d for decreasing: ");
	scanf("%s",dirs);
	dir=dirs[0];
	if(dir=='d') {
		s=slo;slo=shi;shi=s;
		sinc=1.0/sinc; }

	r=(double*)calloc(n,sizeof(double));
	rdfa=(double*)calloc(n,sizeof(double));
	rdfd=(double*)calloc(n,sizeof(double));
	if(!r||!rdfa||!rdfd) {
		printf("Out of memory.  Function stopped.\n");return; }

	if(mode=='i'||mode=='I') b=-1;
	else if(mode=='r'||mode=='R') b=blor;
	else b=blob;
	done=0;
	if(mode=='i') printf("step     flux\n");
	else if(mode=='r'||mode=='b') printf("b      step       flux\n");
	else printf("\n");

	while(!done) {
		if(mode=='i'||mode=='I') dr=10.0/n;						// r max is set to 10 for mode i
		else if(mode=='r'||mode=='R') dr=(b+3.0)/n;		// r max is set to b+3 for mode r
		else dr=5.0/n;																// r max is set to 5 for mode b
		r[0]=0;																				// r min is set to 0
		for(i=1;r[i-1]<1&&i-1<n;i++) r[i]=r[i-1]+dr;
		r[i-1]=0.9999;																// r point below 1 is set to 0.9999
		r[i++]=1.0001;																// r point above 1 is set to 1.0001
		for(;i<n;i++) r[i]=r[i-1]+dr;

		for(i=0;i<n&&r[i]<1;i++) rdfa[i]=0;
		if(dir=='u'&&(mode=='i'||mode=='I')) for(;i<n;i++) rdfa[i]=1.0-1.0/r[i];
		else if(dir=='u'&&(mode=='r'||mode=='R'))
			for(;i<n&&r[i]<b;i++) rdfa[i]=1.0-(b-r[i])/r[i]/(b-1.0);
		for(;i<n;i++) rdfa[i]=1.0;

		flux=0;
		for(s=slo;slo<shi?s<shi*sqrt(sinc):s>shi*sqrt(sinc);s*=sinc) {
			if(flux>=0) flux=rdfsteadystate(r,rdfa,rdfd,n,s,b,eps);
			if(mode=='i') printf("%lf %lf\n",s,flux);
			else if(mode=='r'||mode=='b') printf("%lf %lf %lf\n",b,s,flux);
			else printf("%lf,",fabs(flux)); }
		printf("\n");

		if(mode=='i'||mode=='I') done=1;
		else if(mode=='r'||mode=='R') {
			b*=bincr;
			if(b>bhir*sqrt(bincr)) done=1; }
		else {
			b+=bincb;
			if(b>bhib+bincb/2.0) done=1; }}
	free(r);
	free(rdfa);
	free(rdfd);
	return; }



