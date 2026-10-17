/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations 
whose solutions have discontinuities.  


Copyright (C) 1999 by The University at Stony Brook. 
 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/************************************************************************************
FronTier is a set of libraries that implements differnt types of Front Traking algorithms.
Front Tracking is a numerical method for the solution of partial differential equations 
whose solutions have discontinuities.  


Copyright (C) 1999 by The University at Stony Brook. 
 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/


/*
*				quad():
*
*	Contains routines for the evalution of definite integrals.
*/

#include <cdecs.h>
#include <vmalloc.h>

LOCAL	const double TINY = 5e-29;

/*
*                  	dqng():
*
*       ***date written   19800101   (yyyymmdd)
*       ***revision date  19810101   (yyyymmdd)
*       ***category no.  h2a1a1
*       ***keywords  automatic integrator, smooth integrand,
*             non-adaptive, gauss-kronrod(patterson)
*       ***author  piessens,robert,appl. math. & progr. div. - k.u.leuven
*           de doncker,elise,appl math & progr. div. - k.u.leuven
*           kahaner,david,nbs - modified (2/82)
*       ***purpose  the routine calculates an approximation result to a
*            given definite integral i = integral of f over (a,b),
*            hopefully satisfying following claim for accuracy
*            abs(i-result).le.max(epsabs,epsrel*abs(i)).
*
*       ***description
*
*       non-adaptive integration
*
*           f      - double
*                    function subprogram defining the integrand function 
*
*                    f(x). the actual name for f needs to be declared
*                    e x t e r n a l in the driver program.
*
*           a      - double
*                    lower limit of integration
*
*           b      - double
*                    upper limit of integration
*
*           epsabs - double
*                    absolute accuracy requested
*           epsrel - double
*                    relative accuracy requested
*                    if  epsabs.le.0
*                    and epsrel.lt.max(50*rel.mach.acc.,0.5d-28),
*                    the routine will end with ier = INVALID_EPSILON.
*
*       on return
*           result - double
*                    approximation to the integral i
*                    result is obtained by applying the 21-point
*                    gauss-kronrod rule (res21) obtained by optimal
*                    addition of abscissae to the 10-point gauss rule
*                    (res10), or by applying the 43-point rule (res43)
*                    obtained by optimal addition of abscissae to the
*                    21-point gauss-kronrod rule, or by applying the
*                    87-point rule (res87) obtained by optimal addition 
*                    of abscissae to the 43-point rule.
*
*           abserr - double
*                    estimate of the modulus of the absolute error,
*                    which should equal or exceed abs(i-result)
*
*           neval  - integer
*                    number of integrand evaluations
*
*           ier    - ier = ACCURACY_INTEGRAL
*                            normal and reliable termination of the routine.
*			     it is assumed that
*			     the requested accuracy has been achieved.
*                    ier = INACCURATE_INTEGRAL
*                            abnormal termination of the routine. it is 
*                            assumed that the requested accuracy has
*                            not been achieved. The maximum number of steps
*			     has been executed. the integral is probably too
*                            difficult to be calculated by dqng.
*                    ier = INVALID_EPSILON
*                            the input is invalid, because
*                            epsabs.le.0 and
*                            epsrel.lt.max(50*rel.mach.acc.,0.5d-28).
*                            result is -HUGE_VAL, abserr is HUGE_VAL
*			     and neval is zero.
*
*	***end prologue  dqng
*/



/*
*	the following data statements contain the
*           abscissae and weights of the integration rules used.
*
*           x1      abscissae common to the 10-, 21-, 43- and 87-
*                   point rule
*           x2      abscissae common to the 21-, 43- and 87-point rule
*           x3      abscissae common to the 43- and 87-point rule
*           x4      abscissae of the 87-point rule
*           w10     weights of the 10-point formula
*           w21a    weights of the 21-point formula for abscissae x1
*           w21b    weights of the 21-point formula for abscissae x2
*           w43a    weights of the 43-point formula for abscissae x1, x3 
*
*           w43b    weights of the 43-point formula for abscissae x3
*           w87a    weights of the 87-point formula for abscissae x1,
*                   x2, x3
*           w87b    weights of the 87-point formula for abscissae x4
*/


