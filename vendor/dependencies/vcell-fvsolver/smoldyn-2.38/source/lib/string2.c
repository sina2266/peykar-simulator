/* Steven Andrews, 1/99
See documentation called string2_doc.doc
Copyright 2003-2007 by Steven Andrews.	This work is distributed under the terms
of the Gnu Lesser General Public License (LGPL). */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "string2.h"


/******************************************************************/
/************ Declarations for internal functions ****************/
/******************************************************************/

#define CHECKS(A,...)		if(!(A)) {sprintf(StrErrorString,__VA_ARGS__); goto failure;} else (void)0

char StrErrorString[STRCHAR];
int MathParseError=0;

int permutelex(int *seq,int n);
int allocresults(char ***resultsptr,int *maxrptr,int nchar);


/******************************************************************/
/******************* String classification ************************/
/******************************************************************/


/* strisnumber */
int strisnumber(const char *str) {
  char *ptr;
  int len;
  double dbl;
  
  if(!(len=strlen(str))) return 0;
  dbl=strtod(str,&ptr);
  return(ptr==str+len); }


/* okname */
int okname(const char *name) {
	int ok;

	ok=1;
	ok=ok && isalpha(*name);
	for(name++;*name && ok;name++)
		ok=ok && (isalnum(*name) || *name=='_');
	return ok; }


/* strbegin */
int strbegin(const char *strshort,const char *strlong,int casesensitive) {
	int i;
	
	i=0;
	if(casesensitive)
		while(strshort[i]!='\0' && strshort[i]==strlong[i]) i++;
	else
		while(strshort[i]!='\0' && tolower(strshort[i])==tolower(strlong[i])) i++;
	if(strshort[i]=='\0' && i>0) return 1;
	return 0; }


/* symbolcount */
int symbolcount(const char *s,char c) {
	int n;
	
	n=0;
	for(;*s;s++)
		if(*s==c) n++;
	return n; }


/******************************************************************/
/*********************** Character locating ***********************/
/******************************************************************/


/* strchrindex */
int strchrindex(const char *cs,char c,int i) {
  while(cs[i]!='\0') {
    if(cs[i]==c) return i;
    i++; }
  return -1; }


/* strrpbrk */
char *strrpbrk(char *cs,const char *ct) {
	char *s2;
	int i,n;

	if(!cs || !ct) return NULL;
	n=strlen(ct);
	for(s2=cs+strlen(cs)-1;s2>=cs;s2--)
		for(i=0;i<n;i++)
			if(*s2==ct[i]) return s2;
	return NULL; }


/* StrChrQuote */
char *StrChrQuote(char *cs,char c) {
	int qt;
	
	qt=0;
	for(;*cs;cs++)	{
		if(*cs=='"')	qt=!qt;
		else if(*cs==c && !qt) return cs;	}
	return NULL; }


/* StrChrPQuote */
int StrChrPQuote(const char *cs,char c) {
	unsigned int i,pn,qt;
	
	pn=qt=0;
	for(i=0;i<strlen(cs);i++)
		if(cs[i]=='"') qt=!qt;
		else if(cs[i]==c && !qt && !pn) return i;
		else if(cs[i]=='(' && !qt) pn++;
		else if(cs[i]==')' && !qt && pn) pn--;
		else if(cs[i]==')' && !qt) return -2;
	if(pn) return -2;
	if(qt) return -3;
	return -1; }


/* StrrChrPQuote */
int StrrChrPQuote(const char *cs,char c) {
	int i,pn,qt;
	
	pn=qt=0;
	for(i=strlen(cs)-1;i>=0;i--)
		if(cs[i]=='"') qt=!qt;
		else if(cs[i]==c && !qt && !pn) return i;
		else if(cs[i]==')' && !qt) pn++;
		else if(cs[i]=='(' && !qt && pn) pn--;
		else if(cs[i]=='(' && !qt) return -2;
	if(pn) return -2;
	if(qt) return -3;
	return -1; }


/* strChrBrackets */
int strChrBrackets(const char *string,int n,char c,const char *delimit) {
	int i,pn,bk,bc,qt,sq,ckpn,ckbk,ckbc,ckqt,cksq;
	char ch;
	
	pn=bk=bc=qt=sq=0;
	ckpn=strchr(delimit,'(')?1:0;
	ckbk=strchr(delimit,'[')?1:0;
	ckbc=strchr(delimit,'{')?1:0;
	ckqt=strchr(delimit,'"')?1:0;
	cksq=strchr(delimit,'\'')?1:0;
	if(n<0)
		n=strlen(string);
  
	for(i=0;i<n;i++) {
		ch=string[i];
		if(ch==c) {
			if(pn!=0 || bk!=0 || bc!=0 || qt!=0 || sq!=0);
			else return i; }
		else if(ckpn && ch=='(') pn++;
		else if(ckbk && ch=='[') bk++;
		else if(ckbc && ch=='{') bc++;
		else if(ckqt && ch=='"') qt=!qt;
		else if(cksq && ch=='\'') sq=!sq;
		else if(ckpn && ch==')') {if(--pn<0) return -2;}
		else if(ckbk && ch==']') {if(--bk<0) return -3;}
		else if(ckbc && ch=='}') {if(--bc<0) return -4;} }
	
	return -1; }


