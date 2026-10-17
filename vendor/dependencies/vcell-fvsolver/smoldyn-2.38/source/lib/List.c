/* Steven Andrews, 10/6/2015
See documentation called list_doc.doc
Copyright 2015 by Steven Andrews.  This work is distributed under the terms
of the Gnu Lesser General Public License (LGPL). */

#include <stdlib.h>
#include "List.h"
#include "string2.h"


int List_ExpandLI(listptrli list,int spaces);


/************************ List_Expand **********************/


/* List_ExpandLI */
int List_ExpandLI(listptrli list,int spaces) {
	long int *newxs;
	int newmax,i,newn;

	newmax=list->max+spaces;
	if(newmax<=0) {
		newxs=NULL;
		newmax=0;
		newn=0; }
	else {
		newxs=(long int*) calloc(newmax,sizeof (long int));
		if(!newxs) return 1;
		for(i=0;i<list->n && i<newmax;i++) newxs[i]=list->xs[i];
		newn=i;
		for(;i<newmax;i++) newxs[i]=0; }
	free(list->xs);
	list->xs=newxs;
	list->max=newmax;
	list->n=newn;
	return 0; }


/************************* List_Alloc **********************/


/* List_AllocLI */
listptrli List_AllocLI(int max) {
	listptrli list;
	int er;

	list=(listptrli) malloc(sizeof(struct liststructli));
	if(!list) return NULL;
	list->max=0;
	list->n=0;
	list->xs=NULL;
	er=List_ExpandLI(list,max);
	if(er) {
		List_FreeLI(list);
		return NULL; }
	return list; }


/************************* List_Free ***********************/


/* List_FreeLI */
void List_FreeLI(listptrli list) {
	if(list) {
		free(list->xs);
		free(list); }
	return; }


/********************** List_ReadString ********************/


/* List_ReadStringLI */
listptrli List_ReadStringLI(char *string) {
	listptrli list;
	int n,nread;

	n=wordcount(string);
	list=List_AllocLI(n);
	if(!list) return NULL;
	nread=strreadnli(string,n,list->xs,NULL);
	if(nread!=n) {
		List_FreeLI(list);
		return NULL; }
	list->n=n;

	return list; }


/*********************** List_AppendTo *********************/


/* List_AppendToLI */
int List_AppendToLI(listptrli list,listptrli newstuff) {
	int spaces,i,newn,er;

	newn=newstuff->n;
	spaces=newn-(list->max-list->n);		// number of spaces to allocate
	if(spaces>0) {
		er=List_ExpandLI(list,spaces);
		if(er) return 1; }
	for(i=0;i<newn;i++)
		list->xs[list->n+i]=newstuff->xs[i];
	list->n+=newn;
	return 0; }


/********************** List_RemoveFrom ********************/


/* List_RemoveFromLI */
int List_RemoveFromLI(listptrli list,listptrli remove) {
	int i,i2,j,count;

	count=0;
	for(j=0;j<remove->n;j++)
		for(i=list->n-1;i>=0;i--)
			if(list->xs[i]==remove->xs[j]) {
				for(i2=i;i2<list->n-1;i2++)
					list->xs[i2]=list->xs[i2+1];
				list->n--;
				count++;
				break; }
	return count; }


/************************ List_Member **********************/


/* List_MemberLI */
int List_MemberLI(listptrli list,long int x) {
	int i;

	for(i=0;i<list->n;i++)
		if(list->xs[i]==x) return 1;
	return 0; }



