#include <menuet/os.h>
#include <math.h>
#include <time.h>

#ifdef DBG
#undef DBG
#endif
#define DBG(s) __menuet__debug_out(s) /* For the debug messages in BOARD */
//#define DBG(s) LOG((s))            /* So that we see debug in Netsurf's LOG files */

/*	
long int  __divdi3(long int  a, long int  b) {
	//DBG("divdi3\n");
	return 0;}
*/

/*
long long int __divdi3(long long int a, long long int b) {
	DBG("divdi3\n");
	return a/b;}
*/


char * locale_charset(){
	DBG("Charset is US\n");
	return "US-ASCII";}

float strtof(const char* str, char** endptr){
	DBG("STRTOOOF\n");
	return 0.0f;}
	
	
char *realpath(const char *path, char *resolved_path){
	char zuup[1024];
	int i;
	char *zoo;
	char *boo;
	char n, s;
	
	if (resolved_path==0) {
		DBG("ResPath is null");
		resolved_path=malloc(strlen(path)+1);
	}
	
	zoo=resolved_path;
	boo=path;
	memcpy(zoo,boo,strlen(boo));
	zoo[strlen(boo)]=0;
	
	
	for (i=0; i<strlen(boo); i++) {
		n=*path;
		path++;
		s=*path;
		
		if ((n=='/' && s!='/') || (n!='/') ) {
			*resolved_path=n;
			resolved_path++;
			
		}
	}
	*resolved_path=0;
	
	
	
	DBG("Real path is... \n");
	sprintf (zuup, "A:%s\n", boo);
	DBG(zuup);
	sprintf (zuup, "B:%s\n", zoo);
	DBG(zuup);
	
	//memcpy(resolved_path, path, strlen(path));
	
	
	
	
	
	return zoo;
}

#include <stdint.h>
#include <netinet/in.h>

int inet_aton(const char *cp, struct in_addr *inp){
	DBG("No IP\n");
	return 0;
}

#include <stdarg.h>

void va_copy(va_list dest, va_list src){ //WHAA
dest=src;
DBG("Some shit...\n");
}



void timeradd(struct timeval *a, struct timeval *b,
                     struct timeval *res){
						 
						  (res)->tv_sec = (a)->tv_sec + (b)->tv_sec;      
        (res)->tv_usec = (a)->tv_usec + (b)->tv_usec;   
         if ((res)->tv_usec >= 1000000) {        
             (res)->tv_sec++;                    
             (res)->tv_usec -= 1000000;          
         }      
						 //DBG("Timeradd\n");
 }

void timersub(struct timeval *a, struct timeval *b,
                     struct timeval *res){
						 
						 res->tv_sec=a->tv_sec - b->tv_sec;
						 res->tv_usec=a->tv_usec - b->tv_usec;
						 if ((res)->tv_usec < 0) {               
             (res)->tv_sec--;                    
             (res)->tv_usec += 1000000;          
         }     
						// DBG("Timersub\n");
	 }

      

       int timerisset(struct timeval *tvp){
		   
		   //DBG("Timer is set?\n");
		   
		   return  ((tvp)->tv_sec || (tvp)->tv_usec);
		   
		   }

       int timercmp(struct timeval *a, struct timeval *b, int z){
		   
		   //DBG("Timercmp is a MACRO \n");
		   
		   if (a->tv_sec > b->tv_sec) return 1; //was 1
		   return 0;}

int wctomb(char *s, int wchar){
	
	DBG("wctomb\n");
	return 0;}
	
int wcrtomb(char * s, int wc, int * ps){
	DBG("wcrtomb\n");
	return 0;
}

int mbrtowc(int * pwc, const char * s,
       int n, int * ps){
		   DBG("mbrtowc\n");
		   return 0;}

int johab_hangul_decompose( const char * s){
		   DBG("hanguul?\n");
		   return 0;}