/* strPbrkBrackets */
int strPbrkBrackets(const char *string,int n,const char *c,const char *delimit) {
	int i,pn,bk,bc,qt,sq,ckpn,ckbk,ckbc,ckqt,cksq;
	char ch;
	
	pn=bk=bc=qt=sq=0;
	ckpn=strchr(delimit,'(')?1:0;
	ckbk=strchr(delimit,'[')?1:0;
	ckbc=strchr(delimit,'{')?1:0;
	ckqt=strchr(delimit,'"')?1:0;
	cksq=strchr(delimit,'\'')?1:0;
	if(n<0)
		n=strlen(string);
  
	for(i=0;i<n;i++) {
		ch=string[i];
		if(strchr(c,ch)) {
			if(pn!=0 || bk!=0 || bc!=0 || qt!=0 || sq!=0);
			else return i; }
		else if(ckpn && ch=='(') pn++;
		else if(ckbk && ch=='[') bk++;
		else if(ckbc && ch=='{') bc++;
		else if(ckqt && ch=='"') qt=!qt;
		else if(cksq && ch=='\'') sq=!sq;
		else if(ckpn && ch==')') {if(--pn<0) return -2;}
		else if(ckbk && ch==']') {if(--bk<0) return -3;}
		else if(ckbc && ch=='}') {if(--bc<0) return -4;} }
	
	return -1; }


/* strparenmatch */
int strparenmatch(const char *string,int index) {
	char ch1,ch2;
	int dir,i,count;
	
	ch1=string[index];
	if(ch1=='(') {dir=1;ch2=')';}
	else if(ch1=='[') {dir=1;ch2=']';}
	else if(ch1=='{') {dir=1;ch2='}';}
	else if(ch1==')') {dir=-1;ch2='(';}
	else if(ch1==']') {dir=-1;ch2='[';}
	else if(ch1=='}') {dir=-1;ch2='{';}
	else return -1;
	
	i=index+dir;
	count=0;
	for(;;) {
		while(i>=0 && string[i]!='\0' && string[i]!=ch1 && string[i]!=ch2) i+=dir;
		if(i<0 || string[i]=='\0') return -2;
		if(string[i]==ch1) count++;
		else if(count==0) return i;
		else count--;
		i+=dir; }
	return 0; }


/* strsymbolmatch */
int strsymbolmatch(char *str1,char *str2,const char *symbols) {
	char *s1,*s2;

	s1=str1;
	s2=str2;
	while(1) {
		s1=strpbrk(s1,symbols);
		s2=strpbrk(s2,symbols);
		if(!s1 && !s2) return 1;
		if(!s1 || !s2) return 0;
		if(*s1!=*s2) return 0;
		s1++;
		s2++; }
	return 0; }



/******************************************************************/
/************************* Word operations ************************/
/******************************************************************/


/* wordcount */
int wordcount(const char *s) {
	int n,sp;
	const char *s2;
	
	n=0;
	sp=1;
	for(s2=s;*s2;s2++) {
		if(sp && !isspace(*s2)) n++;
		sp=isspace(*s2); }
	return n; }


/* strnword */
char *strnword(char *s,int n) {
	char *s2;
	
	if(!s) return NULL;
	s2=s;
	if(!isspace(*s)) n--;
	for(;n>0 && *s2;n--) {
		while(!isspace(*s2) && *s2) s2++;
		while(isspace(*s2)) s2++; }
	return *s2?s2:NULL; }


/* strnword1 */
char *strnword1(char *s,int n) {
	char *s2;
	
	if(!s) return NULL;
	s2=s;
	for(n--;n>0 && *s2;n--) {
		while(*s2 && *s2!=' ' && *s2!='\t') s2++;
		if(*s2) s2++; }
	if(*s2==' ' || *s2=='\t' || !*s2) return NULL;
	else return s2;}


/******************************************************************/
/************************** String arrays *************************/
/******************************************************************/


/* stringfind */
int stringfind(char **slist,int n,const char *s) {
	int i;
	
	for(i=0;i<n && strcmp(slist[i],s);i++);
	return i<n?i:-1; }