/*
*	gauss-kronrod-patterson quadrature coefficients for use in
*	quadpack routine qng.  these coefficients were calculated with
*	101 decimal digit arithmetic by l. w. fullerton, bell labs, nov 1981. 
*/

/*           list of major variables
*           -----------------------
*
*           centr  - mid point of the integration interval
*           hlgth  - half-length of the integration interval
*           fcentr - function value at mid point
*           absc   - abscissa
*           fval   - function value
*           savfun - array of function values which have already been
*                    computed
*           res10  - 10-point gauss result
*           res21  - 21-point kronrod result
*           res43  - 43-point result
*           res87  - 87-point result
*           resabs - approximation to the integral of abs(f)
*           resasc - approximation to the integral of abs(f-i/(b-a))
*
*           machine dependent constants
*           ---------------------------
*
*           epmach is the largest relative spacing.
*           uflow is the smallest positive magnitude.
*/

EXPORT double dqng(
	double             (*f)(double,POINTER),
	POINTER           prms,
	double            a,
	double            b,
	double            epsabs,
	double            epsrel,
	double             *pabserr,
	int               *pneval,
	QUADRATURE_STATUS *pier)
{
	double result, abserr;
	double sresult, sabserr;
	int   neval, sneval;
	QUADRATURE_STATUS ier, sier;
	/* Initialized data */

	static const double x1[5] = {
	    .973906528517171720077964012084452,
	    .865063366688984510732096688423493,
	    .679409568299024406234327365114874,
	    .433395394129247190799265943165784,
	    .14887433898163121088482600112972 };
	static const double w87a[21] = {
	    .00814837738414917290000287844819,
	    .018761438201562822243935059003794,
	    .027347451050052286161582829741283,
	    .033677707311637930046581056957588,
	    .036935099820427907614589586742499,
	    .002884872430211530501334156248695,
	    .013685946022712701888950035273128,
	    .023280413502888311123409291030404,
	    .030872497611713358675466394126442,
	    .035693633639418770719351355457044,
	   9.15283345202241360843392549948e-4,
	    .005399280219300471367738743391053,
	    .010947679601118931134327826856808,
	    .01629873169678733526266570322328,
	    .02108156888920383511243306018819,
	    .02537096976925382724346799983171,
	    .02918969775647575250144615408492,
	    .032373202467202789685788194889595,
	    .034783098950365142750781997949596,
	    .036412220731351787562801163687577,
	    .037253875503047708539592001191226 };
	static const double w87b[23] = {
	   2.74145563762072350016527092881e-4,
	    .001807124155057942948341311753254,
	    .00409686928275916486445807068348,
	    .006758290051847378699816577897424,
	    .009549957672201646536053581325377,
	    .01232944765224485369462663996378,
	    .015010447346388952376697286041943,
	    .0175489679862431910996653529259,
	    .019938037786440888202278192730714,
	    .022194935961012286796332102959499,
	    .024339147126000805470360647041454,
	    .026374505414839207241503786552615,
	    .02828691078877120065996800298796,
	    .030052581128092695322521110347341,
	    .031646751371439929404586051078883,
	    .033050413419978503290785944862689,
	    .034255099704226061787082821046821,
	    .035262412660156681033782717998428,
	    .036076989622888701185500318003895,
	    .036698604498456094498018047441094,
	    .037120549269832576114119958413599,
	    .037334228751935040321235449094698,
	    .037361073762679023410321241766599 };
	static const double w10[5] = {
	    .066671344308688137593568809893332,
	    .149451349150580593145776339657697,
	    .219086362515982043995534934228163,
	    .269266719309996355091226921569469,
	    .295524224714752870173892994651338 };
	static const double x2[5] = {
	    .995657163025808080735527280689003,
	    .930157491355708226001207180059508,
	    .780817726586416897063717578345042,
	    .562757134668604683339000099272694,
	    .294392862701460198131126603103866 };
	static const double w21a[5] = {
	    .03255816230796472747881897245939,
	    .07503967481091995276704314091619,
	    .109387158802297641899210590325805,
	    .134709217311473325928054001771707,
	    .147739104901338491374841515972068 };
	static const double w21b[6] = {
	    .011694638867371874278064396062192,
	    .05475589657435199603138130024458,
	    .093125454583697605535065465083366,
	    .123491976262065851077958109831074,
	    .142775938577060080797094273138717,
	    .149445554002916905664936468389821 };
	static const double x3[11] = {
	    .999333360901932081394099323919911,
	    .987433402908088869795961478381209,
	    .954807934814266299257919200290473,
	    .900148695748328293625099494069092,
	    .82519831498311415084706673258852,
	    .732148388989304982612354848755461,
	    .622847970537725238641159120344323,
	    .499479574071056499952214885499755,
	    .364901661346580768043989548502644,
	    .222254919776601296498260928066212,
	    .074650617461383322043914435796506 };
	static const double w43a[10] = {
	    .016296734289666564924281974617663,
	    .037522876120869501461613795898115,
	    .054694902058255442147212685465005,
	    .067355414609478086075553166302174,
	    .073870199632393953432140695251367,
	    .005768556059769796184184327908655,
	    .027371890593248842081276069289151,
	    .046560826910428830743339154433824,
	    .061744995201442564496240336030883,
	    .071387267268693397768559114425516 };
	static const double w43b[12] = {
	    .001844477640212414100389106552965,
	    .010798689585891651740465406741293,
	    .021895363867795428102523123075149,
	    .032597463975345689443882222526137,
	    .042163137935191811847627924327955,
	    .050741939600184577780189020092084,
	    .058379395542619248375475369330206,
	    .064746404951445885544689259517511,
	    .069566197912356484528633315038405,
	    .072824441471833208150939535192842,
	    .074507751014175118273571813842889,
	    .074722147517403005594425168280423 };
	static const double x4[22] = {
	    .999902977262729234490529830591582,
	    .99798989598667874542749632236596,
	    .992175497860687222808523352251425,
	    .981358163572712773571916941623894,
	    .965057623858384619128284110607926,
	    .943167613133670596816416634507426,
	    .91580641468550720959182643072005,
	    .883221657771316501372117548744163,
	    .845710748462415666605902011504855,
	    .803557658035230982788739474980964,
	    .75700573068549555832894279343202,
	    .70627320978732181982409427474084,
	    .651589466501177922534422205016736,
	    .593223374057961088875273770349144,
	    .531493605970831932285268948562671,
	    .46676362304202284487196678165927,
	    .399424847859218804732101665817923,
	    .329874877106188288265053371824597,
	    .258503559202161551802280975429025,
	    .185695396568346652015917141167606,
	    .111842213179907468172398359241362,
	    .037352123394619870814998165437704 };

	/* Local variables */
	double absc, fval, res10, res21, res43, res87, fval1, fval2;
	double hlgth, centr, reskh;

	const double epmach = 50.0*DBL_EPSILON;
	const double uflow = DBL_MIN;
	double dhlgth, resabs, resasc, fcentr, savfun[21],
	       fv1[5], fv2[5], fv3[5], fv4[5];
	int    k, l;
	int    ipx;


/* ***first executable statement  dqng */

/*           test on validity of parameters */
/*           ------------------------------ */

	result = -HUGE_VAL;
	abserr = HUGE_VAL;
	neval = 0;
	if (epsabs <= 0. && epsrel < max(epmach,TINY))
	{
	    (void) printf("WARNING in dqng(), invalid epsilons"
	                  "epsabs = %24.20g, epsrel = %24.20g\n",epsabs,epsrel);
	    ier = INVALID_EPSILON;
	    if (pabserr)
	        *pabserr = abserr;
	    if (pneval)
	        *pneval = neval;
	    if (pier)
	        *pier = ier;
	    return -HUGE_VAL;
	}
	hlgth = 0.5*(b - a);
	dhlgth = fabs(hlgth);
	centr = 0.5*(b + a);
	fcentr = (*f)(centr,prms);
	neval = 21;

	ier = INACCURATE_INTEGRAL;
	for (l = 0; l < 3; ++l)
	{
	    switch (l)
	    {
	    case 0:
            /* compute the integral using the 10- and 21-point formula. */
	        res10 = 0.0;
	        res21 = w21b[5] * fcentr;
	        resabs = w21b[5] * fabs(fcentr);
	        for (k = 0; k < 5; ++k)
	        {
	            absc = hlgth * x1[k];
	            fval1 = (*f)(centr + absc,prms);
	            fval2 = (*f)(centr - absc,prms);
	            fval = fval1 + fval2;
	            res10 += w10[k] * fval;
	            res21 += w21a[k] * fval;
	            resabs += w21a[k] * (fabs(fval1) + fabs(fval2));
	            savfun[k] = fval;
	            fv1[k] = fval1;
	            fv2[k] = fval2;
	        }
	        ipx = 5;
	        for (k = 0; k < 5; ++k)
	        {
	            ++ipx;
	            absc = hlgth * x2[k];
	            fval1 = (*f)(centr + absc,prms);
	            fval2 = (*f)(centr - absc,prms);
	            fval = fval1 + fval2;
	            res21 += w21b[k] * fval;
	            resabs += w21b[k] * (fabs(fval1) + fabs(fval2));
	            savfun[ipx - 1] = fval;
	            fv3[k] = fval1;
	            fv4[k] = fval2;
	        }

                /* test for convergence. */
	        result = res21 * hlgth;
	        resabs *= dhlgth;
	        reskh = res21 * 0.5;
	        resasc = w21b[5] * fabs(fcentr - reskh);
	        for (k = 0; k < 5; ++k)
	        {
	            resasc += w21a[k]*(fabs(fv1[k]-reskh) + fabs(fv2[k]-reskh)) +
		              w21b[k]*(fabs(fv3[k]-reskh) + fabs(fv4[k]-reskh));
	        }
	        abserr = fabs((res21-res10)*hlgth);
	        resasc *= dhlgth;
	        break;

	    case 1:
            /* compute the integral using the 43-point formula. */

	        res43 = w43b[11] * fcentr;
	        neval = 43;
	        for (k = 0; k < 10; ++k)
	            res43 += savfun[k] * w43a[k];
	        for (k = 0; k < 11; ++k)
	        {
	            ++ipx;
	            absc = hlgth * x3[k];
	            fval = (*f)(absc + centr,prms) + (*f)(centr - absc,prms);
	            res43 += fval * w43b[k];
	            savfun[ipx - 1] = fval;
	        }

                /* test for convergence. */
	        result = res43 * hlgth;
	        abserr = fabs((res43 - res21)*hlgth);
	        break;

	    case 2:
            /* compute the integral using the 87-point formula. */

	        res87 = w87b[22] * fcentr;
	        neval = 87;
	        for (k = 0; k < 21; ++k)
	            res87 += savfun[k]*w87a[k];
	        for (k = 0; k < 22; ++k)
	        {
	            absc = hlgth * x4[k];
	            res87 += w87b[k]*((*f)(absc+centr,prms) +
			              (*f)(centr-absc,prms));
	        }
	        result = res87 * hlgth;
	        abserr = fabs((res87 - res43)*hlgth);
	        break;
	    default:
	        break;
	    }

	    if (resasc != 0.0 && abserr != 0.0)
	        abserr = resasc * min(1.0,pow(abserr*200.0, 1.5));
	    if (resabs > uflow/epmach)
	        abserr = max(epmach*resabs,abserr);
	    if (abserr <= max(epsabs,epsrel*fabs(result)))
	    {
		ier = ACCURATE_INTEGRAL;
		if (pier)
	            *pier = ier;
		if (pabserr)
	            *pabserr = abserr;
		if (pneval)
	            *pneval = neval;
	        return result;
	    }
	}
#if DONT_COMPILE
        (void) printf("WARNING in dqng(), non-convergent integral "
		      "attempting Simpson's rule\n");
	(void) printf("Current abserr = %24.20g, neval = %d\n",abserr,neval);
	(void) printf("Current result = %24.20g\n",result);
#endif /*DONT_COMPILE*/
	sresult = SimpRule(f,prms,a,b,epsabs,epsrel,&sabserr,&sneval,&sier);
	if (pneval)
	    *pneval = neval+sneval;
	if (sier == ACCURATE_INTEGRAL)
	{
	    result = sresult;
	    ier = ier;
	    abserr = sabserr;
	}
	if (pier)
	    *pier = ier;
	if (pabserr)
	    *pabserr = abserr;
	return  result;
}		/*end dqng */

