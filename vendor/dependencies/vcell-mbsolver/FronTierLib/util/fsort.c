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
*				fsort.c
*
*	Copyright 1999 by The University at Stony Brook, All rights reserved.
*
*/

#include <cdecs.h>
#include <stdlib.h>
#ifndef __clang__
# include <malloc.h>
#endif

struct _NAME_LIST {
	const char *name;
	int        time_step;
	int        node_number;
};
typedef struct _NAME_LIST NAME_LIST;


	/* LOCAL Function Prototypes */
LOCAL	int in_order(NAME_LIST*,NAME_LIST*);
LOCAL	void	sort_list(NAME_LIST**,int,int);
LOCAL	void	swap(NAME_LIST**,int,int);

EXPORT	const char ***sort_in_files(
	int nnames,
	const char **fnames)
{
	NAME_LIST **files, *file;
	int i, k, n, num_time_steps, nfiles;
	int j;
	char name[512];
	char *s, *s1, *s2;
	char wk[180];
	const char ***list;

        for (i = 0; i < nnames && fnames[i][0] != '-'; ++i );
	nfiles = i;
	if (nfiles == 0)
	    return NULL;

	files = (NAME_LIST **)malloc(nfiles*sizeof(NAME_LIST *));
        for (i = 0; i < nfiles; ++i )
	{
	    files[i] = file = (NAME_LIST *)malloc(sizeof(NAME_LIST));
	    (void) strcpy(name,fnames[i]);
	    file->name = fnames[i];
	    file->time_step = 0;
	    file->node_number = 0;
	    for (j = ((int)strlen(name))-1; j >= 0; --j)
	    {
	    	if (name[j] == '.')
	    	{
	    	    s = name + j + 1;
	    	    break;
	    	}
	    }
	    if ( j < 0 ) continue;
	    if ((strncmp(s,"gz",2) == 0) || (strncmp(s,"Z",1)==0))
	    {
	    	name[j] = '\0';
	    	for (--j; j >= 0; --j)
	    	{
	    	    if (name[j] == '.')
	    	    {
	    	    	s = name + j + 1;
	    	    	break;
	    	    }
	    	}
	    	if ( j < 0 ) continue;
	    }
	    if (strncmp(s,"ts",2) == 0)
	    {
	    	(void) strcpy(wk,s+2);
	    	s1 = wk;
	    	while (isdigit(*s1))
		    ++s1;
	    	if (strncmp(s1,"-nd",3) == 0)
	    	{
	    	    s2 = s1+3;
	    	    while (isdigit(*s2))
		        ++s2;
	    	    if (*s2 != '\0') continue;
	    	    s2 = s1+3;
	    	    if (*s2 != '\0')
	    	    	(void) sscanf(s2,"%d",&file->node_number);
	    	    *s1 = '\0';
	    	}
	        if (wk[0] != '\0' && *s1 == '\0')
	      	    (void) sscanf(wk,"%d",&file->time_step);
	    }
	    else
	    {
	        s1 = s;
	        while (isdigit(*s1))
		    ++s1;
	        if (*s != '\0' && *s1 == '\0')
	       	    (void) sscanf(s,"%d",&file->node_number);
	    }
	}
	sort_list(files,0,nfiles-1);
	j = files[0]->time_step;
	num_time_steps = 1;
        for (i = 1; i < nfiles; ++i )
	{
	    if (files[i]->time_step > j)
	    {
	    	++num_time_steps;
	    	j = files[i]->time_step;
	    }
	}
	list = (const char ***)malloc((num_time_steps+1)*sizeof(char **));
	list[num_time_steps] = NULL;
        for (i = 0, j = 0; i < nfiles; ++j)
	{
	    for (k = i; k < nfiles; ++k)
	    	if (files[i]->time_step != files[k]->time_step)
	    	    break;
	    list[j] = (const char **)malloc((k-i+1)*sizeof(char *));
	    for (n = 0; n < k-i; n++)
	    {
	    	list[j][n] = strdup(files[i+n]->name);
	    }
	    list[j][k-i] = NULL;
	    i = k;
	}
	/* clean up */
        for (i = 0; i < nfiles; ++i) free(files[i]);
	free(files);
	return list;
}