/* stringnfind */
int stringnfind(char **slist,int n,const char *s,int nchar) {
	int i;
	
	for(i=0;i<n && strncmp(slist[i],s,nchar);i++);
	return i<n?i:-1; }


/******************************************************************/
/**************** Reading sequential items from strings ***********/
/******************************************************************/


/* strreadni */
int strreadni(char *s,int n,int *a,char **endp) {
	int i,ok;
	char *s2;
	
	s2=s;
	ok=1;
	for(i=0;i<n && ok;i++) {
		a[i]=(int) strtol(s,&s2,10);
		if(s2==s) ok=0;
		s=s2; }
	if(endp) *endp=s2;
	return ok?i:i-1; }


/* strreadnli */
int strreadnli(char *s,int n,long int *a,char **endp) {
	int i,ok;
	char *s2;
	
	s2=s;
	ok=1;
	for(i=0;i<n && ok;i++) {
		a[i]=(long int) strtol(s,&s2,10);
		if(s2==s) ok=0;
		s=s2; }
	if(endp) *endp=s2;
	return ok?i:i-1; }


/* strreadnf */
int strreadnf(char *s,int n,float *a,char **endp) {
	int i,ok;
	char *s2;
	
	s2=s;
	ok=1;
	for(i=0;i<n && ok;i++) {
		a[i]=(float) strtod(s,&s2);
		if(s2==s) ok=0;
		s=s2; }
	if(endp) *endp=s2;
	return ok?i:i-1; }


/* strreadnd */
int strreadnd(char *s,int n,double *a,char **endp) {
	int i,ok;
	char *s2;
	
	s2=s;
	ok=1;
	for(i=0;i<n && ok;i++) {
		a[i]=strtod(s,&s2);
		if(s2==s) ok=0;
		s=s2; }
	if(endp) *endp=s2;
	return ok?i:i-1; }


/* strreadns */
int strreadns(char *s,int n,char **a,char **endp) {
	int i,j;
	char *s2;
	
	s2=s;
	j=1;
	for(i=0;i<n && *s2;i++) {
		while(isspace(*s2)) s2++;
		for(j=0;!isspace(*s2) && *s2;j++) a[i][j]=*(s2++);
		if(j) a[i][j]='\0'; }
	if(endp) *endp=s2;
	return j?i:i-1; }


/******************************************************************/
/************ String copying with memory allocation ***************/
/******************************************************************/


/* EmptyString */
char *EmptyString() {
	char *s;
	int i;
	
	s=(char*) calloc(STRCHAR,sizeof(char));
	if(s)	for(i=0;i<STRCHAR;i++)	s[i]='\0';
	return s; }


/* StringCopy */
char *StringCopy(const char *s) {
	char *s2;
	int i;

	s2=(char *) calloc(strlen(s)+1,sizeof(char));
	if(!s2) return NULL;
	for(i=0;s[i];i++)	s2[i]=s[i];
	s2[i]='\0';
	return s2; }


/* PascalString */
unsigned char *PascalString(const char *s) {
	unsigned char *s2;
	int i;

	s2=(unsigned char *) calloc(strlen(s)+1,sizeof(unsigned char));
	if(!s2) return 0;
	for(i=0;s[i];i++)	s2[i+1]=s[i];
	s2[0]=(unsigned char) i;
	return s2; }


/******************************************************************/
/************ String modifying without memory allocation **********/
/******************************************************************/


/* strPreCat */
char *strPreCat(char *str,const char *cat,int start,int stop) {
	int i,n,len;
	
	n=stop-start;
	len=strlen(str);
	for(i=len+n;i>=n;i--) str[i]=str[i-n];
	for(;i>=0;i--) str[i]=cat[start+i];
	return str; }


/* strPostCat */
char *strPostCat(char *str,const char *cat,int start,int stop) {
	int i,n,len;
	
	n=stop-start;
	len=strlen(str);
	for(i=0;i<n;i++) str[len+i]=cat[start+i];
	str[len+i]='\0';
	return str; }


/* strMidCat */
char *strMidCat(char *str,int s1,int s2,const char *cat,int start,int stop) {
	int i,n,len,shift;

	if(stop<0) stop=strlen(cat);
	n=stop-start;
	shift=n-(s2-s1);
	len=strlen(str);
	if(shift>0)
		for(i=len+shift;i>=s2+shift;i--) str[i]=str[i-shift];
	else if(shift<0)
		for(i=s2+shift;i<=len+shift;i++) str[i]=str[i-shift];
	for(i=0;i<n;i++) str[i+s1]=cat[start+i];
	return str; }