/*
*			SimpRule():
*
*	Uses Simpson's rule on successively finer grids to evaluate the
*	definite intergral of f(x) on the interval [a, b].
*/

/*ARGSUSED*/
EXPORT double SimpRule(
	double             (*f)(double,POINTER),
	POINTER           prms,
	double            a,
	double            b,
	double            epsabs,
	double            epsrel,
	double             *pabserr,
	int               *pneval,
	QUADRATURE_STATUS *pier)
{
    	double             result;
	double             fv1, fv2, fv;
	static double      *T;	/*Trapozoidal Rule Results for f*/
	static double      *M;	/*Midpoint Rule Results for f*/
	static double      *S;	/*Simpson's Rule Results for f*/
	static double      *aT;	/*Trapozoidal Rule Results of |f|*/
	static double      *aM;	/*Midpoint Rule Results of |f|*/
	static double      *aS;	/*Simpson's Rule Results |f|*/

	const double      epmach = 50.0*DBL_EPSILON;
	const double      uflow = DBL_MIN;
	const int         kmax = 21;
	int               k, j, neval;
	int               twok;
	double             abserr, dx;
	QUADRATURE_STATUS ier;

	if (T == NULL)
	{
	    uni_array(&T,kmax,FLOAT);
	    uni_array(&M,kmax,FLOAT);
	    uni_array(&S,kmax,FLOAT);

	    uni_array(&aT,kmax,FLOAT);
	    uni_array(&aM,kmax,FLOAT);
	    uni_array(&aS,kmax,FLOAT);
	}
	result = -HUGE_VAL;
	abserr = HUGE_VAL;
	neval = 0;
	if (epsabs <= 0. && epsrel < max(epmach,TINY))
	{
	    (void) printf("WARNING in SimpRule(), invalid epsilons"
	                  "epsabs = %24.20g, epsrel = %24.20g\n",epsabs,epsrel);
	    ier = INVALID_EPSILON;
	    if (pabserr)
	        *pabserr = abserr;
	    if (pneval)
	        *pneval = neval;
	    if (pier)
	        *pier = ier;
	    return result;
	}

	dx = b - a;
	twok = 1;
	fv1 = (*f)(a,prms);
	fv2 = (*f)(b,prms);
	neval = 2;
	T[0] = 0.5*(fv1 + fv2)*dx;
	aT[0] = 0.5*(fabs(fv1) + fabs(fv2))*dx;
	fv = (*f)(0.5*(b+a),prms);
	++neval;
	M[0] = fv*dx;
	aM[0] = fabs(fv)*dx;
	S[0] = aS[0] = 0.0;

	dx *= 0.5;
	twok *= 2;
	T[1] = 0.5*(T[0] + M[0]);
	aT[1] = 0.5*(aT[0] + aM[0]);
	S[1] = (T[0] + 2*M[0])/3.0;
	aS[1] = (aT[0] + 2*aM[0])/3.0;

	ier = INACCURATE_INTEGRAL;
	for (k = 2; k < kmax; ++k)
	{
	    M[k-1] = aM[k-1] = 0.0;
	    for (j = 0; j < twok; ++j)
	    {
		fv = (*f)(a + (j+0.5)*dx,prms);
		M[k-1] += fv*dx;
		aM[k-1] += fabs(fv)*dx;
	    }
	    neval += twok;
	    dx *= 0.5;
	    twok *= 2;
	    T[k] = 0.5*(T[k-1] + M[k-1]);
	    aT[k] = 0.5*(aT[k-1] + aM[k-1]);
	    S[k] = result = (T[k-1] + 2*M[k-1])/3.0;
	    aS[k] = (aT[k-1] + 2*aM[k-1])/3.0;
	    abserr = fabs((S[k] - S[k-1]));

	    if (aS[k] > uflow/epmach)
	        abserr = max(epmach*aS[k],abserr);
	    if (abserr <= max(epsabs,epsrel*fabs(result)))
	    {
		ier = ACCURATE_INTEGRAL;
	        if (pier)
	            *pier = ier;
	        if (pabserr)
	            *pabserr = fabs(S[k]-S[k-1]);
	        if (pneval)
	            *pneval = neval;
	        return result;
	    }
	}
	if (pier)
	    *pier = ier;
	if (pabserr)
	    *pabserr = abserr;
	if (pneval)
	    *pneval = neval;
	return result;
}		/*end SimpRule */
