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


#if !defined(_INT_AMR_H)
#define _INT_AMR_H

#include <intfc/int.h>

#if defined(USE_OVERTURE) 

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif


struct _PATCH_BDRY_FLAG{
        int             patch_bdry;
        int             bdry[3];
        int             bdry_side[3][2];
};
typedef struct _PATCH_BDRY_FLAG Patch_bdry_flag;

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif


#endif /* defined(USE_OVERTURE) */
#endif /* #if !defined(_INT_AMR_H) */  