/* strchrreplace */
int strchrreplace(char *str,char charfrom,char charto) {
	int n;
	char *s1;
	
	n=0;
	while((s1=strchr(str,charfrom))) {
		*s1=charto;
		n++; }
	return n; }


/* strstrreplace */
int strstrreplace(char *str,const char *strfrom,const char *strto,int max) {
	int n,diff,lento,lenfrom,over;
	char *s1,*s2;
	int i,j;
	
	if(strto) lento=strlen(strto);
	else lento=0;
	lenfrom=strlen(strfrom);
	diff=lento-lenfrom;
	over=0;
	n=0;
	s2=str;
	while((s1=strstr(s2,strfrom))) {
		i=s1-str;
		if(diff<0) {
			for(j=i+lenfrom;str[j-1] && j<max;j++)
				str[j+diff]=str[j]; }
		else if(diff>0) {
			for(j=strlen(str);j>=i+lenfrom;j--)
				if(j+diff<max) str[j+diff]=str[j];
			else over=1; }
		for(j=0;j<lento;j++)
			if(i+j<max) str[i+j]=strto[j];
			else over=1;
		if(i+lento<max) s2=s1+lento;
		else s2=str+strlen(str);
		n++; }
	if(over) return -n;
	return n; }


/* strcutwhite */
void strcutwhite(char *str,int end) {
	int i,j;
	
	if(end&2) {	// end of string
		for(i=strlen(str)-1;i>=0 && isspace(str[i]);i--);
		str[i+1]='\0'; }
	if(end&1) {	// start of string
		for(i=0;str[i]!='\0' && isspace(str[i]);i++);
		j=0;
		for(;str[i]!='\0';i++) str[j++]=str[i];
		str[j]='\0'; }
	return; }


/* strbslash2escseq */
int strbslash2escseq(char *str) {
	char *s1,*s2;
	
	s1=s2=str;
	while(*s2) {
		if(*s2=='\\') {
			s2++;
			if(*s2=='a') *s1='\a';
			else if(*s2=='b') *s1='\b';
			else if(*s2=='t') *s1='\t';
			else if(*s2=='n') *s1='\n';
			else if(*s2=='v') *s1='\v';
			else if(*s2=='f') *s1='\f';
			else if(*s2=='r') *s1='\r';
			else if(*s2=='\\') *s1='\\';
			else if(*s2=='"') *s1='\"';
			else *s1='\\'; }
		else
			*s1=*s2;
		s1++;
		s2++; }
	*s1=*s2;
	return s2-s1; }


/******************************************************************/
/******************* Wildcards and enhanced wildcards *************/
/******************************************************************/


/* strcharlistmatch */
int strcharlistmatch(const char *pat,const char ch,int n) {
	int p;

	if(n<0) n=strlen(pat);
	for(p=0;p<n;p++) {
		if(pat[p]=='-') {															// initial -
			if(p+1==n) return 1;												// only -
			if(ch<=pat[p+1]) return 1;									// match initial -
			p++; }
		else if(p+1<n && pat[p+1]=='-') {							// internal or terminal -
			if(p+2==n) {																// terminal -
				if(ch>=pat[p]) return 1;									// terminal - match
				return 0; }																// terminal - no match
			if(ch>=pat[p] && ch<=pat[p+2]) return 1;		// internal - match
			p+=2; }
		else if(pat[p]==ch) return 1; }								// character match
	return 0; }


/* strwildcardmatch */
int strwildcardmatch(const char *pat,const char *str) {
	int s,p,s1,p1,match,blen,brend;

	p1=s1=-1;
	s=p=0;
	while(str[s]) {
		if(pat[p]=='*') {															// star found in pat
			if(!pat[p+1]) return 1;											// terminal star, so this is a match
			p++;
			p1=p;																				// p and p1 are at '*'+1, where post-star text will resume
			s1=s; }																			// s1 is at s, where any star text will start
		else if(pat[p]=='?') {s++;p++;}								// '?' wildcard character
		else if(pat[p]=='[') {												// character list or range
			brend=strchrindex(pat,']',p);
      if(brend==-1) return -1;
      blen=brend-p-1;
			match=strcharlistmatch(pat+p+1,str[s],blen);
			if(match) {
				s++;
				p+=blen+2; }
			else if(p1>=0) goto starlabel;							// act as if this option weren't ever selected
			else return 0; }
		else if(pat[p]==str[s]) {s++;p++;}						// same character
		else if(p1>=0) {															// in star text
		starlabel:
			if(p==p1) s++;
			else {
				p=p1;
				s1++;
				s=s1; }}
		else return 0; }															// no match
	while(pat[p]=='*') p++;
	if(!pat[p]) return 1;														// match
	return 0; }																			// more pattern, so no match


