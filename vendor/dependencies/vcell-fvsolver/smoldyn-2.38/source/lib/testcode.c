/* Steve Andrews */

/**** Compiling and linking ****

Compile and link:

gcc -Wall -O0 -g testcode.c string2.c -o testcode
 
*/


#include <stdio.h>

#include "string2.h"

int main() {
	char pat[256],str[256],dest[256];
	char **resultlist;
	int added,nr,maxr,i;

	printf("Testing logic expansion\n");
	nr=0;
	maxr=0;
	resultlist=NULL;
	while(1) {
		printf("Enter pattern, string, and destination: ");
		scanf("%s %s %s",pat,str,dest);
		printf("simple match: %s\n",strwildcardmatch(pat,str)?"yes":"no");
		printf("subst. match: %s, %s\n",strwildcardmatchandsub(pat,str,dest)?"yes":"no",dest);
		printf("enhanced match: %s\n",strEnhWildcardMatch(pat,str)?"yes":"no");
		added=strexpandlogic(pat,0,-1,&resultlist,0,&nr,&maxr);
		if(added<0) printf("error code: %i\n",added);
		else {
			printf("%i expansion results:\n",added);
			for(i=0;i<nr;i++)
				printf("%s\n",resultlist[i]); }}

	return 0; }


	
