/* Steven Andrews, 10/6/2015
See documentation called list_doc.doc
Copyright 2015 by Steven Andrews.  This work is distributed under the terms
of the Gnu Lesser General Public License (LGPL). */

#ifndef __list_h
#define __list_h


typedef struct liststructli{
	int max;
	int n;
	long int *xs;
	} *listptrli;

listptrli List_AllocLI(int max);
void List_FreeLI(listptrli list);
listptrli List_ReadStringLI(char *string);
int List_AppendToLI(listptrli list,listptrli newstuff);
int List_RemoveFromLI(listptrli list,listptrli remove);
int List_MemberLI(listptrli list,long int x);


#endif