/* strwildcardmatchandsub */
int strwildcardmatchandsub(const char *pat,const char *str,char *dest) {
	int s,p,s1,p1,starpt,isub,nsub,isubstar,match,blen,d,starlen,brend;
	int maxsub=16;
	char subst[16][STRCHAR],patst[16][STRCHAR];

	p1=s1=-1;																				// p1 and s1 are pat and str indicies following star text
	p=s=0;																					// p and s are current pat and str indicies
	starpt=0;																				// starpt is star start location in str
	isub=0;																					// isub is current substitution string index
	isubstar=0;																			// isubstar is substitution string index for most recent open star
	starlen=0;																			// starlen is length of star string
	while(str[s]) {
		if(pat[p]=='*') {
			if(p1>=0) {																	// resolve any open star
				if(isub==maxsub) return -6;
				strncpy(subst[isubstar],str+starpt,starlen);
				subst[isubstar][starlen]='\0';
				strcpy(patst[isubstar],"**");
				p1=-1; }
			starpt=s;																		// point in str where star was reached
			if(!pat[p+1]) {															// terminal star
				s=strlen(str);
				if(isub==maxsub) return -6;
				strncpy(subst[isub],str+starpt,s-starpt);
				subst[isub][s-starpt]='\0';
				strcpy(patst[isub],"**");
				isub++;
				p++; }
			else {																			// new star, which is not terminal
				isubstar=isub++;
				starlen=0;
				p++;
				p1=p;
				s1=s; }}
		else if(pat[p]=='?') {												// '?' wildcard character
			if(isub==maxsub) return -6;
			subst[isub][0]=str[s];
			subst[isub][1]='\0';
			strcpy(patst[isub],"??");
			isub++;
			s++;
			p++; }
		else if(pat[p]=='[') {												// character list or range
      brend=strchrindex(pat,']',p);
      if(brend==-1) return -7;
      blen=brend-p-1;                             // length of text within brackets not including brackets
			match=strcharlistmatch(pat+p+1,str[s],blen);
			if(match) {
				if(isub==maxsub) return -6;
				subst[isub][0]=str[s];
				subst[isub][1]='\0';
				patst[isub][0]='[';
				strncpy(patst[isub]+1,pat+p,blen+2);
				patst[isub][blen+3]='\0';
				isub++;
				s++;
				p+=blen+2; }
			else if(p1>=0) goto starlabel;							// act as if this option weren't ever selected
			else return 0; }														// bracket pattern doesn't match and no open star, so no match
		else if(pat[p]==str[s]) {											// same character
			s++;
			p++; }
		else if(p1>=0) {															// in star text
		starlabel:
			if(p==p1) {																	// str and pat still don't match so keep going
				s++;
				starlen++; }
			else {																			// str and pat did match but now don't, so try one larger star text
				isub=isubstar+1;
				p=p1;
				s1++;
				s=s1;
				starlen=s1-starpt; }}
		else return 0; }															// no match

	if(p1>=0) {																			// resolve any open star
		if(isub==maxsub) return -6;
		strncpy(subst[isubstar],str+starpt,starlen);
		subst[isubstar][starlen]='\0';
		strcpy(patst[isubstar],"**");
		p1=-1; }
	while(pat[p]=='*') {														// any additional stars in pat represent no characters
		if(isub==maxsub) return -6;
		subst[isub][0]='\0';
		strcpy(patst[isub],"**");
		isub++;
		p++; }
	if(pat[p]) return 0;														// more text remains in pat, so no match
	nsub=isub;

	for(isub=0;isub<nsub;isub++) {									// ?? debug code
		printf("%i: pat='%s' subst='%s'\n",isub,patst[isub],subst[isub]); }

	if(dest) {																			// perform substitutions
		for(d=0;dest[d]!='\0';d++) {
			if(dest[d]=='*') {
				for(isub=0;isub<nsub && (patst[isub][0]=='x' || strcmp(patst[isub],"**"));isub++);
				if(isub==nsub) return -9;
				patst[isub][0]='x';
				strMidCat(dest,d,d+1,subst[isub],0,-1); }
			else if(dest[d]=='?') {
				for(isub=0;isub<nsub && (patst[isub][0]=='x' || strcmp(patst[isub],"??"));isub++);
				if(isub==nsub) return -9;
				patst[isub][0]='x';
				strMidCat(dest,d,d+1,subst[isub],0,-1); }
			else if(dest[d]=='[') {
        brend=strchrindex(dest,']',d);
        if(brend==-1) return -8;
        blen=brend-d-1;
				for(isub=0;isub<nsub && (patst[isub][0]=='x' || strncmp(patst[isub]+1,dest+d,blen+2));isub++);
				if(isub==nsub) return -9;
				patst[isub][0]='x';
				strMidCat(dest,d,d+blen+2,subst[isub],0,-1); }}}

	return 1; }


