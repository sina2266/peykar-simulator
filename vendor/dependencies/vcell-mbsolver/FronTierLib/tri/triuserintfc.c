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
*				triuserintfc.c:
*
*       Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*	A triangulation of the computational region is constructed
*	from a rectangular grid and an interface.  The corners of
*	the triangles and quadrangles in the triangulation are either
*	intersections between grid lines of the rectangular grid or
*	intersections of grid lines with the interface.
*
*	TODO:
*		1. Rig up a mechanism for triangulating only selected
*			components.
*		2. Speed up area computations.
*/



#include <tri/trilocaldecs.h>

	/* LOCAL Function Declarations */


EXPORT	void t_set_interface_hooks(
	int		dim,
	INIT_DATA       *init)
{
	f_set_interface_hooks(dim,init);
}		/*end t_set_interface_hooks*/

