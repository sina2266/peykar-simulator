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
*                               intfc_amr.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*
*       User definable hooks to the interface library.
*
*       IMPORTANT NOTE: All of the i_user_... functions below are NO-OP
*       functions, i.e. they perform no operations.  These hooks are
*       simply default place holders for the interface hook functions.
*       The reason these functions are NO-OPs is that any operation they
*       perform would be meaningful at the intfc library level and so should
*       properly be included in the actual operation function rather than
*       the user hook function.
*/




#include <intfc/iloc.h>

#if defined(USE_OVERTURE)

EXPORT void set_amr_intfc_tol(
	INTERFACE     *intfc,
	double         coeff)
{
        /*  
        InterfaceTolerances(intfc)._Parallel; 
        */  
        InterfaceTolerances(intfc)._Min_sin_sqr *= coeff; 
        InterfaceTolerances(intfc)._MinScaledSeparation *= coeff;
        InterfaceTolerances(intfc)._MinScaledLength *= coeff;
        InterfaceTolerances(intfc)._EndOfCurve = 1.0 - 
                      InterfaceTolerances(intfc)._MinScaledSeparation;
        InterfaceTolerances(intfc)._StartOfCurve = 
                      InterfaceTolerances(intfc)._MinScaledSeparation; 
        InterfaceTolerances(intfc)._TolFac = 32.0; 
        InterfaceTolerances(intfc)._RcbMinScaledSep = 
                      InterfaceTolerances(intfc)._MinScaledLength;
        InterfaceTolerances(intfc)._RobustFac *= coeff;
        InterfaceTolerances(intfc)._RcbMacTol = 1.0e-5; 
        InterfaceTolerances(intfc)._RcbcRobustFac = 
                      InterfaceTolerances(intfc)._MinScaledLength; 
        InterfaceTolerances(intfc)._ReflectTol *= coeff; 
        /* 
        InterfaceTolerances(intfc)._ShortCurveNumPoints; 
        */  
}

#endif /* if defined(USE_OVERTURE) */