/* permutelex - internal function, duplicate of a Zn.c function */
int permutelex(int *seq,int n) {
	int i,j;
	int temp;
	
	i=n-1;
	while(i>0 && seq[i-1]>=seq[i]) i--;
	if(i==0) {						// input was final sequence
		i=0;
		j=n-1;
		while(i<j) {
			temp=seq[i];			// swap values at positions i and j
			seq[i]=seq[j];
			seq[j]=temp;
			i++;
			j--; }
		return 2; }
	
	j=n;
	while(seq[j-1]<=seq[i-1]) j--;
	
	temp=seq[i-1];			// swap values at positions (i-1) and (j-1)
	seq[i-1]=seq[j-1];
	seq[j-1]=temp;
	
	i++;
	j=n;
	while(i<j) {
		temp=seq[i-1];			// swap values at positions (i-1) and (j-1)
		seq[i-1]=seq[j-1];
		seq[j-1]=temp;
		i++;
		j--; }
	
	i=n-1;
	while(i>0 && seq[i-1]>=seq[i]) i--;
	if(i==0) return 1;	// at final sequence
	
	return 0; }


/* allocresults - internal function used by strexpandlogic */
int allocresults(char ***resultsptr,int *maxrptr,int nchar) {
	char **results,**newresults;
	int maxr,newmaxr,i;

	results=*resultsptr;
	maxr=*maxrptr;
	if(!results) maxr=0;

	if(nchar<0) {													// free memory
		if(results) {
			for(i=0;i<maxr;i++) free(results[i]);
			free(results);
			results=NULL; }
		maxr=0; }

	else {																// expand current list or make new one
		newmaxr=maxr*2+2;
		newresults=(char **) calloc(newmaxr,sizeof(char*));
		if(!newresults) return 1;
		for(i=0;i<newmaxr;i++) newresults[i]=NULL;
		for(i=0;i<newmaxr;i++) {
			newresults[i]=(char*) calloc(nchar,sizeof(char));
			if(!newresults[i]) return 1;
			newresults[i][0]='\0'; }
		for(i=0;i<maxr;i++) {
			strncpy(newresults[i],results[i],nchar-1);
			newresults[i][nchar-1]='\0'; }
		allocresults(resultsptr,maxrptr,-1);
		results=newresults;
		maxr=newmaxr; }

	*resultsptr=results;
	*maxrptr=maxr;
	return 0; }