EXPORT	char ***free_infile_list(
	char ***list)
{
	int i, j;

	for (i = 0; list[i] != NULL; ++i)
	{
	    for (j = 0; list[i][j] != NULL; ++j)
	    	free(list[i][j]);
	    free(list[i]);
	}
	free(list);
	return NULL;
}

LOCAL	void	sort_list(
	NAME_LIST **files,
	int left,
	int right)
{
	int i, last;

	if (left >= right) return;
	swap(files,left,(left+right)/2);
	last = left;
	for (i = left+1; i <= right; ++i)
		if (in_order(files[i],files[left]) == -1)
			swap(files,++last,i);
	swap(files,left,last);
	sort_list(files,left,last-1);
	sort_list(files,last+1,right);
}
	

LOCAL	int in_order(
	NAME_LIST *f1,
	NAME_LIST *f2)
{
	int cmp;

	if (f1->time_step < f2->time_step) return -1;
	if (f1->time_step > f2->time_step) return 1;
	if (f1->node_number < f2->node_number) return -1;
	if (f1->node_number > f2->node_number) return 1;
	cmp = strcmp(f1->name,f2->name);
	if (cmp < 0) return -1;
	if (cmp > 0) return 1;
	return 0;
}

LOCAL	void	swap(NAME_LIST **files, int i, int j)
{
	NAME_LIST *tmp;

	tmp = files[i];
	files[i] = files[j];
	files[j] = tmp;
}



#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#endif

struct _FLIST {
	FILE	*file;
	char	fname[512];
	struct	_FLIST	*next, *prev;
};
typedef	struct	_FLIST	FLIST;
FLIST	*uc_head;

EXPORT	FILE	*UncompressAndOpenFile(
	const char *fname,
	const char *type)
{
	FLIST	   *uc;
	const char *c;
	char       s[512];
	int	   i, slen = 0;
	int	   len = (int)strlen(fname);

	if (len > 3 && strncmp(fname+len-3,".gz",3)==0)
	{
	    slen = 3;
	}
	else if (len > 2 && strncmp(fname+len-2,".Z",3)==0)
	{
	    slen = 2;
	}
	else
	    return fopen(fname,type);
	uc = (FLIST*) malloc(sizeof(FLIST));
	for (i = len - slen - 1, c = fname + i; i > 0 && *c != '/'; --i, --c);
	if (*c == '/')
	    ++c;
	(void) sprintf(uc->fname,"/tmp/%s",c);
	len = (int)strlen(uc->fname);
	uc->fname[len - slen] = '\0';
	(void) sprintf(s,"zcat < %s > %s",fname,uc->fname);
	(void) system(s);
	uc->file = fopen(uc->fname,type);
	uc->next = uc_head;
	uc->prev = NULL;
	if (uc_head == NULL)
	    uc_head = uc;
	return uc->file;
}

EXPORT	void	CloseAndCleanUpTmpFiles(
	FILE	*file)
{
	FLIST	*uc;

	for (uc = uc_head; uc != NULL; uc = uc->next)
	    if (file == uc->file)
	    	break;

	if (uc != NULL)
	{
	    (void) unlink(uc->fname);
	    if (uc->next)
	    	uc->next->prev = uc->prev;
	    if (uc->prev)
	    	uc->prev->next = uc->next;
	    if (uc_head == uc)
	    	uc_head = uc->next;
	    free(uc);
	}
	(void) Fclose(file);
}

EXPORT	void	CloseAndCleanUpAllFiles(void)
{
	FLIST	*uc, *uc_next = NULL;
	for (uc = uc_head; uc != NULL; uc = uc_next)
	{
	    (void) unlink(uc->fname);
	    (void) fclose(uc->file);
	    uc_next = uc->next;
	    free(uc);
	}
	uc_head = NULL;
}
