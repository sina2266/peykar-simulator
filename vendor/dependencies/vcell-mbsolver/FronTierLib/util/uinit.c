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
*			uinit.c
*
*	Utility programs for program initialization.
*/

/* LINTLIBRARY */

#include <cdecs.h>
#include <vmalloc.h>

EXPORT	void	init_prompting_and_debugging(
	INIT_DATA	*init)
{
	IMPORT boolean	suppress_prompts;
	IMPORT boolean	fgetstring_debug;		/* from fgetstring.c */
	char		s[Gets_BUF_SIZE];

	screen("\nType 'p' to obtain prompting on input: ");
	(void) fgets(s,Gets_BUF_SIZE-2,stdin);
	screen("\n");

	if (s[0] == 'p')
	{
	    (void) fprintf(stderr,"\n\n");
	    interactive_prompting(init) = YES;
	    suppress_prompts = NO;
	}
	else
	{
	    interactive_prompting(init) = NO;
	    suppress_prompts = YES;
	}

	dbparams(init) = init_debug(PROMPT_FOR_DEBUG);

	if (debugging("vm_1") || debugging("vmalloc_1"))
	    vmalloc_debug(1);
	if (debugging("vm_2") || debugging("vmalloc_2"))
	    vmalloc_debug(2);
	if (debugging("vm_3") || debugging("vmalloc_3"))
	    vmalloc_debug(3);
	if (debugging("vm_4") || debugging("vmalloc_4"))
	    vmalloc_debug(4);
	if (debugging("vm_5") || debugging("vmalloc_5"))
	    vmalloc_debug(5);
	if (debugging("make_core"))
	    set_dump_core(YES);

	if (debugging("fgetstring") || debugging("init"))
		fgetstring_debug = YES;
	if (debugging("nobuf"))
	{
	    (void) fflush(stdout);
	    setbuf(stdout,NULL);
	}
}		/*end init_prompting_and_debugging*/

EXPORT	void	init_default_debugging(
	INIT_DATA	*init)
{
	IMPORT boolean	suppress_prompts;
	IMPORT boolean	fgetstring_debug;		/* from fgetstring.c */

	interactive_prompting(init) = NO;
	suppress_prompts = YES;

	dbparams(init) = default_debug(PROMPT_FOR_DEBUG);

	if (debugging("vm_1") || debugging("vmalloc_1"))
	    vmalloc_debug(1);
	if (debugging("vm_2") || debugging("vmalloc_2"))
	    vmalloc_debug(2);
	if (debugging("vm_3") || debugging("vmalloc_3"))
	    vmalloc_debug(3);
	if (debugging("vm_4") || debugging("vmalloc_4"))
	    vmalloc_debug(4);
	if (debugging("vm_5") || debugging("vmalloc_5"))
	    vmalloc_debug(5);
	if (debugging("make_core"))
	    set_dump_core(YES);

	if (debugging("fgetstring") || debugging("init"))
		fgetstring_debug = YES;
	if (debugging("nobuf"))
	{
	    (void) fflush(stdout);
	    setbuf(stdout,NULL);
	}
}	/* end init_default_debugging */