/* strexpandlogic */
int strexpandlogic(const char *pat,int start,int stop,char ***resultsptr,int top,int *nrptr,int *maxrptr) {
	char **results,*tempresult;
	int done,pdone,iamp,er,i,i1,i2,totaladded,added,temptop,ptop,topdiff;
	int ampindx1[8],ampindx2[8],indx[8],maxamp=8,nampterm;	// only supports up to 8 ampersand terms

	if(!pat) {
		allocresults(resultsptr,maxrptr,-1);
		return 0; }

	results=*resultsptr;
	totaladded=0;
	done=0;
	if(stop<0) stop=strlen(pat);
	if(*nrptr>top) *nrptr=top;

	while(pat[start]=='{' && strparenmatch(pat,start)==stop-1) {	// ignore any terminal braces
		start++;
		stop--; }

	i2=start+strChrBrackets(pat+start,stop-start,'|',"{");				// OR operator
	if(i2>=start) {																								// OR operator found at i2
		i1=start;
		while(!done) {
			if(i2==i1) return -2;																			// missing OR operand
			added=strexpandlogic(pat,i1,i2,resultsptr,top,nrptr,maxrptr);
			if(added<0) return added;
			results=*resultsptr;
			top+=added;
			totaladded+=added;
			i1=i2+1;
			if(i2==stop) done=1;
			else {
				i2=i1+strChrBrackets(pat+i1,stop-i1,'|',"{");
				if(i2<i1) i2=stop; }}}

	if(!done) i2=start+strChrBrackets(pat+start,stop-start,'&',"{");	// AND operator
	if(!done && i2>=start) {
		temptop=top;							// temptop will be the top of the expanded operands
		i1=start;
		nampterm=0;								// number of ampersand-separated terms
		while(i2>=0) {						// loop for expanding individual operands
			if(i2==i1) return -3;		// missing AND operand
			if(nampterm==maxamp) return -4;		// too many AND terms
			ampindx1[nampterm]=temptop;
			added=strexpandlogic(pat,i1,i2,resultsptr,temptop,nrptr,maxrptr);
			if(added<0) return added;
			results=*resultsptr;
			temptop+=added;
			ampindx2[nampterm]=temptop;
			nampterm++;
			i1=i2+1;
			if(i2==stop) i2=-1;
			else {
				i2=i1+strChrBrackets(pat+i1,stop-i1,'&',"{");
				if(i2<i1) i2=stop; }}

		pdone=0;
		ptop=temptop;						// ptop will be the top of the permuted results
		while(pdone!=2) {					// loop for permuting operands
			for(iamp=0;iamp<nampterm;iamp++) indx[iamp]=ampindx1[iamp];
			while(indx[nampterm-1]!=ampindx2[nampterm-1]) {
				if(*nrptr==*maxrptr) {
					er=allocresults(resultsptr,maxrptr,STRCHAR);
					if(er) return -1;
					results=*resultsptr; }
				results[ptop][0]='\0';
				for(iamp=0;iamp<nampterm;iamp++)
					strcat(results[ptop],results[indx[iamp]]);
				*nrptr+=1;
				ptop++;
				indx[0]++;					// indx[] is a counter that goes through all options for each operand
				iamp=0;
				while(iamp<nampterm-1 && indx[iamp]==ampindx2[iamp]) {
					indx[iamp]=ampindx1[iamp];
					iamp++;
					indx[iamp]++; }}
			pdone=permutelex(ampindx1,nampterm);	// get next permutation
			permutelex(ampindx2,nampterm); }

		totaladded=ptop-temptop;
		topdiff=temptop-top;			// compact results list
		for(i=top;i<temptop;i++) results[i][0]='\0';
		for(i=temptop;i<ptop;i++) {
			tempresult=results[i-topdiff];
			results[i-topdiff]=results[i];
			results[i]=tempresult; }
		*nrptr-=topdiff;
		done=1; }

	if(!done) i2=start+strChrBrackets(pat+start,stop-start,'{',"");		 // braces
	if(!done && i2>=start) {
		i1=i2;
		i2=strparenmatch(pat,i1);
		if(i2<0) return -5;		// no matching brace
		added=strexpandlogic(pat,i1+1,i2,resultsptr,top,nrptr,maxrptr);
		if(added<0) return added;
		results=*resultsptr;
		for(i=top;i<top+added;i++) {
			strPreCat(results[i],pat,start,i1);
			strPostCat(results[i],pat,i2+1,stop); }
		totaladded=added;
		done=1; }

	if(!done) {
		if(stop==start) done=1;											// empty pattern
		else {
			if(*nrptr==*maxrptr) {
				er=allocresults(resultsptr,maxrptr,STRCHAR);
				if(er) return -1;
				results=*resultsptr; }
			strncpy(results[top],pat+start,stop-start);
			results[top][stop-start]='\0';
			totaladded=1;
			*nrptr+=1; }}
	
	return totaladded; }


/* strEnhWildcardMatch */
int strEnhWildcardMatch(const char *pat,const char *str) {
	static char localpat[STRCHAR]="";
	static char **results=NULL;
	static int nr=0,maxr=0;
	int i,ans;

	if(!pat || !str) {																			// free stored memory
		strexpandlogic(NULL,0,-1,&results,0,&nr,&maxr);
		return 0; }

	if(strcmp(pat,localpat)) {															// create list of patterns
		strncpy(localpat,pat,STRCHAR-1);
		localpat[STRCHAR-1]='\0';
		ans=strexpandlogic(localpat,0,-1,&results,0,&nr,&maxr);
		if(ans<0) return ans; }

	for(i=0;i<nr;i++)
		if(strwildcardmatch(results[i],str)) return 1;
	
	return 0; }


/* strEnhWildcardMatchAndSub */
int strEnhWildcardMatchAndSub(const char *pat,const char *str,const char *destpat,char *dest) {
	static char localpat[STRCHAR]="",localdestpat[STRCHAR]="";
	static char **patlist=NULL,**destlist=NULL;
	static int npr=0,maxpr=0,ndr=0,maxdr=0;
	int i,ans;
	
	if(!pat || !str || !destpat) {
		strexpandlogic(NULL,0,-1,&patlist,0,&npr,&maxpr);
		strexpandlogic(NULL,0,-1,&destlist,0,&ndr,&maxdr);
		return 0; }

	if(strcmp(pat,localpat)) {
		strcpy(localpat,pat);
		ans=strexpandlogic(localpat,0,-1,&patlist,0,&npr,&maxpr);
		if(ans<0) return ans; }
	
	if(strcmp(destpat,localdestpat)) {
		strcpy(localdestpat,destpat);
		ans=strexpandlogic(localdestpat,0,-1,&destlist,0,&ndr,&maxdr);
		if(ans<0) return ans; }

	if(ndr>1 && !strsymbolmatch(localpat,localdestpat,"{}&|")) return -10;

	for(i=0;i<npr;i++)
		if(strwildcardmatch(patlist[i],str)) {
			strcpy(dest,destlist[ndr==1?0:i]);
			strwildcardmatchandsub(patlist[i],str,dest);
			return 1; }

	return 0; }



/******************************************************************/
/***************************** Math parsing ***********************/
/******************************************************************/



double dblnan();



/* dblnan */
#if (_MSC_VER >= 1600)
#include <limits>
double dblnan() {
	return std::numeric_limits<double>::quiet_NaN( ); }
#else
double dblnan() {
  return 0.0/0.0; }
#endif


/* strmatheval */
double strmatheval(char *expression,char **varnames,const double *varvalues,int nvar) {
  static int unarysymbol=0;
  int length,i1,i2;
  double answer,term;
  char *ptr,*ptr2,ptrchar,ptr2char;

  MathParseError=0;
  length=strlen(expression);
  CHECKS(length>0,"missing expression");

  if(strisnumber(expression)) {
    unarysymbol=0;
    answer=strtod(expression,NULL); }

  else if((i1=stringfind(varnames,nvar,expression))>=0) {
    unarysymbol=0;
    answer=varvalues[i1]; }

  else if((i1=strPbrkBrackets(expression,length-1,"+-","([{"))>0) {  // binary + -
    unarysymbol=0;
    ptr=expression+i1;
    ptrchar=*ptr;
    *ptr='\0';
    answer=strmatheval(expression,varnames,varvalues,nvar);        // first term
    *ptr=ptrchar;
    while((i2=strPbrkBrackets(ptr+1,strlen(ptr+1)-1,"+-","([{"))>0) {
      ptr2=ptr+1+i2;
      ptr2char=*ptr2;
      *ptr2='\0';
      term=strmatheval(ptr+1,varnames,varvalues,nvar);          // middle terms
      *ptr2=ptr2char;
      if(ptrchar=='+') answer=answer+term;
      else answer=answer-term;
      ptr=ptr2;
      ptrchar=ptr2char; }
    term=strmatheval(ptr+1,varnames,varvalues,nvar);            // last term
    if(ptrchar=='+') answer=answer+term;
    else answer=answer-term; }

  else if((i1=strPbrkBrackets(expression,length-1,"*/%","([{"))>0) {  // binary * / %
    unarysymbol=0;
    ptr=expression+i1;
    ptrchar=*ptr;
    *ptr='\0';
    answer=strmatheval(expression,varnames,varvalues,nvar);     // first term
    *ptr=ptrchar;
    while((i2=strPbrkBrackets(ptr+1,strlen(ptr+1)-1,"*/%","([{"))>0) {
      ptr2=ptr+1+i2;
      ptr2char=*ptr2;
      *ptr2='\0';
      term=strmatheval(ptr+1,varnames,varvalues,nvar);          // middle terms
      *ptr2=ptr2char;
      if(ptrchar=='*') answer=answer*term;
      else if(ptrchar=='/') {
        CHECKS(term!=0,"divide by zero");
        answer=answer/term; }
      else {
        CHECKS(term>0.5,"illegal modulo value");
        answer=(double)((long int)(answer+0.5)%(long int)(term+0.5)); }
      ptr=ptr2;
      ptrchar=ptr2char; }
    term=strmatheval(ptr+1,varnames,varvalues,nvar);            // last term
    if(ptrchar=='*') answer=answer*term;
    else if(ptrchar=='/') {
      CHECKS(term!=0,"divide by zero");
      answer=answer/term; }
    else {
      CHECKS(term>0.5,"illegal modulo value");
      answer=(double)((long int)(answer+0.5)%(long int)(term+0.5)); }}

  else if(expression[0]=='+' || expression[0]=='-') {           // unary + -
    CHECKS(!unarysymbol,"cannot have multiple preceding signs");
    unarysymbol=1;
    term=strmatheval(expression+1,varnames,varvalues,nvar);
    answer=expression[0]=='+'?term:-term; }

  else if(strchr("([{",expression[0]) && strparenmatch(expression,0)==length-1) {   // {[()]}
    unarysymbol=0;
    ptr=expression+length-1;
    ptrchar=*ptr;
    *ptr='\0';
    answer=strmatheval(expression+1,varnames,varvalues,nvar);
    *ptr=ptrchar; }

  else {
    answer=0;
    CHECKS(1,"syntax error"); }

  return answer;
failure:
  return dblnan(); }
    

/* strmatherror */
int strmatherror(char *string) {
  if(string)
    strcpy(string,MathParseError?StrErrorString:"");
  return MathParseError; }





